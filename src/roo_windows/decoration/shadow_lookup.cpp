#include "roo_windows/decoration/shadow_lookup.h"

#include <algorithm>

#include "roo_windows/core/theme.h"
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

#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
#include "roo_windows/decoration/shadow_lookup_tables.h"

struct LayerPreset {
  int8_t offset;
  uint8_t blur;
  uint8_t spread;
};

// Material Web 47adb655, elevation/internal/_elevation.scss. Coordinates in dp;
// key opacity is 77/255, ambient is 38/255. Runtime uses no floating point.
constexpr LayerPreset kKeyPresets[] = {{0, 0, 0}, {1, 2, 0}, {1, 2, 0},
                                       {1, 3, 0}, {2, 3, 0}, {4, 4, 0}};
constexpr LayerPreset kAmbientPresets[] = {{0, 0, 0}, {1, 3, 1},  {2, 6, 2},
                                           {4, 8, 3}, {6, 10, 4}, {8, 12, 6}};

int ClampLevel(int level) { return std::max(0, std::min(5, level)); }

// Signed distance to a spread-adjusted rounded rectangle, in sixteenths.
// Straight strips avoid square roots. Half-pixel centers avoid scale-dependent
// shifts. Quarter selection supports the same per-corner radii as Decoration.
int SignedDistance(const LookupShadowSpec& spec, int x, int y) {
  int local_x = (x - spec.x) * 16 + 8;
  int local_y = (y - spec.y) * 16 + 8 - spec.offset_16;
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
    distance = squared < (1u << 24) ? isqrt24(squared) : isqrt32(squared);
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
  int distance = SignedDistance(spec, x, y);
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
#endif
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
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
  const LayerPreset& preset = ambient ? kAmbientPresets[ClampLevel(level)]
                                      : kKeyPresets[ClampLevel(level)];
  spec.x = bounds.xMin();
  spec.y = bounds.yMin();
  spec.w = bounds.width();
  spec.h = bounds.height();
  spec.radii = radii;
  spec.offset_16 = Scaled(int(preset.offset) * 16);
  spec.spread_16 = Scaled(int(preset.spread) * 16);
  int sigma_16 = Scaled(int(preset.blur) * 8);
  spec.reach_16 = spec.spread_16 + 3 * sigma_16;
  spec.index_scale =
      sigma_16 == 0 ? 0 : (255 * 4096 + 3 * sigma_16) / (6 * sigma_16);
#endif
  return spec;
}

Rect LookupShadowExtents(const Rect& bounds, int level) {
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
  int horizontal = 0;
  int top = 0;
  int bottom = 0;
  for (bool ambient : {false, true}) {
    LookupShadowSpec spec =
        MakeLookupShadow(bounds.asBox(), {}, level, ambient);
    horizontal = std::max(horizontal, (spec.reach_16 + 15) / 16);
    top = std::max(top, (spec.reach_16 - spec.offset_16 + 15) / 16);
    bottom = std::max(bottom, (spec.reach_16 + spec.offset_16 + 15) / 16);
  }
  return Rect(bounds.xMin() - horizontal, bounds.yMin() - top,
              bounds.xMax() + horizontal, bounds.yMax() + bottom);
#else
  return bounds;
#endif
}

uint8_t ReadLookupShadow(const LookupShadowSpec& key,
                         const LookupShadowSpec& ambient, int16_t x,
                         int16_t y) {
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
  uint16_t k = ReadLayer<false>(key, x, y);
  uint16_t a = ReadLayer<true>(ambient, x, y);
  return k + a - (k * a + 127) / 255;
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
