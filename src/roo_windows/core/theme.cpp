#include <assert.h>

#include "theme.h"

namespace roo_windows {

const material3::Material3Theme& Theme::material3Theme() const {
  assert(material3_theme != nullptr && "Material 3 theme is not installed");
  return *material3_theme;
}

const KeyboardColorTheme& DefaultKeyboardColorTheme() {
  static KeyboardColorTheme theme = {
      .background = roo_display::Color(0xFF303030),
      .normalButton = roo_display::Color(0xFF4C4C4C),
      .modifierButton = roo_display::Color(0xFF3C3C3C),
      .acceptButton = roo_display::Color(0xFF018786),
      .text = roo_display::Color(0xFFFFFFFF)};
  return theme;
}

}  // namespace roo_windows
