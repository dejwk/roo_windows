#include "roo_windows/core/paint_effect.h"

#include <algorithm>
#include <vector>

#include "gtest/gtest.h"

namespace roo_windows::internal {
namespace {

using roo_display::Box;
using roo_display::Color;

// Verifies modulation changes straight RGB without filling transparent
// coverage.
TEST(PaintEffectTest, TintPreservesCoverage) {
  const Color tint(128, 220, 40, 100);
  for (int alpha : {1, 37, 128, 254, 255}) {
    const Color result =
        roo_display::ApplyBlending(roo_display::BlendingMode::kSourceAtop,
                                   Color(alpha, 20, 180, 60), tint);
    EXPECT_EQ(result.a(), alpha);
    EXPECT_NEAR(result.r(), 20 + (220 - 20) * 128.0 / 255, 1);
    EXPECT_NEAR(result.g(), 180 + (40 - 180) * 128.0 / 255, 1);
    EXPECT_NEAR(result.b(), 60 + (100 - 60) * 128.0 / 255, 1);
  }
  const Color transparent(0, 20, 180, 60);
  EXPECT_EQ(roo_display::ApplyBlending(roo_display::BlendingMode::kSourceAtop,
                                       transparent, tint),
            transparent);
  const Color source(37, 20, 180, 60);
  EXPECT_EQ(roo_display::ApplyBlending(roo_display::BlendingMode::kSourceAtop,
                                       source, Color(0)),
            source);
  EXPECT_EQ(roo_display::ApplyBlending(roo_display::BlendingMode::kSourceAtop,
                                       source, tint.withA(255)),
            tint.withA(37));
}

// Verifies ancestors apply after descendants and the boundary limit is
// excluded.
TEST(PaintEffectTest, AppliesInnerThenOuterAndHonorsLimit) {
  const Box bounds(0, 0, 9, 9);
  const PaintEffect outer(nullptr, bounds, Color(128, 0, 0, 255), nullptr);
  const PaintEffect inner(&outer, bounds, Color(255, 255, 0, 0), nullptr);
  const Color source(37, 20, 180, 60);
  const Color both = PaintEffectStack(&inner, nullptr).apply(4, 4, source);
  EXPECT_EQ(both.a(), 37);
  EXPECT_NEAR(both.r(), 127, 1);
  EXPECT_EQ(both.g(), 0);
  EXPECT_NEAR(both.b(), 128, 1);
  EXPECT_EQ(PaintEffectStack(&inner, &outer).apply(4, 4, source),
            Color(37, 255, 0, 0));
  EXPECT_EQ(PaintEffectStack(&inner, &inner).apply(4, 4, source), source);
  EXPECT_EQ(PaintEffectStack(nullptr, nullptr).apply(4, 4, source), source);
  EXPECT_EQ(PaintEffectStack(&inner, inner.parent()).apply(4, 4, source),
            Color(37, 255, 0, 0));
}

// Verifies each scope uses its own bounds, including disjoint ancestor bounds.
TEST(PaintEffectTest, RasterReadsRespectScopeBounds) {
  const PaintEffect outer(nullptr, Box(0, 0, 4, 4), Color(0xFFFF0000), nullptr);
  const PaintEffect inner(&outer, Box(8, 0, 12, 4), Color(0xFF0000FF), nullptr);
  EXPECT_EQ(PaintEffectStack(&inner).extents(), Box(0, 0, 12, 4));
  const int16_t x[] = {-1, 0, 4, 5, 8, 12, 13};
  const int16_t y[] = {2, 2, 2, 2, 2, 2, 2};
  const Color expected[] = {Color(0), Color(0xFFFF0000), Color(0xFFFF0000),
                            Color(0), Color(0xFF0000FF), Color(0xFF0000FF),
                            Color(0)};
  Color actual[7];
  PaintEffectStack(&inner).readColors(x, y, 7, actual);
  for (int i = 0; i < 7; ++i) EXPECT_EQ(actual[i], expected[i]);
  const Color source(37, 20, 180, 60);
  EXPECT_EQ(PaintEffectStack(&inner, inner.parent()).apply(2, 2, source),
            source);
  EXPECT_EQ(PaintEffectStack(&inner, nullptr).apply(2, 2, source),
            Color(37, 255, 0, 0));

  Color uniform;
  ASSERT_TRUE(PaintEffectStack(&inner, inner.parent())
                  .readUniformColorRect(1, 1, 3, 3, &uniform));
  EXPECT_EQ(uniform, Color(0));
  ASSERT_TRUE(
      PaintEffectStack(&inner).readUniformColorRect(1, 1, 3, 3, &uniform));
  EXPECT_EQ(uniform, Color(0xFFFF0000));
  ASSERT_TRUE(
      PaintEffectStack(&inner).readUniformColorRect(9, 1, 11, 3, &uniform));
  EXPECT_EQ(uniform, Color(0xFF0000FF));
  ASSERT_TRUE(
      PaintEffectStack(&inner).readUniformColorRect(5, 1, 7, 3, &uniform));
  EXPECT_EQ(uniform, Color(0));
  EXPECT_FALSE(
      PaintEffectStack(&inner).readUniformColorRect(3, 1, 9, 3, &uniform));
}

// Verifies ripple modulation uses the borrowed sample and obeys scope clipping.
TEST(PaintEffectTest, RippleHasSpatialCoverageAndUniformInterior) {
  const Color tint(128, 220, 40, 100);
  const PressOverlay ripple(10, 10, 8, tint);
  const PaintEffect effect(nullptr, Box(4, 4, 16, 16), Color(0), &ripple);
  const Color source(37, 20, 180, 60);
  EXPECT_EQ(PaintEffectStack(&effect).apply(10, 10, source),
            roo_display::ApplyBlending(roo_display::BlendingMode::kSourceAtop,
                                       source, tint));
  EXPECT_EQ(PaintEffectStack(&effect).apply(3, 10, source), source);
  Color actual;
  ASSERT_TRUE(PaintEffectStack(&effect, effect.parent())
                  .readUniformColorRect(9, 9, 11, 11, &actual));
  EXPECT_EQ(actual, tint);
  EXPECT_FALSE(PaintEffectStack(&effect, effect.parent())
                   .readUniformColorRect(4, 4, 16, 16, &actual));
  ASSERT_TRUE(PaintEffectStack(&effect, effect.parent())
                  .readUniformColorRect(0, 0, 2, 2, &actual));
  EXPECT_EQ(actual, Color(0));
}

// Verifies rectangle modulation matches point semantics for clipped scopes,
// nested effects, exclusive limits, ripple edges, and transparent source RGB.
TEST(PaintEffectTest, RectangleMatchesPointApplication) {
  const Box bounds(-7, -3, 28, 19);
  const PressOverlay ripple(8, 9, 12, Color(0x806040C0));
  const PaintEffect outer(nullptr, Box(-4, 0, 27, 18), Color(0x60C04020),
                          nullptr);
  const PaintEffect inner(&outer, Box(-2, -1, 25, 17), Color(0), &ripple);
  const PaintEffect flat(nullptr, bounds, Color(0x8050B020), nullptr);
  const PaintEffect* starts[] = {nullptr, &flat, &outer, &inner};
  const PaintEffect* limits[] = {nullptr, &outer, &inner};
  for (const PaintEffect* first : starts) {
    for (const PaintEffect* limit : limits) {
      for (bool uniform : {false, true}) {
        for (int alpha : {0, 37, 255}) {
          std::vector<Color> source(bounds.area());
          std::vector<Color> expected(bounds.area());
          for (int i = 0; i < bounds.area(); ++i) {
            source[i] = uniform ? Color(alpha, 40, 90, 160)
                                : Color((alpha + i) % 256, i % 256,
                                        (i * 7) % 256, (i * 13) % 256);
            expected[i] =
                PaintEffectStack(first, limit)
                    .apply(bounds.xMin() + i % bounds.width(),
                           bounds.yMin() + i / bounds.width(), source[i]);
          }
          std::vector<Color> actual(bounds.area() + 1, Color(0x12345678));
          if (uniform) {
            actual[0] = source[0];
          } else {
            std::copy(source.begin(), source.end(), actual.begin());
          }
          const bool result_uniform =
              PaintEffectStack(first, limit)
                  .applyRect(bounds, actual.data(), uniform);
          for (int i = 0; i < bounds.area(); ++i) {
            EXPECT_EQ(result_uniform ? actual[0] : actual[i], expected[i]);
          }
          EXPECT_EQ(actual.back(), Color(0x12345678));
        }
      }
    }
  }
}

// Counts region queries independently of the source buffer's uniformity.
class CountingPress : public PressOverlay {
 public:
  using PressOverlay::PressOverlay;

