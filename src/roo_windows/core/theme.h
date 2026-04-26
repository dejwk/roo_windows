#pragma once

#include "roo_display.h"
#include "roo_display/color/color.h"
#include "roo_display/font/smooth_font.h"
#include "roo_fonts/NotoSans_Light/120.h"
#include "roo_fonts/NotoSans_Light/144.h"
#include "roo_fonts/NotoSans_Light/192.h"
#include "roo_fonts/NotoSans_Light/45.h"
#include "roo_fonts/NotoSans_Light/60.h"
#include "roo_fonts/NotoSans_Light/72.h"
#include "roo_fonts/NotoSans_Light/90.h"
#include "roo_fonts/NotoSans_Light/96.h"
#include "roo_fonts/NotoSans_Medium/11.h"
#include "roo_fonts/NotoSans_Medium/14.h"
#include "roo_fonts/NotoSans_Medium/15.h"
#include "roo_fonts/NotoSans_Medium/20.h"
#include "roo_fonts/NotoSans_Medium/21.h"
#include "roo_fonts/NotoSans_Medium/28.h"
#include "roo_fonts/NotoSans_Medium/30.h"
#include "roo_fonts/NotoSans_Medium/40.h"
#include "roo_fonts/NotoSans_Regular/10.h"
#include "roo_fonts/NotoSans_Regular/11.h"
#include "roo_fonts/NotoSans_Regular/12.h"
#include "roo_fonts/NotoSans_Regular/14.h"
#include "roo_fonts/NotoSans_Regular/15.h"
#include "roo_fonts/NotoSans_Regular/16.h"
#include "roo_fonts/NotoSans_Regular/18.h"
#include "roo_fonts/NotoSans_Regular/20.h"
#include "roo_fonts/NotoSans_Regular/21.h"
#include "roo_fonts/NotoSans_Regular/24.h"
#include "roo_fonts/NotoSans_Regular/26.h"
#include "roo_fonts/NotoSans_Regular/28.h"
#include "roo_fonts/NotoSans_Regular/32.h"
#include "roo_fonts/NotoSans_Regular/34.h"
#include "roo_fonts/NotoSans_Regular/36.h"
#include "roo_fonts/NotoSans_Regular/48.h"
#include "roo_fonts/NotoSans_Regular/51.h"
#include "roo_fonts/NotoSans_Regular/68.h"
#include "roo_fonts/NotoSans_Regular/72.h"
#include "roo_fonts/NotoSans_Regular/8.h"
#include "roo_fonts/NotoSans_Regular/9.h"
#include "roo_fonts/NotoSans_Regular/96.h"
#include "roo_windows/config.h"

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

#define ROO_WINDOWS_FONT_SIZE_H1 192
#define ROO_WINDOWS_FONT_SIZE_H2 120
#define ROO_WINDOWS_FONT_SIZE_H3 96
#define ROO_WINDOWS_FONT_SIZE_H4 68
#define ROO_WINDOWS_FONT_SIZE_H5 48
#define ROO_WINDOWS_FONT_SIZE_H6 40
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 32
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 28
#define ROO_WINDOWS_FONT_SIZE_BODY1 32
#define ROO_WINDOWS_FONT_SIZE_BODY2 28
#define ROO_WINDOWS_FONT_SIZE_BUTTON 28
#define ROO_WINDOWS_FONT_SIZE_CAPTION 24
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 20

#elif (ROO_WINDOWS_ZOOM >= 150)
#define ROO_WINDOWS_ICON_SIZE 36
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 24

namespace roo_windows {
template <typename T>
constexpr T Scaled(T in) {
  return in * 3 / 2;
}
}  // namespace roo_windows

