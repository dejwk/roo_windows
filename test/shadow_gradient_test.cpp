#include <cmath>

#include "gtest/gtest.h"
#include "roo_display/color/color_modes.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/decoration/shadow_lookup.h"
#include "roo_windows/material3/internal/elevation.h"
#include "test/shadow_test_reference.h"

namespace roo_windows {
namespace {
// Verifies the claimed RGB565 cutoff using the actual conversion/blending
// implementation, exhaustively over every destination and both omitted alphas.
TEST(ShadowGradientTest, Rgb565CutoffPreservesExistingPixels) {
  roo_display::Rgb565 mode;
  for (int raw = 0; raw < 65536; ++raw) {
    roo_display::Color bg = mode.toArgbColor(raw);
    for (int alpha = 1; alpha <= 2; ++alpha) {
      EXPECT_EQ(raw, mode.fromArgbColor(roo_display::AlphaBlendOverOpaque(
                         bg, roo_display::Color(uint32_t(alpha) << 24))));
    }
  }
}

#if ROO_WINDOWS_SHADOW_ALPHA_CUTOFF
// Verifies that each generated bound is tight for a straight edge and leaves
// only invisible combined alpha outside, with no per-layer cutoff assumption.
TEST(ShadowGradientTest, CutoffBoundsMatchStraightEdgeSupport) {
  const roo_display::Box box(0, 0, 511, 511);
  for (int level = 0; level < 32; ++level) {
    auto key = internal::MakeLookupShadow(box, {}, level, false);
    auto ambient = internal::MakeLookupShadow(box, {}, level, true);
    int margins[3] = {};
    for (int d = 1; d < 150; ++d) {
      const int xs[] = {-d, 256, 256};
      const int ys[] = {256, -d, 511 + d};
      for (int side = 0; side < 3; ++side) {
        int k = test::ReferenceLayer(key, false, xs[side], ys[side]);
        int a = test::ReferenceLayer(ambient, true, xs[side], ys[side]);
        if (k + a - (k * a + 127) / 255 > 2) margins[side] = d;
      }
    }
    EXPECT_EQ(
        Rect(-margins[0], -margins[1], 511 + margins[0], 511 + margins[2]),
        internal::LookupShadowExtents(Rect(box), level));
  }
}
#endif

// Independent continuous-dp equations for the selected fitted profile.
// These deliberately do not share the renderer's constexpr table builder.
struct ReferenceParameters {
  double offset;
  double blur;
  double spread;
};

ReferenceParameters ParametersAtDp(double h, bool ambient) {
  if (h == 0) return {0, 0, 0};
  if (ambient)
    return {0.29536194914303787 * std::pow(h, 1.2621567045489686),
            1.5842575616751684 * std::pow(h, 0.77850006861395982),
            0.289127889356667 * std::pow(h, 1.1458896433855448)};
  return {3.2183622568085243e-71 *
                  std::pow(h + 158.48931924611142, 31.778960489561193) -
              (-0.092746355666016134),
          0.96763006100631976 * std::pow(h + 0, 0.52088400095262666) -
              (-0.025465001148427646),
          0};
}

// Verifies all half-dp inputs resolve the intended logical curves before zoom
// and rounding, including the 0.5 dp minimum and the 14..15.5 dp extrapolation.
TEST(ShadowGradientTest, EveryHalfDpResolvesBeforePixelRounding) {
  double scale = Scaled(100) / 100.0;
  for (int elevation = 0; elevation <= 31; ++elevation) {
    for (bool ambient : {false, true}) {
      ReferenceParameters expected = ParametersAtDp(elevation / 2.0, ambient);
      internal::LookupShadowSpec actual = internal::MakeLookupShadow(
          roo_display::Box(0, 0, 55, 55), {16, 16, 16, 16}, elevation, ambient);
      int sigma = std::lround(expected.blur * 8 * scale);
      EXPECT_EQ(std::lround(expected.offset * 16 * scale), actual.offset_16);
      EXPECT_EQ(std::lround(expected.spread * 16 * scale), actual.spread_16);
      EXPECT_EQ(sigma == 0 ? 0 : (255 * 4096 + 3 * sigma) / (6 * sigma),
                actual.index_scale);
      EXPECT_EQ(elevation, actual.level);
    }
  }
}

// Verifies range clamping, the exact Material-height mapping and unchanged
// packed layer size. Zoom must never change the elevation index.
TEST(ShadowGradientTest, ClampsIndicesAndPreservesMaterialHeights) {
  const uint8_t expected[] = {0, 6, 10, 16, 20, 28};
  for (int level = 0; level <= 5; ++level) {
    EXPECT_EQ(expected[level], material3::internal::RendererElevation(level));
  }
  EXPECT_EQ(0, material3::internal::RendererElevation(-1));
  EXPECT_EQ(28, material3::internal::RendererElevation(100));
  EXPECT_EQ(20u, sizeof(internal::LookupShadowSpec));
  for (int input : {-100, -1, 0, 31, 32, 255, 1000}) {
    int clamped = std::clamp(input, 0, 31);
    for (bool ambient : {false, true}) {
      roo_display::Box box(0, 0, 55, 55);
      internal::LookupShadowSpec actual =
          internal::MakeLookupShadow(box, {}, input, ambient);
      internal::LookupShadowSpec expected_spec =
          internal::MakeLookupShadow(box, {}, clamped, ambient);
      EXPECT_EQ(clamped, actual.level);
      EXPECT_EQ(expected_spec.offset_16, actual.offset_16);
      EXPECT_EQ(expected_spec.spread_16, actual.spread_16);
      EXPECT_EQ(expected_spec.index_scale, actual.index_scale);
      EXPECT_EQ(expected_spec.reach_16, actual.reach_16);
      EXPECT_EQ(internal::LookupShadowExtents(Rect(box), clamped),
                internal::LookupShadowExtents(Rect(box), input));
    }
  }
}

// Verifies every cached radius, a non-cached radius, asymmetric corners and
// all levels against the original formula, at this target's configured zoom.
TEST(ShadowGradientTest, EveryPresetMatchesReferenceAndDamage) {
  for (int dp : {0, 4, 8, 12, 16, 20, 24, 18, 28, -1}) {
    uint8_t radius = Scaled(std::max(dp, 0));
    BorderStyle::CornerRadii radii{radius, radius, radius, radius};
    if (dp < 0) {
      radii = {uint8_t(Scaled(4)), uint8_t(Scaled(8)), uint8_t(Scaled(18)),
               uint8_t(Scaled(20))};
    }
    for (int level = 0; level <= 31; ++level) {
      roo_display::Box box(-11, 17, Scaled(96) - 12, Scaled(72) + 16);
      internal::LookupShadowSpec key =
          internal::MakeLookupShadow(box, radii, level, false);
      internal::LookupShadowSpec ambient =
          internal::MakeLookupShadow(box, radii, level, true);
      roo_display::Box bounds =
          internal::LookupShadowExtents(Rect(box), level).asBox();
      for (int y = box.yMin() - Scaled(40); y <= box.yMax() + Scaled(40); ++y) {
        for (int x = box.xMin() - Scaled(40); x <= box.xMax() + Scaled(40);
             ++x) {
          int k = test::ReferenceLayer(key, false, x, y);
          int a = test::ReferenceLayer(ambient, true, x, y);
          int expected = k + a - (k * a + 127) / 255;
          ASSERT_EQ(expected, internal::ReadLookupShadow(key, ambient, x, y))
              << "zoom " << ROO_WINDOWS_ZOOM << " radius " << dp << " level "
              << level << " pixel " << x << ',' << y;
          if (x < bounds.xMin() || x > bounds.xMax() || y < bounds.yMin() ||
              y > bounds.yMax()) {
            ASSERT_LE(expected, ROO_WINDOWS_SHADOW_ALPHA_CUTOFF);
          }
        }
      }
    }
  }
}
}  // namespace
}  // namespace roo_windows
