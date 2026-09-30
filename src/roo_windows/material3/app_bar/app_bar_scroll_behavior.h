#pragma once
#include "roo_windows/core/scroll_connection.h"

namespace roo_windows::material3 {
/// Selects how an app bar responds to its connected panel.
enum class AppBarScrollBehavior : uint8_t {
  kPinned,
  kEnterAlways,
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
  bool scrolled() const { return scrolled_; }

 private:
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
