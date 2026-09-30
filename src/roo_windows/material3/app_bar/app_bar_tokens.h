#pragma once

#include <stdint.h>

namespace roo_windows::material3::internal {

// Shared geometry vocabulary. Values are device-independent Material 3 dp
// tokens; later implementation phases resolve them with Scaled().
struct AppBarVariantTokens {
  int16_t container_height_dp;
  int16_t subtitle_container_height_dp;
  int16_t title_top_inset_dp;
  int16_t title_bottom_inset_dp;
  uint8_t title_max_lines;
  bool supports_subtitle;
};

inline constexpr AppBarVariantTokens kSmallAppBarTokens = {64, 64, 0,
                                                           0,  1,  false};
inline constexpr AppBarVariantTokens kMediumFlexibleAppBarTokens = {
    112, 136, 0, 24, 2, true};
inline constexpr AppBarVariantTokens kLargeFlexibleAppBarTokens = {
    152, 176, 0, 28, 2, true};

struct SearchEntryTokens {
  int16_t container_height_dp;
  int16_t edge_padding_dp;
  int16_t slot_gap_dp;
  int16_t max_width_dp;
};

inline constexpr SearchEntryTokens kStandaloneSearchEntryTokens = {56, 24, 4,
                                                                   0};
inline constexpr SearchEntryTokens kEmbeddedSearchEntryTokens = {56, 12, 4,
                                                                 720};

inline constexpr int16_t kActionTapTargetDp = 48;
// The control strip has 4dp horizontal padding; its 48dp action slots sit
// 8dp below the top of a normal 64dp row. Single-row titles have a 4dp
// gap to adjacent action slots. The 16dp title inset applies without a
// navigation control and to the separate title row of flexible bars.
inline constexpr int16_t kAppBarEdgeInsetDp = 4;
inline constexpr int16_t kAppBarTitleInsetDp = 16;
inline constexpr int16_t kAppBarTitleActionGapDp = 4;

}  // namespace roo_windows::material3::internal
