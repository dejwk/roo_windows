#include "theme.h"

#include "roo_display/color/color.h"
#include "roo_fonts/NotoSans_CondensedBold/15.h"
#include "roo_fonts/NotoSans_Regular/12.h"
#include "roo_fonts/NotoSans_Regular/15.h"
#include "roo_fonts/NotoSans_Regular/18.h"
#include "roo_fonts/NotoSans_Regular/27.h"
#include "roo_fonts/NotoSans_Regular/40.h"
#include "roo_fonts/NotoSans_Regular/60.h"
#include "roo_fonts/NotoSans_Regular/90.h"

namespace roo_windows {

const Theme& DefaultTheme() {
  static Theme theme = {
      .color = {.primary = roo_display::Color(0xFF6200EE),
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
                .onError = roo_display::Color(0xFFFFFFFF)},
      .state = {
          .disabled = 49,  // 38% of 128, used by TranslucencyFilter.

          .hoverOnPrimary = 20,
          .hoverOnSecondary = 20,
          .hoverOnBackground = 10,
          .hoverOnSurface = 10,
          .hoverOnError = 20,

          .focusOnPrimary = 61,
          .focusOnSecondary = 61,
          .focusOnBackground = 31,
          .focusOnSurface = 31,
          .focusOnError = 61,

          .selectedOnPrimary = 41,
          .selectedOnSecondary = 41,
          .selectedOnBackground = 20,
          .selectedOnSurface = 20,
          .selectedOnError = 41,

          .activatedOnPrimary = 61,
          .activatedOnSecondary = 61,
          .activatedOnBackground = 31,
          .activatedOnSurface = 31,
          .activatedOnError = 61,

          .pressedOnPrimary = 61,
          .pressedOnSecondary = 61,
          .pressedOnBackground = 31,
          .pressedOnSurface = 31,
          .pressedOnError = 61,

          .draggedOnPrimary = 41,
          .draggedOnSecondary = 41,
          .draggedOnBackground = 20,
          .draggedOnSurface = 20,
          .draggedOnError = 41,
      }};
  return theme;
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
