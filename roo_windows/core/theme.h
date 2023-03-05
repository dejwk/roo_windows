#pragma once

#include "roo_display/core/color.h"
#include "roo_display/font/smooth_font.h"

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

  roo_display::Color highlighterColor(roo_display::Color bg) const {
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

  uint8_t pressAnimationOpacity(roo_display::Color bg) const {
    if (bg == color.surface) {
      return state.activatedOnSurface + state.pressedOnSurface;
    } else if (bg == color.primary || bg == color.primaryVariant) {
      return state.activatedOnPrimary + state.pressedOnPrimary;
    } else if (bg == color.secondary || bg == color.secondaryVariant) {
      return state.activatedOnSecondary + state.pressedOnSecondary;
    } else if (bg == color.error) {
      return state.activatedOnError + state.pressedOnError;
    } else {
      return state.activatedOnBackground + state.pressedOnBackground;
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