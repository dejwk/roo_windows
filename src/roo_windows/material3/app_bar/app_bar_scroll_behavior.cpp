#include "roo_windows/material3/app_bar/app_bar_scroll_behavior.h"

#include <cmath>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/animation_registry.h"

namespace roo_windows::material3::internal {
using roo_windows::internal::ScrollConnectionRegistry;
AppBarScrollConnection::AppBarScrollConnection(Widget& owner,
                                               SimpleScrollablePanel& panel,
                                               AppBarScrollBehavior behavior)
    : ScrollConnection(owner, panel), behavior_(behavior) {}

void AppBarScrollConnection::onPositionChanged(ScrollPosition,
                                               ScrollPosition current,
                                               ScrollSource source) {
  if (source == ScrollSource::kProgrammatic &&
      behavior_ != AppBarScrollBehavior::kPinned) {
    cancel();
    setCollapse(current.y < 0 ? limit_ : 0);
  }
  bool scrolled = current.y < 0;
  if (scrolled_ == scrolled) return;
  scrolled_ = scrolled;
  owner().invalidateInterior();
}

void AppBarScrollConnection::onDisconnected(Widget* destroying) {
  if (&owner() != destroying) {
    cancel();
    owner().invalidateInterior();
    owner().requestLayout();
  }
  if (&panel() != destroying) stopPanelMotion();
}

YDim AppBarScrollConnection::measureHeight(YDim expanded, YDim compact,
                                           HeightSpec spec) {
  YDim limit =
      behavior_ == AppBarScrollBehavior::kPinned ? 0 : expanded - compact;
  limit_ = std::max<YDim>(0, limit);
  collapse_ = std::min(collapse_, limit_);
  YDim desired = expanded - collapse_;
  constrained_ = spec.kind() == EXACTLY && spec.value() != desired;
  if (constrained_) {
    if (!warned_) {
      LOG(WARNING)
          << "App bar scroll behavior needs a height that follows its content";
      warned_ = true;
    }
    collapse_ = 0;
    desired = expanded;
  }
  return spec.resolveSize(desired);
}

void AppBarScrollConnection::setCollapse(YDim value) {
  value = constrained_ ? 0 : std::max<YDim>(0, std::min(limit_, value));
  if (collapse_ == value) return;
  collapse_ = value;
  owner().invalidateInterior();
  owner().requestLayout();
}

YDim AppBarScrollConnection::consume(YDim available) {
  YDim previous = collapse_;
  setCollapse(collapse_ - available);
  return previous - collapse_;
}

YDim AppBarScrollConnection::onPreScroll(YDim available) {
  if (behavior_ == AppBarScrollBehavior::kEnterAlways ||
      (behavior_ == AppBarScrollBehavior::kExitUntilCollapsed && available < 0))
    return consume(available);
  return 0;
}

YDim AppBarScrollConnection::onPostScroll(YDim available) {
  return behavior_ == AppBarScrollBehavior::kExitUntilCollapsed && available > 0
             ? consume(available)
             : 0;
}

void AppBarScrollConnection::cancel() {
  context().animations().cancel(owner(), kSettle);
}

void AppBarScrollConnection::reset() { setCollapse(0); }

void AppBarScrollConnection::suspend() {
  cancel();
  stopPanelMotion();
}

void AppBarScrollConnection::finish() {
  if (collapse_ == 0 || collapse_ == limit_ || constrained_) return;
  if (owner().presentationState() != PresentationState::kPresented ||
      panel().presentationState() != PresentationState::kPresented)
    return;
  YDim target = collapse_ * 2 >= limit_ ? limit_ : 0;
  if (behavior_ == AppBarScrollBehavior::kExitUntilCollapsed &&
      panel().getScrollPosition().y < 0)
    target = limit_;
  AnimationSpec spec =
      AnimationSpec::Value(collapse_, target, roo_time::Millis(150));
  spec.easing.kind = EasingKind::kCubicBezier;
  spec.easing.x1 = 1.0f / 3;
  spec.easing.y1 = 1;
  spec.easing.x2 = 2.0f / 3;
  spec.easing.y2 = 1;
  context().animations().start(owner(), kSettle, spec);
}

void AppBarScrollConnection::animate(const AnimationSample& sample) {
  setCollapse(static_cast<YDim>(std::lround(sample.value)));
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
  if (search && behavior == AppBarScrollBehavior::kExitUntilCollapsed)
    return ScrollConnectionStatus::kUnsupportedBehavior;
  auto old = FindAppBarConnection(bar);
  if (old != nullptr && old->isDispatching())
    return ScrollConnectionStatus::kBusy;
  if (old != nullptr && &old->panel() == &panel && old->behavior() == behavior)
    return ScrollConnectionStatus::kSuccess;
  auto connection =
      std::make_shared<AppBarScrollConnection>(bar, panel, behavior);
  ScrollConnectionStatus status =
      context.scrollConnections().install(connection);
  if (status == ScrollConnectionStatus::kSuccess) {
    connection->onPositionChanged({}, panel.getScrollPosition(),
                                  ScrollSource::kProgrammatic);
    context.presentations().observe(bar);
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
