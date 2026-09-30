#include "roo_windows/material3/app_bar/app_bar_scroll_behavior.h"

#include "roo_windows/containers/scrollable_panel.h"

namespace roo_windows::material3::internal {
using roo_windows::internal::ScrollConnectionRegistry;
AppBarScrollConnection::AppBarScrollConnection(Widget& owner,
                                               SimpleScrollablePanel& panel,
                                               AppBarScrollBehavior behavior)
    : ScrollConnection(owner, panel), behavior_(behavior) {}

void AppBarScrollConnection::onPositionChanged(ScrollPosition,
                                               ScrollPosition current,
                                               ScrollSource) {
  bool scrolled = current.y < 0;
  if (scrolled_ == scrolled) return;
  scrolled_ = scrolled;
  owner().invalidateInterior();
}

void AppBarScrollConnection::onDisconnected(Widget* destroying) {
  if (&owner() != destroying) {
    owner().invalidateInterior();
    owner().requestLayout();
  }
}

std::shared_ptr<AppBarScrollConnection> FindAppBarConnection(
    const Widget& bar) {
  return std::static_pointer_cast<AppBarScrollConnection>(
      ScrollConnectionRegistry::Find(bar));
}

ScrollConnectionStatus ConnectAppBar(ApplicationContext& context, Widget& bar,
                                     SimpleScrollablePanel& panel,
                                     AppBarScrollBehavior behavior,
                                     bool search) {
  if (behavior != AppBarScrollBehavior::kPinned)
    return ScrollConnectionStatus::kUnsupportedBehavior;
  auto old = FindAppBarConnection(bar);
  if (old != nullptr && &old->panel() == &panel && old->behavior() == behavior)
    return ScrollConnectionStatus::kSuccess;
  auto connection =
      std::make_shared<AppBarScrollConnection>(bar, panel, behavior);
  ScrollConnectionStatus status =
      context.scrollConnections().install(connection);
  if (status == ScrollConnectionStatus::kSuccess) {
    connection->onPositionChanged({}, panel.getScrollPosition(),
                                  ScrollSource::kProgrammatic);
    bar.invalidateInterior();
  }
  return status;
}

ScrollConnectionStatus ClearAppBarConnection(ApplicationContext& context,
                                             Widget& bar) {
  auto* registry = context.scrollConnectionsIfPresent();
  return registry == nullptr ? ScrollConnectionStatus::kSuccess
                             : registry->remove(bar);
}
}  // namespace roo_windows::material3::internal
