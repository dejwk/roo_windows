#pragma once

#include <stdint.h>

namespace roo_windows::material3::internal {

/// Material 3 navigation-rail destination geometry in Material dp units.
///
/// The destination consumes these values in Phase 2. The rail container will
/// reuse the width and group-layout values when it lands in Phase 3.
struct NavigationRailTokens {
  int16_t collapsed_min_width_dp;
  int16_t expanded_min_width_dp;
  int16_t destination_height_dp;
  int16_t icon_size_dp;
  int16_t icon_label_gap_dp;
  int16_t collapsed_indicator_width_dp;
  int16_t collapsed_indicator_height_dp;
  int16_t expanded_indicator_padding_dp;
  int16_t expanded_indicator_height_dp;
};

inline constexpr NavigationRailTokens kNavigationRailTokens = {
    80, 256, 64, 24, 4, 64, 32, 16, 40};

}  // namespace roo_windows::material3::internal
