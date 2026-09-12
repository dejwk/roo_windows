#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "roo_collections_flat_small_hash_map.h"
#include "roo_time.h"
#include "roo_windows/core/animation_types.h"

namespace roo_windows {

class Application;
class ApplicationContext;
class DisplayWindow;
class Widget;
namespace test {
struct AnimationRegistryTestAccess;
}

/// Owns time-based animation playback for widgets in one application context.
class AnimationRegistry {
 public:
  explicit AnimationRegistry(ApplicationContext& context);

  /// Validates and replaces the current animation on `target` and `tag`.
  AnimationStatus start(Widget& target, AnimationTag tag,
                        const AnimationSpec& spec);

  /// Silently removes the current animation on `target` and `tag`.
  AnimationStatus cancel(Widget& target, AnimationTag tag);

  /// Pauses the current channel, or reports unsupported until controls land.
  AnimationStatus pause(Widget& target, AnimationTag tag);

  /// Resumes the current channel, or reports unsupported until controls land.
  AnimationStatus resume(Widget& target, AnimationTag tag);

  /// Restarts the current channel, or reports unsupported until controls land.
  AnimationStatus restart(Widget& target, AnimationTag tag);

  /// Seeks the current channel, or reports unsupported until controls land.
  AnimationStatus seek(Widget& target, AnimationTag tag,
                       roo_time::Duration elapsed);

  /// Retargets a value channel, or reports unsupported until controls land.
  AnimationStatus retarget(Widget& target, AnimationTag tag, float to,
                           roo_time::Duration duration);

  /// Finishes a finite channel, or reports unsupported until controls land.
  AnimationStatus finish(Widget& target, AnimationTag tag);

  /// Returns whether `target` has a current animation on `tag`.
  bool contains(const Widget& target, AnimationTag tag) const;

  /// Silently removes every animation belonging to `target`.
  void clearTarget(Widget& target);

 private:
  friend class Application;
  friend class ApplicationContext;
  friend class DisplayWindow;
  friend struct test::AnimationRegistryTestAccess;

  struct ChannelKey {
    Widget* target = nullptr;
    AnimationTag tag = 0;

    bool operator==(const ChannelKey& other) const {
      return target == other.target && tag == other.tag;
    }
  };

  struct ChannelHash {
    size_t operator()(const ChannelKey& key) const;
  };

  struct Track {
    AnimationSpec spec;
    roo_time::Uptime anchor;
    roo_time::Duration last_sample_elapsed;
    bool anchored = false;
    bool sampled = false;
  };

  struct DispatchItem {
    ChannelKey key;
    bool valid = true;
  };

  using TrackMap =
      roo_collections::FlatSmallHashMap<ChannelKey, Track, ChannelHash>;

  TrackMap::iterator find(Widget& target, AnimationTag tag);
  TrackMap::const_iterator find(const Widget& target, AnimationTag tag) const;
  void invalidateDispatchItem(const ChannelKey& key);
  void requestFrame(roo_time::Uptime deadline);
  roo_time::Uptime trackDeadline(const Track& track) const;
  bool isDue(const Track& track) const;
  roo_time::Uptime nextFrameDeadline() const;
  void beginFrame(roo_time::Uptime now);
  bool dispatchNext();
  void endFrame();
  void stop();

  ApplicationContext& context_;
  TrackMap tracks_;
  std::vector<DispatchItem> dispatch_;
  roo_time::Uptime frame_time_;
  size_t next_item_ = 0;
  bool dispatching_ = false;
  bool stopped_ = false;
};

}  // namespace roo_windows
