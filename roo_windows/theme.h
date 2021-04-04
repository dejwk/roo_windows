#pragma once

#include "roo_display/core/color.h"
#include "roo_smooth_fonts/NotoSans_Regular/12.h"
#include "roo_smooth_fonts/NotoSans_Regular/18.h"
#include "roo_smooth_fonts/NotoSans_Regular/27.h"
#include "roo_smooth_fonts/NotoSans_Regular/40.h"
#include "roo_smooth_fonts/NotoSans_Regular/60.h"
#include "roo_smooth_fonts/NotoSans_Regular/90.h"

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

  roo_display::Color defaultColor(roo_display::Color bg) const {
    if (bg == surface) {
      return onSurface;
    } else if (bg == primary || bg == primaryVariant) {
      return onPrimary;
    } else if (bg == secondary || bg == secondaryVariant) {
      return onSecondary;
    } else if (bg == error) {
      return onError;
    } else {
      return onBackground;
    }
  }

  roo_display::Color defaultColorActivated(roo_display::Color bg) const {
    if (bg == surface) {
      return primary;
    } else if (bg == primary || bg == primaryVariant) {
      return onPrimary;
    } else if (bg == secondary || bg == secondaryVariant) {
      return onSecondary;
    } else if (bg == error) {
      return onError;
    } else {
      return primary;
    }
  }
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
  struct FontTheme font;
  struct StateOpacityTheme state;

  uint8_t hoverOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.hoverOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.hoverOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.hoverOnSecondary;
    } else if (bg == color.error) {
      return state.hoverOnError;
    } else {
      return state.hoverOnBackground;
    }
  }

  uint8_t focusOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.focusOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.focusOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.focusOnSecondary;
    } else if (bg == color.error) {
      return state.focusOnError;
    } else {
      return state.focusOnBackground;
    }
  }

  uint8_t selectedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.selectedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.selectedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.selectedOnSecondary;
    } else if (bg == color.error) {
      return state.selectedOnError;
    } else {
      return state.selectedOnBackground;
    }
  }

  uint8_t activatedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.activatedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.activatedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.activatedOnSecondary;
    } else if (bg == color.error) {
      return state.activatedOnError;
    } else {
      return state.activatedOnBackground;
    }
  }

  uint8_t pressedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.pressedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.pressedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.pressedOnSecondary;
    } else if (bg == color.error) {
      return state.pressedOnError;
    } else {
      return state.pressedOnBackground;
    }
  }

  uint8_t draggedOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.draggedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.draggedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.draggedOnSecondary;
    } else if (bg == color.error) {
      return state.draggedOnError;
    } else {
      return state.draggedOnBackground;
    }
  }
};

const Theme& DefaultTheme();

}  // namespace roo_windows