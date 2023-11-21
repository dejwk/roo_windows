#pragma once

#include "roo_windows/core/theme.h"

namespace roo_windows {

class Environment {
 public:
  Environment(const Theme& theme = DefaultTheme(),
              const KeyboardColorTheme& kb_theme = DefaultKeyboardColorTheme())
      : theme_(theme), kb_theme_(kb_theme) {}

  const Theme& theme() const { return theme_; }
  const KeyboardColorTheme& keyboardColorTheme() const { return kb_theme_; }

 private:
  const Theme& theme_;
  const KeyboardColorTheme& kb_theme_;
};

}  // namespace roo_windows
