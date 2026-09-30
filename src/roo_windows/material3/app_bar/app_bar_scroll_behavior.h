#pragma once
#include "roo_windows/core/animation_types.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/scroll_connection.h"

namespace roo_windows::material3 {
/// Selects how an app bar responds to its connected panel.
enum class AppBarScrollBehavior : uint8_t {
  /// Keeps geometry fixed and follows the content surface state.
  kPinned,
  /// Collapses on forward scrolling and expands immediately on reversal.
  kEnterAlways,
  /// Expands only after content returns to the top. Small bars stay pinned;
  /// SearchAppBar rejects this policy.
  kExitUntilCollapsed
};
namespace internal {
/// Optional Material state, allocated only for a connected bar.
class AppBarScrollConnection : public roo_windows::internal::ScrollConnection {
 public:
  AppBarScrollConnection(Widget& owner, SimpleScrollablePanel& panel,
                         AppBarScrollBehavior behavior);
  void onPositionChanged(ScrollPosition previous, ScrollPosition current,
                         ScrollSource source) override;
  void onDisconnected(Widget* destroying) override;
  AppBarScrollBehavior behavior() const { return behavior_; }
  bool scrolled() const { return scrolled_ || collapse_ > 0; }
  /// Updates collapse bounds from current tokens and parent constraints.
  YDim measureHeight(YDim expanded, YDim compact, HeightSpec spec);
  /// Returns the applied collapse in physical pixels.
  YDim collapse() const { return collapse_; }
  /// Returns the expanded-to-collapsed travel.
  YDim limit() const { return limit_; }
  YDim onPreScroll(YDim available) override;
  YDim onPostScroll(YDim available) override;
  YDim viewportTravel() const override { return collapse_; }
  bool canScroll() const override { return limit_ > 0 && !constrained_; }
  void cancel() override;
  void finish() override;
  void reset() override;
  /// Applies a retained settlement sample before layout and paint.
  void animate(const AnimationSample& sample);
  /// Suspends both motion participants on hide or detachment.
  void suspend();
  static constexpr AnimationTag kSettle = 230;

 private:
  void setCollapse(YDim collapse);
  YDim consume(YDim available);
  YDim collapse_ = 0;
  YDim limit_ = 0;
  bool constrained_ = false;
  bool warned_ = false;

  AppBarScrollBehavior behavior_;
  bool scrolled_ = false;
};
/// Installs a Material connection after validating the requested policy.
ScrollConnectionStatus ConnectAppBar(ApplicationContext& context, Widget& bar,
                                     SimpleScrollablePanel& panel,
                                     AppBarScrollBehavior behavior,
                                     bool search);
/// Looks up the optional Material state for a bar.
std::shared_ptr<AppBarScrollConnection> FindAppBarConnection(const Widget& bar);
/// Removes a binding and restores ordinary bar behavior.
ScrollConnectionStatus ClearAppBarConnection(ApplicationContext& context,
                                             Widget& bar);
}  // namespace internal
}  // namespace roo_windows::material3
