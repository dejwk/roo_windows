#include "roo_windows/core/scroll_connection.h"

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/application_context.h"

namespace roo_windows::internal {
ScrollConnection::ScrollConnection(Widget& owner, SimpleScrollablePanel& panel)
    : owner_(&owner), panel_(&panel) {}

ApplicationContext& ScrollConnection::context() const {
  return owner().context();
}

void ScrollConnection::stopPanelMotion() {
  kinetic = false;
  panel().stopMotionAndClamp();
}

std::shared_ptr<ScrollConnection> ScrollConnectionRegistry::find(
    const Widget& endpoint) const {
  auto owner = owners_.find(&endpoint);
  if (owner != owners_.end()) return (*owner).second;
  auto panel = panels_.find(&endpoint);
  return panel == panels_.end() ? nullptr : (*panel).second;
}

ScrollConnectionStatus ScrollConnectionRegistry::install(
    std::shared_ptr<ScrollConnection> connection) {
  Widget& owner = connection->owner();
  SimpleScrollablePanel& panel = connection->panel();
  if (&owner.context() != &panel.context())
    return ScrollConnectionStatus::kDifferentContext;
  if (panel.direction() == SimpleScrollablePanel::Direction::kHorizontal)
    return ScrollConnectionStatus::kUnsupportedAxis;
  auto old = find(owner);
  auto occupied = find(panel);
  if ((old != nullptr && old->dispatch_depth_ != 0) ||
      (occupied != nullptr && occupied->dispatch_depth_ != 0))
    return ScrollConnectionStatus::kBusy;
  if (occupied != nullptr && &occupied->owner() != &owner)
    return ScrollConnectionStatus::kAlreadyConnected;
  remove(owner);
  owners_[&owner] = connection;
  panels_[&panel] = std::move(connection);
  return ScrollConnectionStatus::kSuccess;
}

ScrollConnectionStatus ScrollConnectionRegistry::remove(Widget& endpoint,
                                                        bool destroying) {
  if (destroying) flex_scratch_.erase(&endpoint);
  auto connection = find(endpoint);
  if (connection == nullptr) return ScrollConnectionStatus::kSuccess;
  if (!destroying && connection->dispatch_depth_ != 0)
    return ScrollConnectionStatus::kBusy;
  owners_.erase(&connection->owner());
  panels_.erase(&connection->panel());
  connection->onDisconnected(destroying ? &endpoint : nullptr);
  return ScrollConnectionStatus::kSuccess;
}

void ScrollConnectionRegistry::Disconnect(Widget& endpoint) {
  ApplicationContext* context = endpoint.tryContext();
  if (context != nullptr && context->scrollConnectionsIfPresent() != nullptr)
    context->scrollConnectionsIfPresent()->remove(endpoint, true);
}

std::shared_ptr<ScrollConnection> ScrollConnectionRegistry::Find(
    const Widget& endpoint) {
  const ApplicationContext* context = endpoint.tryContext();
  return context == nullptr || context->scrollConnectionsIfPresent() == nullptr
             ? nullptr
             : context->scrollConnectionsIfPresent()->find(endpoint);
}
}  // namespace roo_windows::internal
