#include "roo_windows/core/animation_registry.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>

#include "roo_windows/core/animation_evaluator.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {
namespace {

roo_time::Uptime addSaturated(roo_time::Uptime base,
                              roo_time::Duration duration) {
  const int64_t max_us = std::numeric_limits<int64_t>::max();
  if (duration == roo_time::Duration::Max() ||
      base.inMicros() > max_us - duration.inMicros()) {
    return roo_time::Uptime::Max();
  }
  return base + duration;
}

}  // namespace

AnimationRegistry::AnimationRegistry(ApplicationContext& context)
    : context_(context), tracks_(0) {}

size_t AnimationRegistry::ChannelHash::operator()(const ChannelKey& key) const {
  const size_t pointer = reinterpret_cast<uintptr_t>(key.target);
  return (pointer >> 3) ^ (static_cast<size_t>(key.tag) * 0x9e3779b1U);
}

AnimationRegistry::TrackMap::iterator AnimationRegistry::find(
    Widget& target, AnimationTag tag) {
  return tracks_.find(ChannelKey{&target, tag});
}

AnimationRegistry::TrackMap::const_iterator AnimationRegistry::find(
    const Widget& target, AnimationTag tag) const {
  return tracks_.find(ChannelKey{const_cast<Widget*>(&target), tag});
}

AnimationStatus AnimationRegistry::start(Widget& target, AnimationTag tag,
                                         const AnimationSpec& spec) {
  if (!internal::isValidAnimationSpec(spec)) {
    return AnimationStatus::kInvalidSpec;
  }
  if (target.tryContext() != &context_) return AnimationStatus::kNotFound;
  if (stopped_ || context_.frame_driver_ == nullptr) {
    return AnimationStatus::kNoFrameDriver;
  }

  const ChannelKey key{&target, tag};
  TrackMap::iterator existing = tracks_.find(key);
  dispatch_.reserve(tracks_.size() + (existing == tracks_.end() ? 1 : 0));
  invalidateDispatchItem(key);
  Track track;
  track.spec = spec;
  track.last_value = spec.from;
  if (existing == tracks_.end()) {
    tracks_.insert(std::make_pair(key, track));
  } else {
    existing->second = track;
  }
  requestFrame(roo_time::Uptime::Start());
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::cancel(Widget& target, AnimationTag tag) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  const ChannelKey key{&target, tag};
  if (tracks_.find(key) == tracks_.end()) return AnimationStatus::kNotFound;
  invalidateDispatchItem(key);
  tracks_.erase(key);
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::pause(Widget& target, AnimationTag tag) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  TrackMap::iterator found = find(target, tag);
  if (found == tracks_.end()) return AnimationStatus::kNotFound;
  invalidateDispatchItem(ChannelKey{&target, tag});
  Track& track = found->second;
  if (!track.paused) {
    if (track.anchored)
      track.elapsed_at_anchor = elapsedAt(track, controlTime());
    track.paused = true;
  }
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::resume(Widget& target, AnimationTag tag) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  TrackMap::iterator found = find(target, tag);
  if (found == tracks_.end()) return AnimationStatus::kNotFound;
  invalidateDispatchItem(ChannelKey{&target, tag});
  Track& track = found->second;
  if (track.paused) {
    track.anchor = controlTime();
    track.paused = false;
  }
  requestFrame(roo_time::Uptime::Start());
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::restart(Widget& target, AnimationTag tag) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  TrackMap::iterator found = find(target, tag);
  if (found == tracks_.end()) return AnimationStatus::kNotFound;
  invalidateDispatchItem(ChannelKey{&target, tag});
  Track& track = found->second;
  track.anchor = roo_time::Uptime();
  track.elapsed_at_anchor = roo_time::Duration();
  track.last_sample_elapsed = roo_time::Duration();
  track.last_value = track.spec.from;
  track.anchored = false;
  track.sampled = false;
  track.seek_pending = false;
  track.finish_pending = false;
  requestFrame(roo_time::Uptime::Start());
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::seek(Widget& target, AnimationTag tag,
                                        roo_time::Duration elapsed) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  TrackMap::iterator found = find(target, tag);
  if (found == tracks_.end()) return AnimationStatus::kNotFound;
  if (elapsed.inMicros() < 0) return AnimationStatus::kInvalidSpec;
  invalidateDispatchItem(ChannelKey{&target, tag});
  Track& track = found->second;
  const roo_time::Duration end = internal::animationEnd(track.spec);
  if (end != roo_time::Duration::Max() && elapsed > end) elapsed = end;
  track.elapsed_at_anchor = elapsed;
  track.anchor = controlTime();
  track.anchored = true;
  track.seek_pending = true;
  track.finish_pending = false;
  requestFrame(roo_time::Uptime::Start());
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::retarget(Widget& target, AnimationTag tag,
                                            float to,
                                            roo_time::Duration duration) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  TrackMap::iterator found = find(target, tag);
  if (found == tracks_.end()) return AnimationStatus::kNotFound;
  if (found->second.spec.kind != AnimationKind::kValue) {
    return AnimationStatus::kUnsupported;
  }
  AnimationSpec replacement =
      AnimationSpec::value(found->second.sampled ? found->second.last_value
                                                 : found->second.spec.from,
                           to, duration);
  replacement.minimum_interval = found->second.spec.minimum_interval;
  replacement.easing = found->second.spec.easing;
  if (!internal::isValidAnimationSpec(replacement)) {
    return AnimationStatus::kInvalidSpec;
  }

  invalidateDispatchItem(ChannelKey{&target, tag});
  const bool paused = found->second.paused;
  Track track;
  track.spec = replacement;
  track.last_value = replacement.from;
  track.paused = paused;
  found->second = track;
  requestFrame(roo_time::Uptime::Start());
  return AnimationStatus::kOk;
}

