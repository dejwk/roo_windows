#include "roo_windows/decoration/shadow_lookup.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"
#include "roo_windows/decoration/decoration.h"
#include "test/shadow_test_reference.h"

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
#if ROO_WINDOWS_SHADOW_FAST_LUT
// Verifies every 24-bit input and every 32-bit perfect-square transition.
TEST(ShadowLookupTest, IntegerNewtonSquareRootIsExact) {
  uint32_t expected = 0;
  uint32_t next_square = 1;
  for (uint32_t value = 0; value < (1u << 24); ++value) {
    if (value == next_square) {
      ++expected;
      next_square = (expected + 1) * (expected + 1);
    }
    ASSERT_EQ(expected, internal::ShadowDistanceSqrt(value)) << value;
  }
  for (uint32_t root = 4096; root <= 65535; ++root) {
    uint32_t square = root * root;
    ASSERT_EQ(root - 1, internal::ShadowDistanceSqrt(square - 1));
    ASSERT_EQ(root, internal::ShadowDistanceSqrt(square));
    ASSERT_EQ(root, internal::ShadowDistanceSqrt(square + 1));
  }
  EXPECT_EQ(65535, internal::ShadowDistanceSqrt(0xFFFFFFFF));
}

using test::ReferenceLayer;

// Verifies tail pruning and integer geometry preserve every reference alpha,
// including samples beyond the tighter damage bounds and large corner radii.
TEST(ShadowLookupTest, OptimizedSamplesExactlyMatchReference) {
  for (BorderStyle::CornerRadii radii : {BorderStyle::CornerRadii{0, 0, 0, 0},
                                         {4, 4, 4, 4},
                                         {8, 8, 8, 8},
                                         {12, 12, 12, 12},
                                         {16, 16, 16, 16},
                                         {20, 20, 20, 20},
                                         {24, 24, 24, 24},
                                         {28, 28, 28, 28},
                                         {4, 12, 18, 8},
                                         {255, 160, 90, 30}}) {
    roo_display::Box box(-10, 30, radii.max() > 28 ? 789 : 45,
                         radii.max() > 28 ? 509 : 85);
    for (int level = 0; level <= 5; ++level) {
      internal::LookupShadowSpec key =
          internal::MakeLookupShadow(box, radii, level, false);
      internal::LookupShadowSpec ambient =
          internal::MakeLookupShadow(box, radii, level, true);
      roo_display::Box bounds =
          internal::LookupShadowExtents(
              Rect(box.xMin(), box.yMin(), box.xMax(), box.yMax()), level)
              .asBox();
      int stride = radii.max() > 28 ? 3 : 1;
      for (int y = box.yMin() - 64; y <= box.yMax() + 64; y += stride) {
        for (int x = box.xMin() - 64; x <= box.xMax() + 64; x += stride) {
          int k = ReferenceLayer(key, false, x, y);
          int a = ReferenceLayer(ambient, true, x, y);
          int expected = k + a - (k * a + 127) / 255;
          ASSERT_EQ(expected, internal::ReadLookupShadow(key, ambient, x, y))
              << x << ',' << y << " L" << level << " R" << int(radii.max());
          if (x < bounds.xMin() || x > bounds.xMax() || y < bounds.yMin() ||
              y > bounds.yMax()) {
            ASSERT_EQ(0, expected);
          }
        }
      }
    }
  }
}

// Verifies rectangles crossing reused row/column bands match scalar samples
// with fractional outlines, transparent fills, mixed corners and press ink.
TEST(ShadowLookupTest, OptimizedCrossingSpansPreserveComposition) {
  PressOverlay ink(30, 45, 20, roo_display::Color(0x80445566));
  uint32_t state = 0x724981;
  for (int trial = 0; trial < 240; ++trial) {
    state = state * 1664525u + 1013904223u;
    int width = 8 + state % 113;
    state = state * 1664525u + 1013904223u;
    int height = 8 + state % 79;
    int radius = std::min(width, height) / 2;
    BorderStyle style =
        trial % 2 == 0 ? BorderStyle(radius, SmallNumber::Of16ths(trial % 60))
                       : BorderStyle(0, radius, radius / 2, 3,
                                     SmallNumber::Of16ths(trial % 60));
    Decoration decoration(
        roo_display::Box(10, 20, 9 + width, 19 + height), trial % 6,
        OverlaySpec(), trial % 5 == 0 ? &ink : nullptr,
        roo_display::Color(trial % 3 == 0 ? 0x80405060 : 0xFF405060),
        style.corner_radii(), style.outline_width(),
        roo_display::Color(0xCC8899AA), true);
    roo_display::Box bounds = decoration.extents();
    int x0 = bounds.xMin() - trial % 4;
    int x1 = bounds.xMax() + trial % 3;
    int y0 = bounds.yMin() - trial % 3;
    int y1 = bounds.yMax() + trial % 4;
    std::vector<roo_display::Color> pixels((x1 - x0 + 1) * (y1 - y0 + 1));
    bool uniform = decoration.readColorRect(x0, y0, x1, y1, pixels.data());
    int index = 0;
    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) {
        roo_display::Color expected = decoration.readWithContent(
            x, y, roo_display::Color(trial % 3 == 0 ? 0x80405060 : 0xFF405060));
        ASSERT_EQ(expected, pixels[uniform ? 0 : index])
            << "trial " << trial << " at " << x << ',' << y;
        ++index;
      }
    }
  }
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
