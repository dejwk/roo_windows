#pragma once

#include <cassert>
#include <cstdint>

#include "roo_scheduler.h"
#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget_event_dispatcher.h"

namespace roo_windows {

class Widget;

/// Bundles application-scoped runtime services shared by widgets.
///
/// `Application` owns one context and initializes it from the surrounding
/// `Environment`. Widgets use this surface for runtime services instead of
/// resolving them indirectly.
class ApplicationContext {
 public:
  /// Creates a context borrowing the supplied scheduler and themes.
  ApplicationContext(roo_scheduler::Scheduler& scheduler, const Theme& theme,
                     const KeyboardColorTheme& keyboard_color_theme);

  /// Invalidates outstanding widget handles before runtime services are
  /// destroyed. Caller-owned widgets may still be inspected through
  /// context-independent state accessors and destroyed afterward.
  ~ApplicationContext();

  ApplicationContext(const ApplicationContext&) = delete;
  ApplicationContext& operator=(const ApplicationContext&) = delete;
  ApplicationContext(ApplicationContext&&) = delete;
  ApplicationContext& operator=(ApplicationContext&&) = delete;

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

  /// Returns the application-owned keyboard-focus service.
  FocusManager& focus() { return focus_; }

  /// Returns the application-owned keyboard-focus service.
  const FocusManager& focus() const { return focus_; }

 private:
  friend class Widget;

  struct Lifetime {
    explicit Lifetime(ApplicationContext& owner) : context(&owner) {}

    void retain() { ++reference_count; }
    void release() {
      assert(reference_count > 0);
      if (--reference_count == 0) delete this;
    }

    ApplicationContext* context;
    // Application and Widget lifetimes are confined to their UI thread, so
    // this deliberately avoids the cost of an atomic reference count. There
    // is one heap-allocated Lifetime per ApplicationContext, not per Widget.
    uint32_t reference_count = 1;

   private:
    ~Lifetime() = default;
  };

  roo_scheduler::Scheduler& scheduler_;
  const Theme& theme_;
  const KeyboardColorTheme& keyboard_color_theme_;
  WidgetEventDispatcher widget_events_;
  FocusManager focus_;
  Lifetime* lifetime_;
};

}  // namespace roo_windows
