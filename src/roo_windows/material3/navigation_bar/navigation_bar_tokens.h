#pragma once

#include <stdint.h>

namespace roo_windows::material3::internal {

/// Shared flexible-navigation-bar geometry, expressed in Material dp units.
///
/// The destination implementation consumes these values in Phase 2; the bar
/// container reuses the same table when it lands in Phase 3.
struct NavigationBarTokens {
  int16_t vertical_height_dp;
  int16_t horizontal_height_dp;
  int16_t icon_size_dp;
  int16_t icon_label_gap_dp;
  int16_t vertical_min_width_dp;
  int16_t vertical_indicator_width_dp;
  int16_t vertical_indicator_height_dp;
  int16_t horizontal_indicator_padding_dp;
  int16_t horizontal_indicator_height_dp;
};

inline constexpr NavigationBarTokens kNavigationBarTokens = {80, 64, 24, 4, 64,
                                                             64, 32, 12, 32};

}  // namespace roo_windows::material3::internal
