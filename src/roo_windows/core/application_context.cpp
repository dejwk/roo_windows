#include "roo_windows/core/application_context.h"

#include "roo_windows/core/scroll_connection.h"
#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
#include "roo_windows/keyboard/editor_destination.h"
#endif

namespace roo_windows {

ApplicationContext::ApplicationContext(
    roo_scheduler::SchedulerClient& scheduler, const Theme& theme,
    const KeyboardColorTheme& keyboard_color_theme)
    : scheduler_(scheduler),
      theme_(theme),
      keyboard_color_theme_(keyboard_color_theme),
      presentations_(*this),
      animations_(*this),
      lifetime_(new Lifetime(*this)) {}

internal::ScrollConnectionRegistry& ApplicationContext::scrollConnections() {
  if (scroll_connections_ == nullptr)
    scroll_connections_ =
        std::make_unique<internal::ScrollConnectionRegistry>();
  return *scroll_connections_;
}

#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
EditorDestination& ApplicationContext::editorDestination() {
  if (editor_destination_ == nullptr) {
    editor_destination_ = std::make_unique<EditorDestination>(*this);
  }
  return *editor_destination_;
}

#endif

ApplicationContext::~ApplicationContext() {
#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
  editor_destination_.reset();
#endif
  animations_.stop();
  presentations_.stop();
  lifetime_->context = nullptr;
  lifetime_->release();
}

roo_scheduler::SchedulerClient& ApplicationContext::scheduler() const {
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
