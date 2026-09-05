#pragma once

#include <stdint.h>

#include "roo_windows/material3/theme.h"

namespace roo_windows::material3::internal {

/// Material 3 menu geometry and color roles in Material dp units.
///
/// The two constants below are the local, reviewable transcription consumed by
/// menu rows and panels. Keeping every role in one table prevents individual
/// widgets from quietly inventing variant-specific values.
struct MenuTokens {
  int16_t min_width_dp;
  int16_t max_width_dp;
  int16_t horizontal_padding_dp;
  int16_t min_item_height_dp;
  int16_t icon_size_dp;
  int16_t trailing_gap_dp;
  int16_t group_gap_dp;
  int16_t divider_inset_dp;
  int16_t viewport_margin_dp;
  int16_t submenu_gutter_dp;
  int16_t panel_corner_radius_dp;
  int16_t item_outer_corner_radius_dp;
  int16_t item_inner_corner_radius_dp;
  uint8_t elevation;
  ColorToken panel_container;
  ColorToken panel_content;
  ColorToken selected_container;
  ColorToken selected_content;
  ColorToken divider;
};

inline constexpr MenuTokens kBaselineMenuTokens = {
    112,
    280,
    12,
    48,
    24,
    12,
    0,
    12,
    8,
    4,
    4,
    0,
    0,
    3,
    ColorToken::kSurfaceContainer,
    ColorToken::kOnSurface,
    ColorToken::kSecondaryContainer,
    ColorToken::kOnSecondaryContainer,
    ColorToken::kOutlineVariant};

inline constexpr MenuTokens kExpressiveStandardMenuTokens = {
    112,
    280,
    12,
    56,
    20,
    16,
    2,
    12,
    8,
    4,
    16,
    16,
    4,
    3,
    ColorToken::kSurfaceContainer,
    ColorToken::kOnSurface,
    ColorToken::kSecondaryContainer,
    ColorToken::kOnSecondaryContainer,
    ColorToken::kOutlineVariant};

inline constexpr MenuTokens kExpressiveVibrantMenuTokens = {
    112,
    280,
    12,
    56,
    20,
    16,
    2,
    12,
    8,
    4,
    16,
    16,
    4,
    3,
    ColorToken::kTertiaryContainer,
    ColorToken::kOnTertiaryContainer,
    ColorToken::kPrimaryContainer,
    ColorToken::kOnPrimaryContainer,
    ColorToken::kOnTertiaryContainer};

}  // namespace roo_windows::material3::internal
