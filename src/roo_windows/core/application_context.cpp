#include "roo_windows/core/application_context.h"

namespace roo_windows {

ApplicationContext::ApplicationContext(
    roo_scheduler::Scheduler& scheduler, const Theme& theme,
    const KeyboardColorTheme& keyboard_color_theme)
    : scheduler_(scheduler),
      theme_(theme),
      keyboard_color_theme_(keyboard_color_theme) {}

roo_scheduler::Scheduler& ApplicationContext::scheduler() const {
  return scheduler_;
}

const Theme& ApplicationContext::theme() const { return theme_; }

const KeyboardColorTheme& ApplicationContext::keyboardColorTheme() const {
  return keyboard_color_theme_;
}

WidgetEventDispatcher& ApplicationContext::widgetEvents() {
  return widget_events_;
}

const WidgetEventDispatcher& ApplicationContext::widgetEvents() const {
  return widget_events_;
}

}  // namespace roo_windows