#define ROO_WINDOWS_FONT_SIZE_H1 144
#define ROO_WINDOWS_FONT_SIZE_H2 90
#define ROO_WINDOWS_FONT_SIZE_H3 72
#define ROO_WINDOWS_FONT_SIZE_H4 51
#define ROO_WINDOWS_FONT_SIZE_H5 36
#define ROO_WINDOWS_FONT_SIZE_H6 30
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 24
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 21
#define ROO_WINDOWS_FONT_SIZE_BODY1 24
#define ROO_WINDOWS_FONT_SIZE_BODY2 21
#define ROO_WINDOWS_FONT_SIZE_BUTTON 21
#define ROO_WINDOWS_FONT_SIZE_CAPTION 18
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 15

#elif (ROO_WINDOWS_ZOOM >= 100)
#define ROO_WINDOWS_ICON_SIZE 24
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 18

#define ROO_WINDOWS_FONT_SIZE_H1 96
#define ROO_WINDOWS_FONT_SIZE_H2 60
#define ROO_WINDOWS_FONT_SIZE_H3 48
#define ROO_WINDOWS_FONT_SIZE_H4 34
#define ROO_WINDOWS_FONT_SIZE_H5 24
#define ROO_WINDOWS_FONT_SIZE_H6 20
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 16
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 14
#define ROO_WINDOWS_FONT_SIZE_BODY1 16
#define ROO_WINDOWS_FONT_SIZE_BODY2 14
#define ROO_WINDOWS_FONT_SIZE_BUTTON 14
#define ROO_WINDOWS_FONT_SIZE_CAPTION 12
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 10

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

#define ROO_WINDOWS_FONT_SIZE_H1 72
#define ROO_WINDOWS_FONT_SIZE_H2 45
#define ROO_WINDOWS_FONT_SIZE_H3 36
#define ROO_WINDOWS_FONT_SIZE_H4 26
#define ROO_WINDOWS_FONT_SIZE_H5 18
#define ROO_WINDOWS_FONT_SIZE_H6 15
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 12
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 11
#define ROO_WINDOWS_FONT_SIZE_BODY1 12
#define ROO_WINDOWS_FONT_SIZE_BODY2 11
#define ROO_WINDOWS_FONT_SIZE_BUTTON 11
#define ROO_WINDOWS_FONT_SIZE_CAPTION 9
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 8

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

namespace roo_windows {

inline const roo_display::Font& font_h1() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Light_,
                       ROO_WINDOWS_FONT_SIZE_H1)();
}

inline const roo_display::Font& font_h2() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Light_,
                       ROO_WINDOWS_FONT_SIZE_H2)();
}

inline const roo_display::Font& font_h3() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_H3)();
}

inline const roo_display::Font& font_h4() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_H4)();
}

inline const roo_display::Font& font_h5() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_H5)();
}

inline const roo_display::Font& font_h6() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Medium_,
                       ROO_WINDOWS_FONT_SIZE_H6)();
}

inline const roo_display::Font& font_subtitle1() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_SUBTITLE1)();
}

inline const roo_display::Font& font_subtitle2() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Medium_,
                       ROO_WINDOWS_FONT_SIZE_SUBTITLE2)();
}

inline const roo_display::Font& font_body1() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_BODY1)();
}

inline const roo_display::Font& font_body2() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_BODY2)();
}

inline const roo_display::Font& font_button() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Medium_,
                       ROO_WINDOWS_FONT_SIZE_BUTTON)();
}

inline const roo_display::Font& font_caption() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_CAPTION)();
}

inline const roo_display::Font& font_overline() {
  return __ROO_CONCAT2(roo_display::font_NotoSans_Regular_,
                       ROO_WINDOWS_FONT_SIZE_OVERLINE)();
}