AnimationStatus AnimationRegistry::finish(Widget& target, AnimationTag tag) {
  if (stopped_ || target.tryContext() != &context_) {
    return AnimationStatus::kNotFound;
  }
  TrackMap::iterator found = find(target, tag);
  if (found == tracks_.end()) return AnimationStatus::kNotFound;
  if (found->second.spec.kind != AnimationKind::kValue ||
      found->second.spec.legs == 0) {
    return AnimationStatus::kUnsupported;
  }
  invalidateDispatchItem(ChannelKey{&target, tag});
  found->second.finish_pending = true;
  found->second.seek_pending = false;
  requestFrame(roo_time::Uptime::Start());
  return AnimationStatus::kOk;
}

bool AnimationRegistry::contains(const Widget& target, AnimationTag tag) const {
  return !stopped_ && target.tryContext() == &context_ &&
         find(target, tag) != tracks_.end();
}

void AnimationRegistry::clearTarget(Widget& target) {
  if (stopped_ || target.tryContext() != &context_) return;
  for (DispatchItem& item : dispatch_) {
    if (item.key.target == &target) item.valid = false;
  }
  for (TrackMap::iterator it = tracks_.begin(); it != tracks_.end();) {
    if (it->first.target == &target) {
      it = tracks_.erase(it);
    } else {
      ++it;
    }
  }
}

void AnimationRegistry::invalidateDispatchItem(const ChannelKey& key) {
  if (!dispatching_) return;
  for (DispatchItem& item : dispatch_) {
    if (item.valid && item.key == key) item.valid = false;
  }
}

void AnimationRegistry::requestFrame(roo_time::Uptime deadline) {
  if (context_.frame_driver_ != nullptr) {
    context_.frame_driver_->requestAnimationFrameAt(deadline);
  }
}

roo_time::Uptime AnimationRegistry::controlTime() const {
  return dispatching_ ? frame_time_ : roo_time::Uptime::Now();
}

roo_time::Duration AnimationRegistry::elapsedAt(const Track& track,
                                                roo_time::Uptime now) const {
  if (!track.anchored || track.paused) return track.elapsed_at_anchor;
  roo_time::Duration since_anchor = now - track.anchor;
  if (since_anchor.inMicros() < 0) since_anchor = roo_time::Duration();
  const int64_t max_us = std::numeric_limits<int64_t>::max();
  if (track.elapsed_at_anchor.inMicros() > max_us - since_anchor.inMicros()) {
    return roo_time::Duration::Max();
  }
  roo_time::Duration elapsed = track.elapsed_at_anchor + since_anchor;
  const roo_time::Duration end = internal::animationEnd(track.spec);
  return end != roo_time::Duration::Max() && elapsed > end ? end : elapsed;
}

