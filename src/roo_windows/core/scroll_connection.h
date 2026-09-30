#pragma once

#include <memory>

#include "roo_collections/flat_small_hash_map.h"
#include "roo_windows/core/scroll_position.h"

namespace roo_windows {
class Widget;
class ApplicationContext;
class SimpleScrollablePanel;

/// Outcome of connecting or disconnecting a scroll participant.
enum class ScrollConnectionStatus : uint8_t {
  kSuccess,
  kDifferentContext,
  kUnsupportedAxis,
  kUnsupportedBehavior,
  kAlreadyConnected,
  kBusy,
};

/// Distinguishes intentional input from geometry reconciliation.
enum class ScrollSource : uint8_t { kDrag, kKinetic, kProgrammatic, kGeometry };

namespace internal {
/// Core scroll participant. The registry owns it; endpoints remain borrowed.
/// All operations run on the UI thread. No callback retains an event reference.
class ScrollConnection {
 public:
  /// Creates a participant linking two widgets in the same context.
  ScrollConnection(Widget& owner, SimpleScrollablePanel& panel);
  virtual ~ScrollConnection() = default;
  /// Returns the visual endpoint.
  Widget& owner() const { return *owner_; }
  /// Returns the scrolling endpoint.
  SimpleScrollablePanel& panel() const { return *panel_; }
  /// Restores a surviving endpoint after removal; destroying is never accessed.
  virtual void onDisconnected(Widget* destroying) {}
  /// Receives committed content coordinates without occupying its callback.
  virtual void onPositionChanged(ScrollPosition previous,
                                 ScrollPosition current, ScrollSource source) {}
  /// Consumes signed movement before content, bounded by available.
  virtual YDim onPreScroll(YDim available) { return 0; }
  /// Consumes signed movement left after content reaches a legal bound.
  virtual YDim onPostScroll(YDim available) { return 0; }
  /// Returns whether the connection has vertical travel.
  virtual bool canScroll() const { return false; }
  /// Cancels optional participant motion on new input or suspension.
  virtual void cancel() {}
  /// Settles the participant after content motion ends.
  virtual void finish() {}
  /// Resets participant state after replacing content.
  virtual void reset() {}

  // Connected motion samples and raw drag coordinates. Ordinary panels retain
  // their existing State and pay no extra instance cost for coordination.
  ScrollPosition trajectory = {0, 0};
  ScrollPosition raw = {0, 0};
  ScrollSource source = ScrollSource::kGeometry;
  bool kinetic = false;
  bool applying = false;

  /// Guards dispatch against public rebind/clear while retaining the record.
  class Dispatch {
   public:
    explicit Dispatch(std::shared_ptr<ScrollConnection> connection)
        : connection_(std::move(connection)) {
      if (connection_ != nullptr) ++connection_->dispatch_depth_;
    }
    ~Dispatch() {
      if (connection_ != nullptr) --connection_->dispatch_depth_;
    }
    Dispatch(const Dispatch&) = delete;
    Dispatch& operator=(const Dispatch&) = delete;

   private:
    std::shared_ptr<ScrollConnection> connection_;
  };

 private:
  friend class ScrollConnectionRegistry;
  Widget* owner_;
  SimpleScrollablePanel* panel_;
  uint16_t dispatch_depth_ = 0;
};

/// Sparse per-context ownership; no storage is added to individual widgets.
class ScrollConnectionRegistry {
 public:
  /// Installs after validating both endpoints; failure preserves existing
  /// links.
  ScrollConnectionStatus install(std::shared_ptr<ScrollConnection> connection);
  /// Returns the connection for either endpoint, or null.
  std::shared_ptr<ScrollConnection> find(const Widget& endpoint) const;
  /// Removes both entries. Destruction bypasses the dispatch guard.
  ScrollConnectionStatus remove(Widget& endpoint, bool destroying = false);
  /// Removes an endpoint without allocating a registry in an unused context.
  static void Disconnect(Widget& endpoint);
  /// Looks up an endpoint without allocating a registry.
  static std::shared_ptr<ScrollConnection> Find(const Widget& endpoint);

 private:
  using Map =
      roo_collections::FlatSmallHashMap<const Widget*,
                                        std::shared_ptr<ScrollConnection>>;
  Map owners_;
  Map panels_;
};
}  // namespace internal
}  // namespace roo_windows