enum class ColorRole {
  kUndefined,  // No explicit role; inherit from parent.
  kPrimary,
  kOnPrimary,
  kPrimaryContainer,
  kOnPrimaryContainer,
  kSecondary,
  kOnSecondary,
  kSecondaryContainer,
  kOnSecondaryContainer,
  kTertiary,
  kOnTertiary,
  kTertiaryContainer,
  kOnTertiaryContainer,
  kBackground,
  kOnBackground,
  kSurface,
  kOnSurface,
  kSurfaceVariant,
  kOnSurfaceVariant,
  kError,
  kOnError,
  kErrorContainer,
  kOnErrorContainer,
  kOutline,
  kOutlineVariant,
  kInverseSurface,
  kInverseOnSurface,
  kInversePrimary,
  kSurfaceTint,
};

enum class InteractionState {
  kHover,
  kFocus,
  kSelected,
  kActivated,
  kPressed,
  kDragged,
};

struct ColorTheme {
  roo_display::Color primary;
  roo_display::Color primaryVariant;
  roo_display::Color primaryContainer;
  roo_display::Color onPrimaryContainer;
  roo_display::Color secondary;
  roo_display::Color secondaryVariant;
  roo_display::Color secondaryContainer;
  roo_display::Color onSecondaryContainer;
  roo_display::Color tertiary;
  roo_display::Color onTertiary;
  roo_display::Color tertiaryContainer;
  roo_display::Color onTertiaryContainer;
  roo_display::Color background;
  roo_display::Color onBackground;
  roo_display::Color surface;
  roo_display::Color onSurface;
  roo_display::Color surfaceVariant;
  roo_display::Color onSurfaceVariant;
  roo_display::Color error;
  roo_display::Color errorContainer;
  roo_display::Color onPrimary;
  roo_display::Color onSecondary;
  roo_display::Color onError;
  roo_display::Color onErrorContainer;
  roo_display::Color outline;
  roo_display::Color outlineVariant;
  roo_display::Color inverseSurface;
  roo_display::Color inverseOnSurface;
  roo_display::Color inversePrimary;
  roo_display::Color surfaceTint;

  roo_display::Color role(ColorRole role) const {
    switch (role) {
      case ColorRole::kUndefined:
        return roo_display::color::Transparent;
      case ColorRole::kPrimary:
        return primary;
      case ColorRole::kOnPrimary:
        return onPrimary;
      case ColorRole::kPrimaryContainer:
        return primaryContainer;
      case ColorRole::kOnPrimaryContainer:
        return onPrimaryContainer;
      case ColorRole::kSecondary:
        return secondary;
      case ColorRole::kOnSecondary:
        return onSecondary;
      case ColorRole::kSecondaryContainer:
        return secondaryContainer;
      case ColorRole::kOnSecondaryContainer:
        return onSecondaryContainer;
      case ColorRole::kTertiary:
        return tertiary;
      case ColorRole::kOnTertiary:
        return onTertiary;
      case ColorRole::kTertiaryContainer:
        return tertiaryContainer;
      case ColorRole::kOnTertiaryContainer:
        return onTertiaryContainer;
      case ColorRole::kBackground:
        return background;
      case ColorRole::kOnBackground:
        return onBackground;
      case ColorRole::kSurface:
        return surface;
      case ColorRole::kOnSurface:
        return onSurface;
      case ColorRole::kSurfaceVariant:
        return surfaceVariant;
      case ColorRole::kOnSurfaceVariant:
        return onSurfaceVariant;
      case ColorRole::kError:
        return error;
      case ColorRole::kOnError:
        return onError;
      case ColorRole::kErrorContainer:
        return errorContainer;
      case ColorRole::kOnErrorContainer:
        return onErrorContainer;
      case ColorRole::kOutline:
        return outline;
      case ColorRole::kOutlineVariant:
        return outlineVariant;
      case ColorRole::kInverseSurface:
        return inverseSurface;
      case ColorRole::kInverseOnSurface:
        return inverseOnSurface;
      case ColorRole::kInversePrimary:
        return inversePrimary;
      case ColorRole::kSurfaceTint:
        return surfaceTint;
    }
    return background;
  }

