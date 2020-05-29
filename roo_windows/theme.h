#pragma once

#include "roo_display/core/color.h"

#include "roo_smooth_fonts/NotoSans_Regular/90.h"
#include "roo_smooth_fonts/NotoSans_Regular/60.h"
#include "roo_smooth_fonts/NotoSans_Regular/40.h"
#include "roo_smooth_fonts/NotoSans_Regular/27.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_smooth_fonts/NotoSans_Regular/12.h"

namespace roo_windows {

struct ColorTheme {
  roo_display::Color primary;
  roo_display::Color primaryVariant;
  roo_display::Color secondary;
  roo_display::Color secondaryVariant;
  roo_display::Color background;
  roo_display::Color surface;
  roo_display::Color error;
  roo_display::Color onPrimary;
  roo_display::Color onSecondary;
  roo_display::Color onBackground;
  roo_display::Color onSurface;
  roo_display::Color onError;
};

struct FontTheme {
  const roo_display::Font* h1;
  const roo_display::Font* h2;
  const roo_display::Font* h3;
  const roo_display::Font* h4;
  const roo_display::Font* h5;
  const roo_display::Font* h6;
  const roo_display::Font* subtitle1;
  const roo_display::Font* subtitle2;
  const roo_display::Font* body1;
  const roo_display::Font* body2;
  const roo_display::Font* button;
  const roo_display::Font* caption;
  const roo_display::Font* overline;
};

struct Theme {
  struct ColorTheme color;
  struct FontTheme font;
};

const Theme& defaultTheme() {
  static Theme theme = {
    .color = {
      .primary = roo_display::Color(0xFF6200EE),
      .primaryVariant = roo_display::Color(0xFF3700B3),
      .secondary = roo_display::Color(0xFF03DAC6),
      .secondaryVariant = roo_display::Color(0xFF018786),
      .background = roo_display::Color(0xFFFFFFFF),
      .surface = roo_display::Color(0xFFFFFFFF),
      .error = roo_display::Color(0xFFB00020),
      .onPrimary = roo_display::Color(0xFFFFFFFF),
      .onSecondary = roo_display::Color(0xFF000000),
      .onBackground = roo_display::Color(0xFF000000),
      .onSurface = roo_display::Color(0xFF000000),
      .onError = roo_display::Color(0xFFFFFFFF)
    },
    .font = {
      .h1 = &font_NotoSans_Regular_60(),
      .h2 = &font_NotoSans_Regular_60(),
      .h3 = &font_NotoSans_Regular_40(),
      .h4 = &font_NotoSans_Regular_40(),
      .h5 = &font_NotoSans_Regular_27(),
      .h6 = &font_NotoSans_Regular_18(),
      .subtitle1 = &font_NotoSans_Regular_18(),
      .subtitle2 = &font_NotoSans_Regular_12(),
      .body1 = &font_NotoSans_Regular_18(),
      .body2 = &font_NotoSans_Regular_12(),
      .button = &font_NotoSans_Regular_18(),
      .caption = &font_NotoSans_Regular_12(),
      .overline = &font_NotoSans_Regular_12(),
    }
  };
  return theme;
}

}  // namespace roo_windows