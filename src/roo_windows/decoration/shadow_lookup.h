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
  uint8_t spread_16;
  uint8_t level;
  uint16_t index_scale;
  uint16_t reach_16;
};

/// Resolves a half-dp elevation index at the configured zoom. Indices 0..31
/// represent 0..15.5 logical dp; out-of-range values clamp to that range.
/// Uses fitted key/ambient parameters referenced to heights 0/3/5/8/10/14 dp.
/// Zero disables both layers. Layer parameters round to the nearest sixteenth
/// of a pixel.
LookupShadowSpec MakeLookupShadow(roo_display::Box bounds,
                                  BorderStyle::CornerRadii radii, int level,
                                  bool ambient);

/// Returns finite shadow support, also used by damage and clipping.
Rect LookupShadowExtents(const Rect& bounds, int level);

/// Samples two layers of the same rounded surface and half-dp elevation,
/// combining their black alphas with source-over.
uint8_t ReadLookupShadow(const LookupShadowSpec& key,
                         const LookupShadowSpec& ambient, int16_t x, int16_t y);

#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
/// Approximates the Gaussian edge integral without a table or floating point.
/// Input is signed distance / (3 * sigma) in Q14, clamped to [-1, 1].
uint8_t AnalyticShadowEdge(int normalized_distance_16384);
#endif

#if ROO_WINDOWS_SHADOW_FAST_LUT
/// Computes exact floor(sqrt(value)) using a bounded integer Newton iteration.
inline uint16_t ShadowDistanceSqrt(uint32_t value) {
  if (value < 4) return value == 0 ? 0 : 1;
  int bits = 32 - __builtin_clz(value);
  uint32_t root = 3u << ((bits + 1) / 2 - 2);
  root = (root + value / root) / 2;
  root = (root + value / root) / 2;
  root = (root + value / root) / 2;
  // The seed is between 0.75 and 1.5 times the true root. Three steps leave
  // less than one integer of error; a square comparison resolves rounding.
  if (root == 65536 || root * root > value) --root;
  return root;
}
#endif

/// Samples one layer for visual inspection, bypassing combined gradient caches.
/// Zero elevation disables the layer; ambient selects the ambient opacity.
uint8_t ReadLookupShadowLayer(const LookupShadowSpec& spec, bool ambient,
                              int16_t x, int16_t y);

/// Returns flash bytes occupied by optional side/corner gradient data.
uint32_t ShadowGradientBytes();

/// Exposes table addresses for benchmark placement verification; null if off.
const uint8_t* ShadowLookupTable(bool ambient);

}  // namespace roo_windows::internal
