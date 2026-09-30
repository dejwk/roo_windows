#pragma once

#include "roo_scheduler.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

/// Shared bootstrap configuration borrowed by `Application` and its runtime
/// `ApplicationContext`.
///
/// Carries references to a `roo_scheduler::SchedulingService` (used for
/// animations and deferred work), the active visual `Theme`, and the
/// `KeyboardColorTheme`. The environment is normally held by the `Application`
/// and outlives the runtime context and widgets that consume those services.
class Environment {
 public:
  Environment(roo_scheduler::SchedulingService& scheduler,
              const Theme& theme = DefaultTheme(),
              const KeyboardColorTheme& kb_theme = DefaultKeyboardColorTheme())
      : scheduler_(scheduler), theme_(theme), kb_theme_(kb_theme) {}

  /// Returns the active visual theme.
  const Theme& theme() const { return theme_; }
  /// Returns the on-screen keyboard color theme.
  const KeyboardColorTheme& keyboardColorTheme() const { return kb_theme_; }

  /// Returns scheduling and cancellation access for animations and deferred
  /// work.
  roo_scheduler::SchedulerClient& scheduler() const { return scheduler_; }

 private:
  friend class Application;

  roo_scheduler::SchedulingService& scheduler_;
  const Theme& theme_;
  const KeyboardColorTheme& kb_theme_;
};

}  // namespace roo_windows