  roo_display::Color contentColorFor(ColorRole bg_role) const {
    switch (bg_role) {
      case ColorRole::kPrimary:
        return onPrimary;
      case ColorRole::kPrimaryContainer:
        return onPrimaryContainer;
      case ColorRole::kSecondary:
        return onSecondary;
      case ColorRole::kSecondaryContainer:
        return onSecondaryContainer;
      case ColorRole::kTertiary:
        return onTertiary;
      case ColorRole::kTertiaryContainer:
        return onTertiaryContainer;
      case ColorRole::kError:
        return onError;
      case ColorRole::kErrorContainer:
        return onErrorContainer;
      case ColorRole::kSurface:
        return onSurface;
      case ColorRole::kSurfaceVariant:
        return onSurfaceVariant;
      case ColorRole::kInverseSurface:
        return inverseOnSurface;
      case ColorRole::kBackground:
        return onBackground;
      default:
        return onBackground;
    }
  }

  roo_display::Color accentColorFor(ColorRole bg_role) const {
    switch (bg_role) {
      case ColorRole::kPrimary:
        return onPrimary;
      case ColorRole::kPrimaryContainer:
        return onPrimaryContainer;
      case ColorRole::kSecondary:
        return onSecondary;
      case ColorRole::kSecondaryContainer:
        return onSecondaryContainer;
      case ColorRole::kTertiary:
        return onTertiary;
      case ColorRole::kTertiaryContainer:
        return onTertiaryContainer;
      case ColorRole::kError:
        return onError;
      case ColorRole::kErrorContainer:
        return onErrorContainer;
      case ColorRole::kInverseSurface:
        return inversePrimary;
      default:
        return primary;
    }
  }

  ColorRole roleForColor(roo_display::Color bg) const {
    if (bg == surface) {
      return ColorRole::kSurface;
    } else if (bg == surfaceVariant) {
      return ColorRole::kSurfaceVariant;
    } else if (bg == primary || bg == primaryVariant) {
      return ColorRole::kPrimary;
    } else if (bg == primaryContainer) {
      return ColorRole::kPrimaryContainer;
    } else if (bg == secondary || bg == secondaryVariant) {
      return ColorRole::kSecondary;
    } else if (bg == secondaryContainer) {
      return ColorRole::kSecondaryContainer;
    } else if (bg == tertiary) {
      return ColorRole::kTertiary;
    } else if (bg == tertiaryContainer) {
      return ColorRole::kTertiaryContainer;
    } else if (bg == error) {
      return ColorRole::kError;
    } else if (bg == errorContainer) {
      return ColorRole::kErrorContainer;
    } else if (bg == inverseSurface) {
      return ColorRole::kInverseSurface;
    } else {
      return ColorRole::kBackground;
    }
  }

  roo_display::Color defaultColor(roo_display::Color bg) const {
    return contentColorFor(roleForColor(bg));
  }

  roo_display::Color highlighterColor(roo_display::Color bg) const {
    return accentColorFor(roleForColor(bg));
  }
};

struct StateOpacityTheme {
  const uint8_t disabled;

  const uint8_t hoverOnPrimary;
  const uint8_t hoverOnSecondary;
  const uint8_t hoverOnBackground;
  const uint8_t hoverOnSurface;
  const uint8_t hoverOnError;

  const uint8_t focusOnPrimary;
  const uint8_t focusOnSecondary;
  const uint8_t focusOnBackground;
  const uint8_t focusOnSurface;
  const uint8_t focusOnError;

  const uint8_t selectedOnPrimary;
  const uint8_t selectedOnSecondary;
  const uint8_t selectedOnBackground;
  const uint8_t selectedOnSurface;
  const uint8_t selectedOnError;

  const uint8_t activatedOnPrimary;
  const uint8_t activatedOnSecondary;
  const uint8_t activatedOnBackground;
  const uint8_t activatedOnSurface;
  const uint8_t activatedOnError;

