#include "roo_windows/core/application_context.h"

#include "roo_windows/core/scroll_connection.h"
#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
#include "roo_windows/core/editor_destination.h"
#include "roo_windows/internal/editor_destination_factory.h"
#endif

namespace roo_windows {

ApplicationContext::ApplicationContext(
    roo_scheduler::SchedulerClient& scheduler, const Theme& theme,
    const KeyboardColorTheme& keyboard_color_theme,
    DesignSystem default_design_system)
    : scheduler_(scheduler),
      theme_(theme),
      keyboard_color_theme_(keyboard_color_theme),
      presentations_(*this),
      animations_(*this),
      lifetime_(new Lifetime(*this)),
      default_design_system_(default_design_system) {}

internal::ScrollConnectionRegistry& ApplicationContext::scrollConnections() {
  if (scroll_connections_ == nullptr)
    scroll_connections_ =
        std::make_unique<internal::ScrollConnectionRegistry>();
  return *scroll_connections_;
}

#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
EditorDestination& ApplicationContext::editorDestination() {
  if (editor_destination_ == nullptr) {
    switch (default_design_system_) {
      case DesignSystem::kMaterial2:
#if ROO_WINDOWS_ENABLE_MATERIAL2
        editor_destination_ = internal::CreateMaterial2EditorDestination(*this);
#else
        CHECK(false) << "Material 2 is disabled";
#endif
        break;
      case DesignSystem::kMaterial3:
        editor_destination_ = internal::CreateMaterial3EditorDestination(*this);
        break;
    }
  }
  CHECK(editor_destination_ != nullptr);
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
