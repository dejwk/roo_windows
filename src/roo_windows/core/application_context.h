#pragma once

#include "roo_scheduler.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget_event_dispatcher.h"

namespace roo_windows {

/// Bundles application-scoped runtime services shared by widgets.
///
/// `Application` owns one context and initializes it from the surrounding
/// `Environment`. Widgets will eventually consume this surface instead of
/// resolving those services indirectly.
class ApplicationContext {
 public:
  /// Creates a context borrowing the supplied scheduler and themes.
  ApplicationContext(roo_scheduler::Scheduler& scheduler, const Theme& theme,
                     const KeyboardColorTheme& keyboard_color_theme);

  /// Returns the scheduler used for animations and deferred work.
  roo_scheduler::Scheduler& scheduler() const;

  /// Returns the active visual theme.
  const Theme& theme() const;

  /// Returns the keyboard color theme.
  const KeyboardColorTheme& keyboardColorTheme() const;

  /// Returns the widget-event dispatcher.
  WidgetEventDispatcher& widgetEvents();

  /// Returns the widget-event dispatcher.
  const WidgetEventDispatcher& widgetEvents() const;

 private:
  roo_scheduler::Scheduler& scheduler_;
  const Theme& theme_;
  const KeyboardColorTheme& keyboard_color_theme_;
  WidgetEventDispatcher widget_events_;
};

}  // namespace roo_windows