#pragma once

#include <cstdint>

#include "roo_windows/config.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/rect.h"

namespace roo_windows::internal {

/// Compact fixed-point state for one experimental Material Web shadow layer.
struct LookupShadowSpec {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
  BorderStyle::CornerRadii radii;
  int16_t offset_16;
  int16_t spread_16;
  uint16_t index_scale;
  uint16_t reach_16;
};

/// Resolves an experimental Material level (clamped to 0..5) at configured
/// zoom.
LookupShadowSpec MakeLookupShadow(roo_display::Box bounds,
                                  BorderStyle::CornerRadii radii, int level,
                                  bool ambient);

/// Returns finite shadow support, also used by damage and clipping.
Rect LookupShadowExtents(const Rect& bounds, int level);

/// Samples the two layers and combines their black alphas with source-over.
uint8_t ReadLookupShadow(const LookupShadowSpec& key,
                         const LookupShadowSpec& ambient, int16_t x, int16_t y);

#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
/// Approximates the Gaussian edge integral without a table or floating point.
/// Input is signed distance / (3 * sigma) in Q14, clamped to [-1, 1].
uint8_t AnalyticShadowEdge(int normalized_distance_16384);
#endif

/// Exposes table addresses for benchmark placement verification; null if off.
const uint8_t* ShadowLookupTable(bool ambient);

}  // namespace roo_windows::internal
