#pragma once

#include "roo_display.h"
#include "roo_display/color/color.h"
#include "roo_display/font/smooth_font.h"
#include "roo_windows/config.h"
#include "roo_windows/core/default_theme.h"
#include "roo_windows/core/framework_theme.h"
#if ROO_WINDOWS_ENABLE_MATERIAL2
#include "roo_windows/material2/theme.h"
#endif
#include "roo_windows/material3/theme.h"

#ifndef ROO_WINDOWS_ZOOM

#define ROO_DISPLAY_DPI 180

#ifdef ROO_DISPLAY_DPI

#if (ROO_DISPLAY_DPI >= 270)
#define ROO_WINDOWS_ZOOM 200
#elif (ROO_DISPLAY_DPI >= 200)
#define ROO_WINDOWS_ZOOM 150
#elif (ROO_DISPLAY_DPI >= 135)
#define ROO_WINDOWS_ZOOM 100
#else
#define ROO_WINDOWS_ZOOM 75
#endif

#else  // ROO_DISPLAY_DPI not defined

#define ROO_WINDOWS_ZOOM 100

#endif
#endif

/// `ROO_WINDOWS_ICON_SIZE` defines icon size (from roo_icons) used by
/// clickable UI elements such as buttons.

#if (ROO_WINDOWS_ZOOM >= 200)
#define ROO_WINDOWS_ICON_SIZE 48
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 36

namespace roo_windows {
template <typename T>
T constexpr Scaled(T in) {
  return in * 2;
}
}  // namespace roo_windows

#elif (ROO_WINDOWS_ZOOM >= 150)
#define ROO_WINDOWS_ICON_SIZE 36
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 24

namespace roo_windows {
template <typename T>
constexpr T Scaled(T in) {
  return in * 3 / 2;
}
}  // namespace roo_windows

#elif (ROO_WINDOWS_ZOOM >= 100)
#define ROO_WINDOWS_ICON_SIZE 24
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 18

namespace roo_windows {
template <typename T>
T constexpr Scaled(T in) {
  return in;
}
}  // namespace roo_windows

#else
#define ROO_WINDOWS_ICON_SIZE 18
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 24

namespace roo_windows {
template <typename T>
T constexpr Scaled(T in) {
  return in * 3 / 4;
}
}  // namespace roo_windows

#endif

#define __ROO_CONCAT6__(a, b, c, d, e, f) a##b##c##d##e##f
#define __ROO_CONCAT6(a, b, c, d, e, f) __ROO_CONCAT6__(a, b, c, d, e, f)

#define __ROO_CONCAT2__(a, b) a##b
#define __ROO_CONCAT2(a, b) __ROO_CONCAT2__(a, b)

#define SCALED_ROO_ICON(family, name) \
  __ROO_CONCAT6(ic_, family, _, ROO_WINDOWS_ICON_SIZE, _, name)()

#ifndef ROO_LANG
#define ROO_LANG ROO_LANG_en
#endif

namespace roo_windows {

/// Application appearance, owning each supported design system's settings.
/// Mutate fields directly, then notify each affected Application with
/// invalidateTheme(). Keep this object alive while environments borrow it.
struct Theme {
  FrameworkTheme framework;
  material3::Material3Theme material3;
#if ROO_WINDOWS_ENABLE_MATERIAL2
  material2::Material2Theme material2;

  /// Returns independently owned Material 2 appearance settings.
  material2::Material2Theme& material2Theme() { return material2; }

  /// Returns independently owned Material 2 appearance settings.
  const material2::Material2Theme& material2Theme() const { return material2; }

#endif

  /// Returns the owned Material 3 appearance settings for customization.
  material3::Material3Theme& material3Theme() { return material3; }

  /// Returns the owned Material 3 appearance settings.
  const material3::Material3Theme& material3Theme() const { return material3; }

  /// Resolves application-owned framework root colors.
  roo_display::Color frameworkColor(FrameworkColorRole role) const {
    return framework.color(role);
  }

  /// Returns the current framework disabled-content opacity.
  uint8_t disabledContentOpacity() const {
    return framework.state.disabledContentOpacity;
  }
};

struct KeyboardColorTheme {
  roo_display::Color background;
  roo_display::Color normalButton;
  roo_display::Color modifierButton;
  roo_display::Color acceptButton;
  roo_display::Color text;
};

/// Returns the framework's default on-screen keyboard color theme.
const KeyboardColorTheme& DefaultKeyboardColorTheme();

}  // namespace roo_windows
