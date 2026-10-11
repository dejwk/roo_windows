#include "roo_windows/decoration/shadow_lookup.h"

#include <algorithm>

#include "roo_windows/core/theme.h"
#include "roo_windows/decoration/shadow_parameters.h"
#include "roo_windows/internal/isqrt.h"

#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM && defined(ESP_PLATFORM) && \
    !defined(ROO_TESTING)
#include "esp_attr.h"
#define SHADOW_LUT_STORAGE DRAM_ATTR
#elif ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM
#define SHADOW_LUT_STORAGE
#else
#define SHADOW_LUT_STORAGE const
#endif

namespace roo_windows::internal {
namespace {

#include "roo_windows/decoration/shadow_lookup_tables.h"

#if ROO_WINDOWS_SHADOW_FAST_LUT
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS
struct CornerGradient {
  uint32_t start;
  int8_t x_min;
  int8_t y_min;
  uint8_t width;
  uint8_t height;
};
static_assert(sizeof(CornerGradient) == 8, "Keep corner metadata compact");
#include "roo_windows/decoration/shadow_gradient_tables.h"

// A side profile already includes both layer offsets, opacities and
// source-over.
uint8_t SideGradient(int level, int direction, int distance) {
  if (distance < -kSideLimit) return 104;
  if (distance > kSideLimit) return 0;
  return kSideGradients[((level - 1) * 3 + direction) * kSideStride + distance +
                        kSideLimit];
}

#endif

// Samples a cache miss using the geometry already resolved for dispatch.
// Reusing nearest-edge coordinates avoids paying for classification twice.
template <bool ambient>
uint8_t ReadUncachedGradientLayer(const LookupShadowSpec& spec, int edge_x,
                                  int edge_y, int radius) {
  if (edge_x < -int(spec.reach_16) || edge_y < -int(spec.reach_16)) return 0;
  radius *= 16;
  int dx = radius - edge_x;
  int dy = radius - edge_y;
  int distance;
  if (dx > 0 && dy > 0) {
    uint32_t squared = uint32_t(dx) * dx + uint32_t(dy) * dy;
    // A radial rejection also removes transparent pixels inside the layer
    // bounding box. The +1 preserves floor-sqrt quantization at the boundary.
    uint32_t limit = radius + spec.reach_16 + 1;
    if (squared >= limit * limit) return 0;
    distance = ShadowDistanceSqrt(squared);
  } else {
    distance = std::max(dx, dy);
  }
  int index = std::max(
      0, std::min(255, (128 * 4096 + (distance - radius - spec.spread_16) *
                                         spec.index_scale) >>
                           12));
  return ambient ? kAmbient[index] : kKey[index];
}

// Shares nearest-edge geometry between both layers. Optional caches resolve
// straight sides and supported corners; all other samples use the same exact
// calculation, including builds without gradient caches.
uint8_t ReadCachedShadow(const LookupShadowSpec& key,
                         const LookupShadowSpec& ambient, int x, int y) {
  if (key.level == 0) return 0;
  int local_x = (x - key.x) * 16 + 8;
  int local_y = (y - key.y) * 16 + 8;
  int key_y = local_y - key.offset_16;
  int ambient_y = local_y - ambient.offset_16;
  bool right = local_x * 2 >= key.w * 16;
  bool key_bottom = key_y * 2 >= key.h * 16;
  bool ambient_bottom = ambient_y * 2 >= key.h * 16;
  int edge_x = std::min(local_x, key.w * 16 - local_x);
  int edge_key_y = std::min(key_y, key.h * 16 - key_y);
  int edge_ambient_y = std::min(ambient_y, key.h * 16 - ambient_y);
  int key_radius =
      right ? (key_bottom ? key.radii.bottom_right : key.radii.top_right)
            : (key_bottom ? key.radii.bottom_left : key.radii.top_left);
  int ambient_radius = key_bottom == ambient_bottom
                           ? key_radius
                           : (right ? (ambient_bottom ? key.radii.bottom_right
                                                      : key.radii.top_right)
                                    : (ambient_bottom ? key.radii.bottom_left
                                                      : key.radii.top_left));
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS
  if (edge_key_y >= key_radius * 16 && edge_ambient_y >= ambient_radius * 16 &&
      edge_x <= edge_key_y && edge_x <= edge_ambient_y) {
    // Pixel-center edge coordinates also give the integer profile index.
    // Reusing them keeps original x/y coordinates out of the live register set.
    int distance = (8 - edge_x) >> 4;
    return SideGradient(key.level, 0, distance);
  }
  if (key_bottom == ambient_bottom &&
      edge_x >= std::max(key_radius, ambient_radius) * 16 &&
      edge_key_y <= edge_x && edge_ambient_y <= edge_x) {
    int distance =
        (8 - edge_key_y + (key_bottom ? key.offset_16 : -key.offset_16)) >> 4;
    return SideGradient(key.level, key_bottom ? 2 : 1, distance);
  }
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS == 2
  constexpr int step = Scaled(4);
  if (key_bottom == ambient_bottom && key_radius == ambient_radius &&
      key_radius > 0 && key_radius <= Scaled(24) && key_radius % step == 0) {
    int radius_index = key_radius / step - 1;
    const CornerGradient& gradient =
        kCornerGradients[((key.level - 1) * 6 + radius_index) * 2 + key_bottom];
    int u = right ? key.x + key.w - 1 - x : x - key.x;
    int v = key_bottom ? key.y + key.h - 1 - y : y - key.y;
    u -= gradient.x_min;
    v -= gradient.y_min;
    if (u < 0 || v < 0) return 0;
    if (u < gradient.width && v < gradient.height) {
      return kCornerPixels[gradient.start + v * gradient.width + u];
    }
  }
#endif
#endif
  uint16_t k =
      ReadUncachedGradientLayer<false>(key, edge_x, edge_key_y, key_radius);
  uint16_t a = ReadUncachedGradientLayer<true>(ambient, edge_x, edge_ambient_y,
                                               ambient_radius);
  return k + a - (k * a + 127) / 255;
}
#endif

int ClampLevel(int level) { return std::max(0, std::min(31, level)); }

// Signed distance to a spread-adjusted rounded rectangle, in sixteenths.
// Straight strips avoid square roots. Half-pixel centers avoid scale-dependent
// shifts. Quarter selection supports the same per-corner radii as Decoration.
inline int SignedDistance(const LookupShadowSpec& spec, int local_x,
                          int local_y) {
  bool right = 2 * local_x >= spec.w * 16;
  bool bottom = 2 * local_y >= spec.h * 16;
  int radius =
      16 * (right ? (bottom ? spec.radii.bottom_right : spec.radii.top_right)
                  : (bottom ? spec.radii.bottom_left : spec.radii.top_left));
  int dx = radius - (right ? spec.w * 16 - local_x : local_x);
  int dy = radius - (bottom ? spec.h * 16 - local_y : local_y);
  int distance;
  if (dx <= 0 || dy <= 0) {
    distance = std::max(dx, dy);
  } else {
    uint32_t squared = uint32_t(dx) * dx + uint32_t(dy) * dy;
#if ROO_WINDOWS_SHADOW_FAST_LUT
    distance = ShadowDistanceSqrt(squared);
#else
    distance = squared < (1u << 24) ? isqrt24(squared) : isqrt32(squared);
#endif
  }
  return distance - radius - spec.spread_16;
}

// LUT modes share quantization and produce identical alpha. Analytical mode
// instead evaluates the same Gaussian profile at the continuous distance.
template <bool ambient>
uint8_t ReadLayer(const LookupShadowSpec& spec, int x, int y) {
  if (spec.index_scale == 0) return 0;
  int local_x = (x - spec.x) * 16 + 8;
  int local_y = (y - spec.y) * 16 + 8 - spec.offset_16;
  int reach = spec.reach_16;
  if (local_x < -reach || local_x > spec.w * 16 + reach || local_y < -reach ||
      local_y > spec.h * 16 + reach) {
    return 0;
  }
  int distance = SignedDistance(spec, local_x, local_y);
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
  // index_scale is 255 * 4096 / (6 * sigma); convert distance to Q14 / 3 sigma.
  int normalized = distance * spec.index_scale * 8 / 255;
  constexpr uint16_t opacity = ambient ? 38 : 77;
  return (uint16_t(AnalyticShadowEdge(normalized)) * opacity + 127) / 255;
#else
  // index = round(127.5 + distance * 255 / (6 * sigma)). The Q12 reciprocal
  // is resolved once in the constructor; clamping bounds every table access.
  int index = std::max(
      0, std::min(255, (128 * 4096 + distance * spec.index_scale) >> 12));
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 1
  constexpr uint16_t opacity = ambient ? 38 : 77;
  return (uint16_t(kEdge[index]) * opacity + 127) / 255;
#else
  return ambient ? kAmbient[index] : kKey[index];
#endif
#endif
}
}  // namespace

#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
uint8_t AnalyticShadowEdge(int normalized_distance_16384) {
  if (normalized_distance_16384 <= -16384) return 255;
  if (normalized_distance_16384 >= 16384) return 0;
  // Abramowitz & Stegun 7.1.27: erf(x) ~= 1 - p(x)^-4. Positive
  // coefficients preserve monotonicity under integer rounding. All products
  // fit in 32 bits. One integer divide, no exp/erf/sqrt or floating point.
  uint32_t magnitude = normalized_distance_16384 < 0
                           ? -normalized_distance_16384
                           : normalized_distance_16384;
  uint32_t x = (magnitude * 34756 + 8192) >> 14;  // 3 / sqrt(2), Q14.
  uint32_t polynomial = 1280;                     // 0.078108, Q14.
  polynomial = 16 + ((polynomial * x + 8192) >> 14);
  polynomial = 3775 + ((polynomial * x + 8192) >> 14);
  polynomial = 4561 + ((polynomial * x + 8192) >> 14);
  polynomial = 16384 + ((polynomial * x + 8192) >> 14);
  uint32_t inverse = (1u << 28) / polynomial;
  uint32_t squared = (inverse * inverse + 8192) >> 14;
  uint32_t fourth = (squared * squared + 8192) >> 14;
  uint32_t integral = normalized_distance_16384 < 0 ? 32768 - fourth : fourth;
  return (integral * 255 + 16384) >> 15;
}
#endif

LookupShadowSpec MakeLookupShadow(roo_display::Box bounds,
                                  BorderStyle::CornerRadii radii, int level,
                                  bool ambient) {
  LookupShadowSpec spec{};
  const LayerPreset& preset = ambient ? kAmbientPresets[ClampLevel(level)]
                                      : kKeyPresets[ClampLevel(level)];
  spec.x = bounds.xMin();
  spec.y = bounds.yMin();
  spec.w = bounds.width();
  spec.h = bounds.height();
  spec.radii = radii;
  spec.offset_16 = preset.offset_16;
  spec.spread_16 = preset.spread_16;
  spec.level = ClampLevel(level);
  int sigma_16 = preset.sigma_16;
  spec.reach_16 = spec.spread_16 + 3 * sigma_16;
  spec.index_scale =
      sigma_16 == 0 ? 0 : (255 * 4096 + 3 * sigma_16) / (6 * sigma_16);
#if ROO_WINDOWS_SHADOW_FAST_LUT
  if (spec.index_scale != 0) {
    // Largest integer distance that still selects a nonzero baked alpha.
    // This removes only already-transparent tails, not any visible samples.
    int last = ambient ? kAmbientLastNonzero : kKeyLastNonzero;
    int last_distance = ((last + 1 - 128) * 4096 - 1) / spec.index_scale;
    spec.reach_16 = spec.spread_16 + last_distance;
  }
#endif
  return spec;
}

Rect LookupShadowExtents(const Rect& bounds, int level) {
  int horizontal = 0;
  int top = 0;
  int bottom = 0;
  for (bool ambient : {false, true}) {
    LookupShadowSpec spec =
        MakeLookupShadow(bounds.asBox(), {}, level, ambient);
#if ROO_WINDOWS_SHADOW_FAST_LUT
    // Samples are at half-pixel centers, so the last live exterior pixel is
    // floor((reach + 8) / 16), including the signed vertical placement.
    constexpr int rounding = 8;
#else
    constexpr int rounding = 15;
#endif
    horizontal = std::max(horizontal, (spec.reach_16 + rounding) / 16);
    top = std::max(top, (spec.reach_16 - spec.offset_16 + rounding) / 16);
    bottom = std::max(bottom, (spec.reach_16 + spec.offset_16 + rounding) / 16);
  }
  return Rect(bounds.xMin() - horizontal, bounds.yMin() - top,
              bounds.xMax() + horizontal, bounds.yMax() + bottom);
}

uint8_t ReadLookupShadow(const LookupShadowSpec& key,
                         const LookupShadowSpec& ambient, int16_t x,
                         int16_t y) {
#if ROO_WINDOWS_SHADOW_FAST_LUT
  return ReadCachedShadow(key, ambient, x, y);
#else
  uint16_t k = ReadLayer<false>(key, x, y);
  uint16_t a = ReadLayer<true>(ambient, x, y);
  return k + a - (k * a + 127) / 255;
#endif
}

uint8_t ReadLookupShadowLayer(const LookupShadowSpec& spec, bool ambient,
                              int16_t x, int16_t y) {
  if (spec.level == 0) return 0;
  return ambient ? ReadLayer<true>(spec, x, y) : ReadLayer<false>(spec, x, y);
}

uint32_t ShadowGradientBytes() {
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS == 2
  return sizeof(kSideGradients) + sizeof(kCornerGradients) +
         sizeof(kCornerPixels);
#elif ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS == 1
  return sizeof(kSideGradients);
#else
  return 0;
#endif
}

const uint8_t* ShadowLookupTable(bool ambient) {
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 1
  return kEdge;
#elif ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 2
  return ambient ? kAmbient : kKey;
#else
  return nullptr;
#endif
}
}  // namespace roo_windows::internal