  bool readUniformColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            Color* result) const override {
    ++queries;
    return PressOverlay::readUniformColorRect(x0, y0, x1, y1, result);
  }

  bool readColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                     Color* result) const override {
    ++rectangles;
    return PressOverlay::readColorRect(x0, y0, x1, y1, result);
  }

  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  Color* result) const override {
    ++batches;
    samples += count;
    PressOverlay::readColors(x, y, count, result);
  }

  mutable int rectangles = 0;
  mutable int batches = 0;
  mutable int samples = 0;
  mutable int queries = 0;
};

// Verifies varying source pixels share one tint resolution and a uniform
// result remains compressed. Fully transparent source colors need no sampling.
TEST(PaintEffectTest, ResolvesUniformChainOncePerRectangle) {
  const Box bounds(0, 0, 63, 31);
  const CountingPress press(32, 16, 100, Color(0x806040C0));
  const PaintEffect effect(nullptr, bounds, Color(0), &press);
  std::vector<Color> pixels(bounds.area());
  for (int i = 0; i < bounds.area(); ++i) pixels[i] = Color(0x80000000 + i);
  EXPECT_FALSE(PaintEffectStack(&effect, nullptr)
                   .applyRect(bounds, pixels.data(), false));
  EXPECT_EQ(press.queries, 1);
  std::fill(pixels.begin(), pixels.end(), Color(0x12345678));
  pixels[0] = Color(0x80402060);
  EXPECT_TRUE(PaintEffectStack(&effect, nullptr)
                  .applyRect(bounds, pixels.data(), true));
  EXPECT_EQ(press.queries, 2);
  for (int i = 1; i < bounds.area(); ++i) {
    EXPECT_EQ(pixels[i], Color(0x12345678));
  }
  pixels[0] = Color(0x00402060);
  EXPECT_TRUE(PaintEffectStack(&effect, nullptr)
                  .applyRect(bounds, pixels.data(), true));
  EXPECT_EQ(pixels[0], Color(0x00402060));
  EXPECT_EQ(press.queries, 2);
}

