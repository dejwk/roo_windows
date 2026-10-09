#include "roo_windows/core/widget_event_dispatcher.h"

#include <utility>

namespace roo_windows {

void WidgetEventDispatcher::setInteractiveChangeHandler(
    Widget& widget, std::function<void()> handler) {
  if (handler == nullptr) {
    clearInteractiveChangeHandler(widget);
    return;
  }
  interactive_change_handlers_[&widget] =
      std::make_shared<std::function<void()>>(std::move(handler));
}

void WidgetEventDispatcher::clearInteractiveChangeHandler(Widget& widget) {
  interactive_change_handlers_.erase(&widget);
}

bool WidgetEventDispatcher::hasInteractiveChangeHandler(
    const Widget& widget) const {
  return interactive_change_handlers_.contains(&widget);
}

void WidgetEventDispatcher::dispatchInteractiveChange(Widget& widget) {
  auto it = interactive_change_handlers_.find(&widget);
  if (it == interactive_change_handlers_.end()) return;
  // Keep the registered callable alive if the callback removes its widget.
  // Copying std::function here could allocate on every dispatch and would
  // discard changes to mutable captures. Sharing adds a registration-time
  // allocation while preserving the original callable and its state.
  std::shared_ptr<std::function<void()>> handler = (*it).second;
  (*handler)();
}

void WidgetEventDispatcher::setScrollPositionChangeHandler(
    Widget& widget, ScrollHandler handler) {
  if (handler == nullptr) {
    clearScrollPositionChangeHandler(widget);
    return;
  }
  scroll_position_change_handlers_[&widget] =
      std::make_shared<ScrollHandler>(std::move(handler));
}

void WidgetEventDispatcher::clearScrollPositionChangeHandler(Widget& widget) {
  scroll_position_change_handlers_.erase(&widget);
}

bool WidgetEventDispatcher::hasScrollPositionChangeHandler(
    const Widget& widget) const {
  return scroll_position_change_handlers_.contains(&widget);
}

void WidgetEventDispatcher::dispatchScrollPositionChange(
    Widget& widget, ScrollPosition previous, ScrollPosition current) {
  auto it = scroll_position_change_handlers_.find(&widget);
  if (it == scroll_position_change_handlers_.end()) return;
  // Keep the original callable alive if the callback changes registration.
  // A std::function copy could allocate during scroll dispatch and would lose
  // mutable-capture updates; sharing pays for storage at registration instead.
  std::shared_ptr<ScrollHandler> handler = (*it).second;
  (*handler)(previous, current);
}

void WidgetEventDispatcher::clearHandlers(Widget& widget) {
  clearInteractiveChangeHandler(widget);
  clearScrollPositionChangeHandler(widget);
}

void WidgetEventDispatcher::moveHandlers(Widget& from, Widget& to) {
  if (&from == &to) return;
  auto it = interactive_change_handlers_.find(&from);
  if (it != interactive_change_handlers_.end()) {
    interactive_change_handlers_[&to] = std::move((*it).second);
    interactive_change_handlers_.erase(&from);
  }
  auto scroll_it = scroll_position_change_handlers_.find(&from);
  if (scroll_it != scroll_position_change_handlers_.end()) {
    std::shared_ptr<ScrollHandler> handler = std::move((*scroll_it).second);
    scroll_position_change_handlers_.erase(&from);
    scroll_position_change_handlers_[&to] = std::move(handler);
  }
}

}  // namespace roo_windows