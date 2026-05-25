#include "roo_windows/core/widget_event_dispatcher.h"

#include <utility>

namespace roo_windows {

void WidgetEventDispatcher::setInteractiveChangeHandler(
    Widget& widget, std::function<void()> handler) {
  if (handler == nullptr) {
    clearInteractiveChangeHandler(widget);
    return;
  }
  interactive_change_handlers_[&widget] = std::move(handler);
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
  (*it).second();
}

void WidgetEventDispatcher::clearHandlers(Widget& widget) {
  clearInteractiveChangeHandler(widget);
}

}  // namespace roo_windows