#pragma once

#include <stdint.h>

namespace roo_windows::material3::internal {

/// Material 3 navigation-rail destination geometry in Material dp units.
struct NavigationRailTokens {
  int16_t collapsed_min_width_dp;
  int16_t expanded_min_width_dp;
  int16_t expanded_destination_height_dp;
  int16_t collapsed_destination_height_dp;
  int16_t collapsed_top_padding_dp;
  int16_t collapsed_header_gap_dp;
  int16_t icon_size_dp;
  int16_t icon_label_gap_dp;
  int16_t collapsed_indicator_width_dp;
  int16_t collapsed_indicator_height_dp;
  int16_t expanded_indicator_padding_dp;
  int16_t expanded_indicator_height_dp;
  int16_t outer_horizontal_padding_dp;
  int16_t outer_vertical_padding_dp;
  int16_t header_destination_gap_dp;
  int16_t destination_gap_dp;
};

inline constexpr NavigationRailTokens kNavigationRailTokens = {
    // Standard items retain 64 dp slots in both wide rail presentations.
    // The horizontal indicator is 56 dp tall inside that slot.
    96, 220, 64, 64, 44, 40, 24, 4, 56, 32, 16, 56, 0, 4, 16, 4};

/// Baseline narrow rail, using the compact Material Android sizing.
inline constexpr NavigationRailTokens kBaselineNavigationRailTokens = {
    80, 220, 64, 60, 8, 8, 24, 4, 56, 32, 16, 56, 0, 4, 16, 0};

}  // namespace roo_windows::material3::internal