  const uint8_t pressedOnPrimary;
  const uint8_t pressedOnSecondary;
  const uint8_t pressedOnBackground;
  const uint8_t pressedOnSurface;
  const uint8_t pressedOnError;

  const uint8_t draggedOnPrimary;
  const uint8_t draggedOnSecondary;
  const uint8_t draggedOnBackground;
  const uint8_t draggedOnSurface;
  const uint8_t draggedOnError;
};

struct Theme {
  struct ColorTheme color;
  struct StateOpacityTheme state;

  uint8_t opacity(ColorRole bg_role, InteractionState interaction) const {
    switch (interaction) {
      case InteractionState::kHover:
        switch (bg_role) {
          case ColorRole::kPrimary:
          case ColorRole::kPrimaryContainer:
            return state.hoverOnPrimary;
          case ColorRole::kSecondary:
          case ColorRole::kSecondaryContainer:
          case ColorRole::kTertiary:
          case ColorRole::kTertiaryContainer:
            return state.hoverOnSecondary;
          case ColorRole::kError:
          case ColorRole::kErrorContainer:
            return state.hoverOnError;
          case ColorRole::kSurface:
          case ColorRole::kSurfaceVariant:
          case ColorRole::kInverseSurface:
            return state.hoverOnSurface;
          default:
            return state.hoverOnBackground;
        }
      case InteractionState::kFocus:
        switch (bg_role) {
          case ColorRole::kPrimary:
          case ColorRole::kPrimaryContainer:
            return state.focusOnPrimary;
          case ColorRole::kSecondary:
          case ColorRole::kSecondaryContainer:
          case ColorRole::kTertiary:
          case ColorRole::kTertiaryContainer:
            return state.focusOnSecondary;
          case ColorRole::kError:
          case ColorRole::kErrorContainer:
            return state.focusOnError;
          case ColorRole::kSurface:
          case ColorRole::kSurfaceVariant:
          case ColorRole::kInverseSurface:
            return state.focusOnSurface;
          default:
            return state.focusOnBackground;
        }
      case InteractionState::kSelected:
        switch (bg_role) {
          case ColorRole::kPrimary:
          case ColorRole::kPrimaryContainer:
            return state.selectedOnPrimary;
          case ColorRole::kSecondary:
          case ColorRole::kSecondaryContainer:
          case ColorRole::kTertiary:
          case ColorRole::kTertiaryContainer:
            return state.selectedOnSecondary;
          case ColorRole::kError:
          case ColorRole::kErrorContainer:
            return state.selectedOnError;
          case ColorRole::kSurface:
          case ColorRole::kSurfaceVariant:
          case ColorRole::kInverseSurface:
            return state.selectedOnSurface;
          default:
            return state.selectedOnBackground;
        }
      case InteractionState::kActivated:
        switch (bg_role) {
          case ColorRole::kPrimary:
          case ColorRole::kPrimaryContainer:
            return state.activatedOnPrimary;
          case ColorRole::kSecondary:
          case ColorRole::kSecondaryContainer:
          case ColorRole::kTertiary:
          case ColorRole::kTertiaryContainer:
            return state.activatedOnSecondary;
          case ColorRole::kError:
          case ColorRole::kErrorContainer:
            return state.activatedOnError;
          case ColorRole::kSurface:
          case ColorRole::kSurfaceVariant:
          case ColorRole::kInverseSurface:
            return state.activatedOnSurface;
          default:
            return state.activatedOnBackground;
        }
      case InteractionState::kPressed:
        switch (bg_role) {
          case ColorRole::kPrimary:
          case ColorRole::kPrimaryContainer:
            return state.pressedOnPrimary;
          case ColorRole::kSecondary:
          case ColorRole::kSecondaryContainer:
          case ColorRole::kTertiary:
          case ColorRole::kTertiaryContainer:
            return state.pressedOnSecondary;
          case ColorRole::kError:
          case ColorRole::kErrorContainer:
            return state.pressedOnError;
          case ColorRole::kSurface:
          case ColorRole::kSurfaceVariant:
          case ColorRole::kInverseSurface:
            return state.pressedOnSurface;
          default:
            return state.pressedOnBackground;
        }
      case InteractionState::kDragged:
        switch (bg_role) {
          case ColorRole::kPrimary:
          case ColorRole::kPrimaryContainer:
            return state.draggedOnPrimary;
          case ColorRole::kSecondary:
          case ColorRole::kSecondaryContainer:
          case ColorRole::kTertiary:
          case ColorRole::kTertiaryContainer:
            return state.draggedOnSecondary;
          case ColorRole::kError:
          case ColorRole::kErrorContainer:
            return state.draggedOnError;
          case ColorRole::kSurface:
          case ColorRole::kSurfaceVariant:
          case ColorRole::kInverseSurface:
            return state.draggedOnSurface;
          default:
            return state.draggedOnBackground;
        }
    }
    return state.pressedOnBackground;
  }

