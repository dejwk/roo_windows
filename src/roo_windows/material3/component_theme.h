#pragma once

#include "roo_windows/material3/color_token.h"

namespace roo_windows::material3 {

/// Selects neutral fills for the three Material card styles.
struct CardTheme {
  ColorToken elevatedContainer = ColorToken::kSurfaceContainerLow;
  ColorToken filledContainer = ColorToken::kSurfaceContainerHighest;
  ColorToken outlinedContainer = ColorToken::kSurface;
};

/// Collects application-wide Material component surface defaults.
///
/// Phase 1 contains only cards. Later component-theme phases append their
/// groups here, keeping existing aggregate initialization source-compatible.
struct ComponentTheme {
  CardTheme card;
};

static_assert(sizeof(CardTheme) == 3,
              "Card surface defaults must occupy three compact tokens.");
static_assert(alignof(CardTheme) == 1,
              "Card surface defaults must not introduce padding.");
static_assert(sizeof(ComponentTheme) == 3,
              "Component theme must contain only implemented card defaults.");
static_assert(alignof(ComponentTheme) == 1,
              "Component theme must not introduce padding.");

}  // namespace roo_windows::material3
