#include "roo_windows/decoration/shadow_lookup.h"

#include <cmath>

#include "gtest/gtest.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {
namespace {
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
// Verifies the integer analytical edge is monotonic and within one 8-bit alpha
// of the Gaussian reference at every representable Q14 input in its support.
TEST(ShadowLookupTest, AnalyticalProfileMatchesGaussianWithoutTables) {
  int previous = 255;
  for (int input = -16384; input <= 16384; ++input) {
    int actual = internal::AnalyticShadowEdge(input);
    int expected =
        std::lround(255 * .5 * std::erfc(3.0 * input / 16384 / std::sqrt(2.0)));
    ASSERT_LE(std::abs(actual - expected), 1) << input;
    ASSERT_LE(actual, previous) << input;
    previous = actual;
  }
  EXPECT_EQ(255, internal::AnalyticShadowEdge(-100000));
  EXPECT_EQ(0, internal::AnalyticShadowEdge(100000));
  EXPECT_EQ(nullptr, internal::ShadowLookupTable(false));
  EXPECT_EQ(nullptr, internal::ShadowLookupTable(true));
}
#else
// Verifies baked opacity tables exactly match scaling the shared 8-bit profile.
TEST(ShadowLookupTest, TablesMatchGaussianAndBakedOpacity) {
  for (int i = 0; i < 256; ++i) {
    int base = std::lround(255 * .5 *
                           std::erfc((-3.0 + 6.0 * i / 255) / std::sqrt(2.0)));
    for (bool ambient : {false, true}) {
      int expected = base;
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 2
      expected = (base * (ambient ? 38 : 77) + 127) / 255;
#endif
      EXPECT_EQ(expected, internal::ShadowLookupTable(ambient)[i]);
    }
  }
}

#endif

// Verifies no residual shadow at zero, and finite support agrees with damage.
TEST(ShadowLookupTest, NoShadowAtZeroAndNoPixelsOutsideBounds) {
  for (int level = 0; level <= 5; ++level) {
    roo_display::Box box(40, 50, 95, 105);
    BorderStyle::CornerRadii radii{16, 16, 16, 16};
    internal::LookupShadowSpec key =
        internal::MakeLookupShadow(box, radii, level, false);
    internal::LookupShadowSpec ambient =
        internal::MakeLookupShadow(box, radii, level, true);
    roo_display::Box bounds =
        CalculateShadowExtents(
            Rect(box.xMin(), box.yMin(), box.xMax(), box.yMax()), level)
            .asBox();
    for (int y = bounds.yMin() - 2; y <= bounds.yMax() + 2; ++y) {
      for (int x = bounds.xMin() - 2; x <= bounds.xMax() + 2; ++x) {
        if (level == 0 || x < bounds.xMin() || x > bounds.xMax() ||
            y < bounds.yMin() || y > bounds.yMax()) {
          EXPECT_EQ(0, internal::ReadLookupShadow(key, ambient, x, y));
        }
      }
    }
  }
}

#endif

#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC ||       \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED
// Verifies bulk fast paths do not change scalar output for edges, corners,
// mixed radii, outlines, circles, and every Material level.
TEST(ShadowLookupTest, BulkAndScalarSamplingAgree) {
  for (BorderStyle style : {BorderStyle(16, 0), BorderStyle(0, 0),
                            BorderStyle(28, SmallNumber::Of16ths(36)),
                            BorderStyle(4, 12, 18, 8, 1)}) {
    for (int level = 0; level <= 5; ++level) {
      roo_display::Box box(40, 50, 95, 105);
      Decoration decoration(box, level, OverlaySpec(), nullptr,
                            roo_display::Color(0xFFF8F8F8),
                            style.corner_radii(), style.outline_width(),
                            roo_display::Color(0xFF223344));
      roo_display::Box bounds = decoration.extents();
      for (int y = bounds.yMin(); y <= bounds.yMax(); y += 7) {
        for (int x = bounds.xMin(); x <= bounds.xMax(); x += 7) {
          int xmax = std::min(x + 6, int(bounds.xMax()));
          int ymax = std::min(y + 6, int(bounds.yMax()));
          roo_display::Color bulk[49];
          bool uniform = decoration.readColorRect(x, y, xmax, ymax, bulk);
          int index = 0;
          for (int yy = y; yy <= ymax; ++yy) {
            for (int xx = x; xx <= xmax; ++xx) {
              int16_t px = xx;
              int16_t py = yy;
              roo_display::Color scalar;
              decoration.readColors(&px, &py, 1, &scalar);
              EXPECT_EQ(scalar, bulk[uniform ? 0 : index])
                  << x << ',' << y << " L" << level;
              ++index;
            }
          }
        }
      }
    }
  }
}
#else
// Verifies experimental assets are absent from the default production build.
TEST(ShadowLookupTest, LegacyDoesNotHaveTables) {
  EXPECT_EQ(nullptr, internal::ShadowLookupTable(false));
  EXPECT_EQ(nullptr, internal::ShadowLookupTable(true));
}
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED
// Verifies fitted level-3 shadow placement and the intentionally additive,
// piecewise-linear profile on the bottom straight edge.
TEST(ShadowLookupTest, TunedLegacyKeepsAdditiveLinearProfile) {
  Decoration decoration(roo_display::Box(40, 50, 95, 105), 3, OverlaySpec(),
                        nullptr, roo_display::Color(0xFFF8F8F8),
                        BorderStyle::CornerRadii{16, 16, 16, 16}, 0,
                        roo_display::Color(0));
  const uint8_t expected[] = {70, 57, 43, 30, 26, 23, 20,
                              17, 13, 10, 7,  4,  1,  0};
  for (int i = 0; i < 14; ++i) {
    int16_t x = 68;
    int16_t y = 106 + i;
    roo_display::Color actual;
    decoration.readColors(&x, &y, 1, &actual);
    EXPECT_EQ(expected[i], actual.a()) << y;
  }
  EXPECT_EQ(nullptr, internal::ShadowLookupTable(false));
  EXPECT_EQ(nullptr, internal::ShadowLookupTable(true));
}

// Verifies damage contains both shifted layers, zero elevation has no exterior
// shadow, and levels above five retain the level-five fitted parameters.
TEST(ShadowLookupTest, TunedLegacyBoundsContainAllShadowPixels) {
  for (int level : {0, 1, 2, 3, 4, 5, 31}) {
    roo_display::Box box(40, 50, 95, 105);
    Decoration decoration(
        box, level, OverlaySpec(), nullptr, roo_display::Color(0xFFF8F8F8),
        BorderStyle::CornerRadii{16, 16, 16, 16}, 0, roo_display::Color(0));
    roo_display::Box bounds = decoration.extents();
    for (int16_t y = 10; y < 150; ++y) {
      for (int16_t x = 0; x < 140; ++x) {
        if (x >= bounds.xMin() && x <= bounds.xMax() && y >= bounds.yMin() &&
            y <= bounds.yMax())
          continue;
        roo_display::Color actual;
        decoration.readColors(&x, &y, 1, &actual);
        EXPECT_EQ(0, actual.a()) << x << ',' << y << " L" << level;
      }
    }
    if (level == 0) {
      EXPECT_EQ(box, bounds);
    }
    if (level == 3) {
      EXPECT_EQ(roo_display::Box(31, 45, 104, 118), bounds);
    }
    if (level >= 5) {
      EXPECT_EQ(roo_display::Box(26, 44, 109, 127), bounds);
    }
  }
}
#endif
}  // namespace
}  // namespace roo_windows
