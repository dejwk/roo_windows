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

roo_time::Uptime addSaturated(roo_time::Uptime base, roo_time::Duration first,
                              roo_time::Duration second) {
  const int64_t max_us = std::numeric_limits<int64_t>::max();
  if (first.inMicros() > max_us - second.inMicros()) {
    return roo_time::Uptime::Max();
  }
  return addSaturated(base,
                      roo_time::Micros(first.inMicros() + second.inMicros()));
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
  if (spec.kind == AnimationKind::kValue &&
      (spec.legs != 1 || spec.playback != AnimationPlayback::kRestart)) {
    return AnimationStatus::kUnsupported;
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
  return find(target, tag) == tracks_.end() ? AnimationStatus::kNotFound
                                            : AnimationStatus::kUnsupported;
}

AnimationStatus AnimationRegistry::resume(Widget& target, AnimationTag tag) {
  return find(target, tag) == tracks_.end() ? AnimationStatus::kNotFound
                                            : AnimationStatus::kUnsupported;
}

AnimationStatus AnimationRegistry::restart(Widget& target, AnimationTag tag) {
  return find(target, tag) == tracks_.end() ? AnimationStatus::kNotFound
                                            : AnimationStatus::kUnsupported;
}

AnimationStatus AnimationRegistry::seek(Widget& target, AnimationTag tag,
                                        roo_time::Duration) {
  return find(target, tag) == tracks_.end() ? AnimationStatus::kNotFound
                                            : AnimationStatus::kUnsupported;
}

AnimationStatus AnimationRegistry::retarget(Widget& target, AnimationTag tag,
                                            float, roo_time::Duration) {
  return find(target, tag) == tracks_.end() ? AnimationStatus::kNotFound
                                            : AnimationStatus::kUnsupported;
}

AnimationStatus AnimationRegistry::finish(Widget& target, AnimationTag tag) {
  return find(target, tag) == tracks_.end() ? AnimationStatus::kNotFound
                                            : AnimationStatus::kUnsupported;
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

roo_time::Uptime AnimationRegistry::trackDeadline(const Track& track) const {
  if (!track.anchored) return roo_time::Uptime::Start();
  const roo_time::Duration end = internal::animationEnd(track.spec);
  if (!track.sampled) return track.anchor;
  if (track.last_sample_elapsed < track.spec.delay) {
    return addSaturated(track.anchor, track.spec.delay);
  }
  const roo_time::Uptime interval_deadline = addSaturated(
      track.anchor, track.last_sample_elapsed, track.spec.minimum_interval);
  const roo_time::Uptime end_deadline = addSaturated(track.anchor, end);
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
    track.anchored = true;
  }
  roo_time::Duration elapsed = frame_time_ - track.anchor;
  if (elapsed.inMicros() < 0) elapsed = roo_time::Duration();
  const roo_time::Duration delta = track.sampled
                                       ? elapsed - track.last_sample_elapsed
                                       : roo_time::Duration();
  const AnimationSample sample =
      internal::evaluateAnimation(track.spec, elapsed, delta);
  track.last_sample_elapsed = elapsed;
  track.sampled = true;
  key.target->onAnimationFrame(key.tag, sample);

  if (!sample.terminal || !dispatch_[item_index].valid) return true;
  found = tracks_.find(key);
  if (found == tracks_.end()) return true;
  tracks_.erase(key);
  key.target->onAnimationFinished(key.tag, AnimationFinishReason::kCompleted);
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