roo_time::Uptime AnimationRegistry::deadlineAtElapsed(
    const Track& track, roo_time::Duration elapsed) const {
  if (elapsed <= track.elapsed_at_anchor) return track.anchor;
  return addSaturated(track.anchor, elapsed - track.elapsed_at_anchor);
}

roo_time::Uptime AnimationRegistry::trackDeadline(const Track& track) const {
  if (track.seek_pending || track.finish_pending) {
    return roo_time::Uptime::Start();
  }
  if (track.paused) return roo_time::Uptime::Max();
  if (!track.anchored) return roo_time::Uptime::Start();
  const roo_time::Duration end = internal::animationEnd(track.spec);
  if (!track.sampled) return track.anchor;
  if (track.last_sample_elapsed < track.spec.delay) {
    return deadlineAtElapsed(track, track.spec.delay);
  }
  roo_time::Duration interval_elapsed = roo_time::Duration::Max();
  const int64_t max_us = std::numeric_limits<int64_t>::max();
  if (track.last_sample_elapsed.inMicros() <=
      max_us - track.spec.minimum_interval.inMicros()) {
    interval_elapsed = track.last_sample_elapsed + track.spec.minimum_interval;
  }
  const roo_time::Uptime interval_deadline =
      deadlineAtElapsed(track, interval_elapsed);
  const roo_time::Uptime end_deadline = deadlineAtElapsed(track, end);
  return std::min(interval_deadline, end_deadline);
}

bool AnimationRegistry::isDue(const Track& track) const {
  return trackDeadline(track) <= frame_time_;
}

roo_time::Uptime AnimationRegistry::nextFrameDeadline() const {
  if (stopped_ || tracks_.empty()) return roo_time::Uptime::Max();
  roo_time::Uptime result = roo_time::Uptime::Max();
  for (const auto& entry : tracks_) {
    result = std::min(result, trackDeadline(entry.second));
  }
  return result;
}

void AnimationRegistry::beginFrame(roo_time::Uptime now) {
  if (stopped_ || dispatching_) return;
  dispatching_ = true;
  frame_time_ = now;
  next_item_ = 0;
  dispatch_.clear();
  dispatch_.reserve(tracks_.size());
  for (const auto& entry : tracks_) {
    dispatch_.push_back(DispatchItem{entry.first, true});
  }
}

bool AnimationRegistry::dispatchNext() {
  if (!dispatching_ || next_item_ >= dispatch_.size()) return false;
  const size_t item_index = next_item_++;
  if (!dispatch_[item_index].valid) return true;
  const ChannelKey key = dispatch_[item_index].key;
  TrackMap::iterator found = tracks_.find(key);
  if (found == tracks_.end() || !isDue(found->second)) return true;

  Track& track = found->second;
  if (!track.anchored) {
    track.anchor = frame_time_;
    track.elapsed_at_anchor = roo_time::Duration();
    track.anchored = true;
  }
  const bool forced = track.finish_pending;
  const bool sought = track.seek_pending;
  const roo_time::Duration elapsed = forced ? internal::animationEnd(track.spec)
                                            : elapsedAt(track, frame_time_);
  const roo_time::Duration delta = track.sampled
                                       ? elapsed - track.last_sample_elapsed
                                       : roo_time::Duration();
  AnimationSample sample =
      internal::evaluateAnimation(track.spec, elapsed, delta);
  if (sought && !forced) sample.terminal = false;
  track.last_sample_elapsed = elapsed;
  track.last_value = sample.value;
  track.sampled = true;
  track.seek_pending = false;
  track.finish_pending = false;
  key.target->onAnimationFrame(key.tag, sample);

  if (!sample.terminal || !dispatch_[item_index].valid) return true;
  found = tracks_.find(key);
  if (found == tracks_.end()) return true;
  tracks_.erase(key);
  key.target->onAnimationFinished(key.tag,
                                  forced ? AnimationFinishReason::kForced
                                         : AnimationFinishReason::kCompleted);
  return true;
}

void AnimationRegistry::endFrame() {
  if (!dispatching_) return;
  dispatch_.clear();
  next_item_ = 0;
  dispatching_ = false;
}

void AnimationRegistry::stop() {
  if (stopped_) return;
  stopped_ = true;
  for (DispatchItem& item : dispatch_) item.valid = false;
  tracks_.clear();
  endFrame();
}

}  // namespace roo_windows