// Verifies direct effect rasterization retains the uniform rectangle promise
// and mixed rectangles match the existing point sampler with bounded scratch
// arrays.
TEST(PaintEffectTest, RasterRectangleKeepsUniformResultsCompressed) {
  const Box bounds(0, 0, 31, 23);
  const CountingPress press(16, 12, 100, Color(0x806040C0));
  const PaintEffect outer(nullptr, bounds, Color(0), &press);
  std::vector<Color> actual(bounds.area(), Color(0x12345678));
  ASSERT_TRUE(
      PaintEffectStack(&outer).readColorRect(0, 0, 31, 23, actual.data()));
  EXPECT_EQ(press.queries, 1);
  EXPECT_EQ(actual[0], Color(0x806040C0));
  for (int i = 1; i < bounds.area(); ++i) {
    EXPECT_EQ(actual[i], Color(0x12345678));
  }
  const PaintEffect inner(&outer, Box(4, 4, 27, 19), Color(0xC010FF80),
                          nullptr);
  ASSERT_FALSE(
      PaintEffectStack(&inner).readColorRect(0, 0, 31, 23, actual.data()));
  for (int i = 0; i < bounds.area(); ++i) {
    const int16_t x = i % bounds.width();
    const int16_t y = i / bounds.width();
    Color expected;
    PaintEffectStack(&inner).readColors(&x, &y, 1, &expected);
    EXPECT_EQ(actual[i], expected);
  }
}

