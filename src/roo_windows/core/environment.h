#pragma once

#include "roo_scheduler.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

class Environment {
 public:
  Environment(roo_scheduler::Scheduler& scheduler,
              const Theme& theme = DefaultTheme(),
              const KeyboardColorTheme& kb_theme = DefaultKeyboardColorTheme())
      : scheduler_(scheduler), theme_(theme), kb_theme_(kb_theme) {}

  const Theme& theme() const { return theme_; }
  const KeyboardColorTheme& keyboardColorTheme() const { return kb_theme_; }

  roo_scheduler::Scheduler& scheduler() const { return scheduler_; }

 private:
  roo_scheduler::Scheduler& scheduler_;
  const Theme& theme_;
  const KeyboardColorTheme& kb_theme_;
};

}  // namespace roo_windows
