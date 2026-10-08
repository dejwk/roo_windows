#pragma once

#include "roo_windows/material3/button/button.h"

namespace roo_windows::material3::internal {

// Shared authored geometry; density only changes the height token.
struct ButtonGeometryTokens {
  uint8_t height_dp;
  uint8_t horizontal_padding_dp;
  uint8_t icon_size_dp;
  uint8_t icon_gap_dp;
  uint8_t square_corner_radius_dp;
  uint8_t pressed_corner_radius_dp;
};

struct ButtonContentMetrics {
  int16_t text_width;
  int16_t text_height;
  int16_t icon_slot_width;
  int16_t icon_slot_height;
  int16_t gap;
  int16_t content_width;
  int16_t content_height;
};

/// Returns the shared tokens for @p size.
const ButtonGeometryTokens& ButtonGeometryTokensFor(ButtonSize size);

/// Measures label/icon content without widget padding. Compact levels exclude
/// symmetric transparent icon margins from vertical content while preserving
/// its anchor alignment and horizontal slot. Zero retains the nominal slot.
ButtonContentMetrics ResolveButtonContentMetrics(roo::string_view label,
                                                 const MonoIcon* icon,
                                                 ButtonSize size,
                                                 int8_t level = 0);

/// Resolves symmetric padding for a valid signed density in [-5, 0].
/// Level zero retains legacy integer rounding, including oversized content.
/// Compact levels reserve at least Scaled(4) pixels on each vertical edge.
Padding ResolveButtonPadding(ButtonSize size, SmallButtonPadding small_padding,
                             int content_height, int8_t level);

/// Resolves resting or pressed corners against actual measured dimensions.
/// Empty dimensions resolve to zero; callers querying before layout may supply
/// the nominal token dimensions instead.
uint8_t ResolveButtonCornerRadius(ButtonSize size, ButtonShape shape,
                                  bool pressed, Dimensions measured);

}  // namespace roo_windows::material3::internal
