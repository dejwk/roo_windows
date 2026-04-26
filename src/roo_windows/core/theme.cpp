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
                .primaryContainer = roo_display::Color(0xFFEADDFF),
                .onPrimaryContainer = roo_display::Color(0xFF21005D),
                .secondary = roo_display::Color(0xFF03DAC6),
                .secondaryVariant = roo_display::Color(0xFF018786),
                .secondaryContainer = roo_display::Color(0xFFE8DEF8),
                .onSecondaryContainer = roo_display::Color(0xFF1D192B),
                .tertiary = roo_display::Color(0xFF7D5260),
                .onTertiary = roo_display::Color(0xFFFFFFFF),
                .tertiaryContainer = roo_display::Color(0xFFFFD8E4),
                .onTertiaryContainer = roo_display::Color(0xFF31111D),
                .background = roo_display::Color(0xFFFFFFFF),
                .onBackground = roo_display::Color(0xFF1C1B1F),
                .surface = roo_display::Color(0xFFFFFFFF),
                .surfaceContainerLowest = roo_display::Color(0xFFFFFFFF),
                .surfaceContainerLow = roo_display::Color(0xFFF7F2FA),
                .surfaceContainer = roo_display::Color(0xFFF3EDF7),
                .surfaceContainerHigh = roo_display::Color(0xFFECE6F0),
                .surfaceContainerHighest = roo_display::Color(0xFFE6E0E9),
                .onSurface = roo_display::Color(0xFF1C1B1F),
                .surfaceVariant = roo_display::Color(0xFFE7E0EC),
                .onSurfaceVariant = roo_display::Color(0xFF49454F),
                .error = roo_display::Color(0xFFB00020),
                .errorContainer = roo_display::Color(0xFFF9DEDC),
                .onPrimary = roo_display::Color(0xFFFFFFFF),
                .onSecondary = roo_display::Color(0xFF000000),
                .onError = roo_display::Color(0xFFFFFFFF),
                .onErrorContainer = roo_display::Color(0xFF410E0B),
                .outline = roo_display::Color(0xFF79747E),
                .outlineVariant = roo_display::Color(0xFFCAC4D0),
                .inverseSurface = roo_display::Color(0xFF313033),
                .inverseOnSurface = roo_display::Color(0xFFF4EFF4),
                .inversePrimary = roo_display::Color(0xFFD0BCFF),
                .surfaceTint = roo_display::Color(0xFF6750A4)},
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