  uint8_t hoverOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.hoverOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.hoverOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.hoverOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.hoverOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.hoverOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.hoverOnSecondary;
    } else if (bg == color.error) {
      return state.hoverOnError;
    } else if (bg == color.errorContainer) {
      return state.hoverOnError;
    } else {
      return state.hoverOnBackground;
    }
  }

  uint8_t focusOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.focusOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.focusOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.focusOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.focusOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.focusOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.focusOnSecondary;
    } else if (bg == color.error) {
      return state.focusOnError;
    } else if (bg == color.errorContainer) {
      return state.focusOnError;
    } else {
      return state.focusOnBackground;
    }
  }

  uint8_t selectedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.selectedOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.selectedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.selectedOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.selectedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.selectedOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.selectedOnSecondary;
    } else if (bg == color.error) {
      return state.selectedOnError;
    } else if (bg == color.errorContainer) {
      return state.selectedOnError;
    } else {
      return state.selectedOnBackground;
    }
  }

  uint8_t activatedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.activatedOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.activatedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.activatedOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.activatedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.activatedOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.activatedOnSecondary;
    } else if (bg == color.error) {
      return state.activatedOnError;
    } else if (bg == color.errorContainer) {
      return state.activatedOnError;
    } else {
      return state.activatedOnBackground;
    }
  }

  uint8_t pressedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.pressedOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.pressedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.pressedOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.pressedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.pressedOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.pressedOnSecondary;
    } else if (bg == color.error) {
      return state.pressedOnError;
    } else if (bg == color.errorContainer) {
      return state.pressedOnError;
    } else {
      return state.pressedOnBackground;
    }
  }

  uint8_t pressAnimationOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.pressedOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.pressedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.pressedOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.pressedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.pressedOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.pressedOnSecondary;
    } else if (bg == color.error) {
      return state.pressedOnError;
    } else if (bg == color.errorContainer) {
      return state.pressedOnError;
    } else {
      return state.pressedOnBackground;
    }
  }

  uint8_t draggedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.draggedOnSurface;
    } else if (bg == color.surfaceVariant || bg == color.inverseSurface) {
      return state.draggedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.draggedOnPrimary;
    } else if (bg == color.primaryContainer) {
      return state.draggedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.draggedOnSecondary;
    } else if (bg == color.secondaryContainer || bg == color.tertiary ||
               bg == color.tertiaryContainer) {
      return state.draggedOnSecondary;
    } else if (bg == color.error) {
      return state.draggedOnError;
    } else if (bg == color.errorContainer) {
      return state.draggedOnError;
    } else {
      return state.draggedOnBackground;
    }
  }
};

struct KeyboardColorTheme {
  roo_display::Color background;
  roo_display::Color normalButton;
  roo_display::Color modifierButton;
  roo_display::Color acceptButton;
  roo_display::Color text;
};

const Theme& DefaultTheme();
const KeyboardColorTheme& DefaultKeyboardColorTheme();

}  // namespace roo_windows