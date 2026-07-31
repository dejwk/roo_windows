#pragma once

#include "roo_display.h"
#include "roo_display/color/color.h"
#include "roo_display/font/smooth_font.h"
#include "roo_windows/config.h"
#include "roo_windows/core/framework_theme.h"

namespace roo_windows::material3 {
enum class ColorToken : uint8_t;
struct Material3Theme;
}  // namespace roo_windows::material3

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

// NOTE: fonts (particularly, large fonts) take up a lot of PROGMEM space, which
// is why we resolve them at compile time here, so that only the fonts that are
// actually referenced get compiled into the program.

#include "roo_windows/material2/typography.h"

namespace roo_windows {

inline const roo_display::Font& font_h1() {
  return material2::text_style_h1().font();
}

inline const roo_display::Font& font_h2() {
  return material2::text_style_h2().font();
}

inline const roo_display::Font& font_h3() {
  return material2::text_style_h3().font();
}

inline const roo_display::Font& font_h4() {
  return material2::text_style_h4().font();
}

inline const roo_display::Font& font_h5() {
  return material2::text_style_h5().font();
}

inline const roo_display::Font& font_h6() {
  return material2::text_style_h6().font();
}

inline const roo_display::Font& font_subtitle1() {
  return material2::text_style_subtitle1().font();
}

inline const roo_display::Font& font_subtitle2() {
  return material2::text_style_subtitle2().font();
}

inline const roo_display::Font& font_body1() {
  return material2::text_style_body1().font();
}

inline const roo_display::Font& font_body2() {
  return material2::text_style_body2().font();
}

inline const roo_display::Font& font_button() {
  return material2::text_style_button().font();
}

inline const roo_display::Font& font_caption() {
  return material2::text_style_caption().font();
}

inline const roo_display::Font& font_overline() {
  return material2::text_style_overline().font();
}

struct Theme {
  // The framework contract is owned by every application theme, independent
  // of whether a Material 3 theme is installed.
  FrameworkTheme framework;

  // Non-owning typed design-system slot. The pointed-to object must outlive
  // this Theme and widgets that use it.
  const material3::Material3Theme* material3_theme = nullptr;

  bool hasMaterial3Theme() const { return material3_theme != nullptr; }

  // Precondition: hasMaterial3Theme(). This asserts in debug builds; a null
  // slot is an application configuration error in release builds as well.
  const material3::Material3Theme& material3Theme() const;
};

static_assert(sizeof(decltype(Theme::material3_theme)) == sizeof(void*),
              "A design-system theme slot must remain a pointer.");

struct KeyboardColorTheme {
  roo_display::Color background;
  roo_display::Color normalButton;
  roo_display::Color modifierButton;
  roo_display::Color acceptButton;
  roo_display::Color text;
};

/// Returns the framework's default Material 3 theme.
const Theme& DefaultTheme();
/// Returns the framework's default on-screen keyboard color theme.
const KeyboardColorTheme& DefaultKeyboardColorTheme();

}  // namespace roo_windows
