#pragma once

#include "roo_scheduler.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

/// Shared, application-wide services injected into every widget at
/// construction.
///
/// Carries references to a `roo_scheduler::Scheduler` (used for animations and
/// deferred work), the active visual `Theme`, and the `KeyboardColorTheme`. The
/// environment is normally held by the `Application` and outlives the widgets
/// that consume it.
class Environment {
 public:
  Environment(roo_scheduler::Scheduler& scheduler,
              const Theme& theme = DefaultTheme(),
              const KeyboardColorTheme& kb_theme = DefaultKeyboardColorTheme())
      : scheduler_(scheduler), theme_(theme), kb_theme_(kb_theme) {}

  /// Returns the active visual theme.
  const Theme& theme() const { return theme_; }
  /// Returns the on-screen keyboard color theme.
  const KeyboardColorTheme& keyboardColorTheme() const { return kb_theme_; }

  /// Returns the scheduler used for animations and deferred work.
  roo_scheduler::Scheduler& scheduler() const { return scheduler_; }

 private:
  roo_scheduler::Scheduler& scheduler_;
  const Theme& theme_;
  const KeyboardColorTheme& kb_theme_;
};

}  // namespace roo_windows