// Verifies a clipped flat layer does not turn later ripple sampling into
// per-pixel calls, and a full opaque ancestor restores a compact result.
TEST(PaintEffectTest, PartialLayersUseOneRippleRectangle) {
  const Box bounds(0, 0, 7, 7);
  const CountingPress ripple(4, 4, 4, Color(0x805020C0));
  const PaintEffect outer(nullptr, bounds, Color(0xFF506080), nullptr);
  const PaintEffect middle(&outer, bounds, Color(0), &ripple);
  const PaintEffect inner(&middle, Box(2, 1, 5, 6), Color(0x6080C020), nullptr);
  Color colors[65];
  colors[64] = Color(0x12345678);
  EXPECT_FALSE(
      PaintEffectStack(&inner, &outer).readColorRect(0, 0, 7, 7, colors));
  EXPECT_EQ(ripple.rectangles, 1);
  EXPECT_EQ(ripple.batches, 0);
  for (int i = 0; i < 64; ++i) {
    Color expected(0);
    if (inner.bounds().contains(i % 8, i / 8)) expected = inner.tint();
    expected = roo_display::AlphaBlend(expected, ripple.get(i % 8, i / 8));
    EXPECT_EQ(colors[i], expected);
  }
  EXPECT_TRUE(PaintEffectStack(&inner).readColorRect(0, 0, 7, 7, colors));
  EXPECT_EQ(colors[0], outer.tint());
  EXPECT_TRUE(
      PaintEffectStack(&inner).readUniformColorRect(0, 0, 7, 7, colors));
  EXPECT_EQ(colors[0], outer.tint());
  EXPECT_EQ(colors[64], Color(0x12345678));
}

// Verifies scattered coordinates are gathered into bounded ripple batches,
// while application preserves transparent RGB and matches scalar composition.
TEST(PaintEffectTest, ScatteredPointsBatchRippleReads) {
  const CountingPress ripple(16, 16, 16, Color(0x806020C0));
  const PaintEffect outer(nullptr, Box(0, 0, 31, 31), Color(0x50208060),
                          nullptr);
  const PaintEffect inner(&outer, Box(0, 0, 31, 31), Color(0), &ripple);
  const PaintEffectStack stack(&inner);
  constexpr int count = 259;
  int16_t x[count];
  int16_t y[count];
  Color colors[count];
  Color expected[count];
  for (int i = 0; i < count; ++i) {
    x[i] = i % 2 == 0 ? i % 32 : -1;
    y[i] = (i / 32) % 32;
    colors[i] = Color(i % 256, 20, 80, 160);
    expected[i] = stack.apply(x[i], y[i], colors[i]);
  }
  stack.applyColors(x, y, count, colors);
  EXPECT_EQ(ripple.samples, 130);
  EXPECT_EQ(ripple.batches, 5);
  for (int i = 0; i < count; ++i) EXPECT_EQ(colors[i], expected[i]);
}

// Verifies a partial tint equal to the accumulated opaque tint needs no
// expansion, even though that scope alone is not uniform across the query.
TEST(PaintEffectTest, PartialIdenticalTintKeepsUniformStorage) {
  const Color tint(0xFF406080);
  const PaintEffect outer(nullptr, Box(2, 2, 5, 5), tint, nullptr);
  const PaintEffect inner(&outer, Box(0, 0, 7, 7), tint, nullptr);
  Color colors[64];
  std::fill_n(colors, 64, Color(0x12345678));
  EXPECT_TRUE(PaintEffectStack(&inner).readColorRect(0, 0, 7, 7, colors));
  EXPECT_EQ(colors[0], tint);
  for (int i = 1; i < 64; ++i) EXPECT_EQ(colors[i], Color(0x12345678));
}

}  // namespace
}  // namespace roo_windows::internal
