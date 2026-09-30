#pragma once

#include <cassert>
#include <cstdint>

#include "roo_scheduler.h"
#include "roo_windows/core/animation_registry.h"
#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/presentation_registry.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget_event_dispatcher.h"

namespace roo_windows {

class Application;
class Widget;
namespace internal {
class ScrollConnectionRegistry;
class ScrollConnection;
}  // namespace internal

/// Bundles application-scoped runtime services shared by widgets.
///
/// `Application` owns one context and initializes it from the surrounding
/// `Environment`. Widgets use this surface for runtime services instead of
/// resolving them indirectly.
class ApplicationContext {
 public:
  /// Creates a context borrowing the supplied scheduler and themes.
  ApplicationContext(roo_scheduler::SchedulerClient& scheduler,
                     const Theme& theme,
                     const KeyboardColorTheme& keyboard_color_theme);

  /// Invalidates outstanding widget handles before runtime services are
  /// destroyed. Caller-owned widgets may still be inspected through
  /// context-independent state accessors and destroyed afterward.
  ~ApplicationContext();

  ApplicationContext(const ApplicationContext&) = delete;
  ApplicationContext& operator=(const ApplicationContext&) = delete;
  ApplicationContext(ApplicationContext&&) = delete;
  ApplicationContext& operator=(ApplicationContext&&) = delete;

  /// Returns scheduling and cancellation access for animations and deferred
  /// work. Dispatch methods are available only to the application runtime. UI
  /// callbacks must return without blocking or driving nested dispatch.
  roo_scheduler::SchedulerClient& scheduler() const;

  /// Returns the active visual theme.
  const Theme& theme() const;

  /// Returns the keyboard color theme.
  const KeyboardColorTheme& keyboardColorTheme() const;

  /// Returns the widget-event dispatcher.
  WidgetEventDispatcher& widgetEvents();

  /// Returns the widget-event dispatcher.
  const WidgetEventDispatcher& widgetEvents() const;

  /// Returns the application-owned widget animation service.
  AnimationRegistry& animations() { return animations_; }

  /// Returns the application-owned widget animation service.
  const AnimationRegistry& animations() const { return animations_; }

  /// Returns the application-owned keyboard-focus service.
  FocusManager& focus() { return focus_; }

  /// Returns the application-owned presentation notification service.
  PresentationRegistry& presentations() { return presentations_; }

  /// Returns the application-owned keyboard-focus service.
  const FocusManager& focus() const { return focus_; }

  /// Creates the optional scroll service on first registration.
  internal::ScrollConnectionRegistry& scrollConnections();
  /// Returns the service without allocating it.
  internal::ScrollConnectionRegistry* scrollConnectionsIfPresent() const {
    return scroll_connections_.get();
  }

 private:
  friend class AnimationRegistry;
  friend class Application;
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

  roo_scheduler::SchedulerClient& scheduler_;
  const Theme& theme_;
  const KeyboardColorTheme& keyboard_color_theme_;
  WidgetEventDispatcher widget_events_;
  FocusManager focus_;
  PresentationRegistry presentations_;
  std::unique_ptr<internal::ScrollConnectionRegistry> scroll_connections_;
  AnimationRegistry animations_;
  Application* frame_driver_ = nullptr;
  Lifetime* lifetime_;
};

}  // namespace roo_windows
