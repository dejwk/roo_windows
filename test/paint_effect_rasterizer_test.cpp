#include <vector>

#include "gtest/gtest.h"
#include "roo_display/shape/basic.h"
#include "roo_windows/core/paint_effect.h"
#include "roo_windows/core/rounded_clip.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows::internal {
namespace {

using roo_display::Box;
using roo_display::Color;

// Verifies a uniform source expands when device-coordinate effects vary,
// including the effect-only path with no rounded mask.
TEST(PaintEffectRasterizerTest, EffectOnlyOverlayUsesDeviceCoordinates) {
  const Color base(96, 13, 25, 37);
  const Color tint(255, 220, 40, 100);
  const roo_display::FilledRect source(Box(0, 0, 7, 3), base);
  const Box bounds(7, -3, 14, 0);
  const Box tinted(9, -2, 12, -1);
  const PaintEffect effect(nullptr, tinted, tint, nullptr);
  const RoundedOverlay overlay(&source, bounds, 7, -3, nullptr, &effect);
  Color rectangle[32];
  ASSERT_FALSE(overlay.readColorRect(7, -3, 14, 0, rectangle));
  int i = 0;
  for (int16_t y = -3; y <= 0; ++y) {
    for (int16_t x = 7; x <= 14; ++x, ++i) {
      const Color expected = tinted.contains(x, y) ? tint.withA(96) : base;
      EXPECT_EQ(rectangle[i], expected);
      Color point;
      overlay.readColors(&x, &y, 1, &point);
      EXPECT_EQ(point, expected);
    }
  }
  Color uniform;
  ASSERT_TRUE(overlay.readUniformColorRect(9, -2, 12, -1, &uniform));
  EXPECT_EQ(uniform, tint.withA(96));
  ASSERT_TRUE(overlay.readColorRect(9, -2, 12, -1, rectangle));
  EXPECT_EQ(rectangle[0], tint.withA(96));
  ASSERT_TRUE(overlay.readUniformColorRect(7, -3, 8, 0, &uniform));
  EXPECT_EQ(uniform, base);
  EXPECT_FALSE(overlay.readUniformColorRect(7, -3, 14, 0, &uniform));
}

// A nonuniform translucent source exposes translation and coverage mistakes.
class CoordinateSource : public roo_display::Rasterizable {
 public:
  Box extents() const override { return Box(-10, -10, 50, 50); }

  static Color Sample(int16_t x, int16_t y) {
    return Color(64, 30 + 3 * x, 30 + 3 * y, 100);
  }

  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  Color* result) const override {
    for (uint32_t i = 0; i < count; ++i) result[i] = Sample(x[i], y[i]);
  }
};

// Verifies point and row reads modulate only surviving nested-mask samples,
// preserve their alpha, and leave rejected corners transparent.
TEST(PaintEffectRasterizerTest, NestedMasksRetainCoverageWithEffects) {
  const Box bounds(0, 0, 31, 23);
  RoundedClip outer;
  outer.reset(&outer, bounds, BorderStyle(8, 0));
  RoundedClip inner;
  inner.reset(&inner, Box(4, 2, 29, 23), BorderStyle(6, 0));
  inner.parent = &outer;
  const PaintEffect effect(nullptr, bounds, Color(128, 220, 40, 100), nullptr);
  const CoordinateSource source;
  const RoundedOverlay overlay(&source, bounds, 2, 3, &inner, &effect);
  std::vector<Color> rectangle(bounds.area());
  std::vector<Color> points(bounds.area());
  std::vector<int16_t> x(bounds.area());
  std::vector<int16_t> y(bounds.area());
  ASSERT_FALSE(overlay.readColorRect(0, 0, 31, 23, rectangle.data()));
  for (int i = 0; i < bounds.area(); ++i) {
    x[i] = i % bounds.width();
    y[i] = i / bounds.width();
  }
  overlay.readColors(x.data(), y.data(), x.size(), points.data());
  for (int i = 0; i < bounds.area(); ++i) {
    const bool visible =
        outer.coverage(x[i], y[i]) == 255 && inner.coverage(x[i], y[i]) == 255;
    const Color expected =
        visible ? PaintEffectStack(&effect).apply(
                      x[i], y[i], CoordinateSource::Sample(x[i] - 2, y[i] - 3))
                : Color(0);
    EXPECT_EQ(rectangle[i], expected);
    EXPECT_EQ(points[i], expected);
  }
  // Also exercise forwarding of an entirely visible, nonuniform source rect.
  ASSERT_FALSE(overlay.readColorRect(10, 8, 15, 12, rectangle.data()));
  for (int i = 0; i < 30; ++i) {
    const int16_t px = 10 + i % 6;
    const int16_t py = 8 + i / 6;
    EXPECT_EQ(rectangle[i],
              PaintEffectStack(&effect).apply(
                  px, py, CoordinateSource::Sample(px - 2, py - 3)));
  }
  Color uniform;
  ASSERT_TRUE(overlay.readUniformColorRect(0, 0, 2, 2, &uniform));
  EXPECT_EQ(uniform, Color(0));
}

// Verifies supplying an outline matches constructing that outline initially,
// while omitting it retains fill, fractional boundary, and shadow pixels.
TEST(PaintEffectRasterizerTest, ResolvedOutlinePreservesDecorationGeometry) {
  const Box bounds(0, 0, 31, 23);
  const Color background(0xFF42685C);
  const Color content(0xFFCA6728);
  const Color original_outline(0xFFCC0033);
  const Color resolved_outline(0xFF2277CC);
  const BorderStyle border(8, 1.5f);
  const OverlaySpec inert;
  const Decoration original(bounds, 2, inert, nullptr, background,
                            border.corner_radii(), border.outline_width(),
                            original_outline, true);
  const Decoration resolved(bounds, 2, inert, nullptr, background,
                            border.corner_radii(), border.outline_width(),
                            resolved_outline, true);
  const Box extents = original.extents();
  int changed = 0;
  for (int y = extents.yMin(); y <= extents.yMax(); ++y) {
    for (int x = extents.xMin(); x <= extents.xMax(); ++x) {
      const Color legacy = original.readWithContent(x, y, content);
      EXPECT_EQ(legacy, original.readWithContent(x, y, content, nullptr));
      EXPECT_EQ(legacy,
                original.readWithContent(x, y, content, &original_outline));
      const Color actual =
          original.readWithContent(x, y, content, &resolved_outline);
      EXPECT_EQ(actual, resolved.readWithContent(x, y, content));
      if (actual != legacy) ++changed;
    }
  }
  EXPECT_GT(changed, 0);
}

// Records rectangle queries to verify the mask path resolves each surviving
// span in bulk and does not evaluate effects for rejected corners.
class SpanCountingPress : public PressOverlay {
 public:
  using PressOverlay::PressOverlay;

  bool readUniformColorRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                            Color* result) const override {
    ++queries;
    return PressOverlay::readUniformColorRect(x0, y0, x1, y1, result);
  }

  mutable int queries = 0;
};

// Verifies masked uniform spans are tinted before expansion and missing spans
// bypass effects entirely, including when the source itself is nonuniform.
TEST(PaintEffectRasterizerTest, ResolvesEffectsOnlyForSurvivingSpans) {
  const Box bounds(0, 0, 31, 23);
  RoundedClip mask;
  mask.reset(&mask, bounds, BorderStyle(10, 0));
  const SpanCountingPress press(16, 12, 100, Color(0xFF204080));
  const PaintEffect effect(nullptr, bounds, Color(0), &press);
  const roo_display::FilledRect solid(bounds, Color(0x80446688));
  const CoordinateSource varying;
  const roo_display::Rasterizable* sources[] = {&solid, &varying};
  for (const roo_display::Rasterizable* source : sources) {
    press.queries = 0;
    const RoundedOverlay overlay(source, bounds, 0, 0, &mask, &effect);
    std::vector<Color> actual(bounds.area());
    ASSERT_FALSE(overlay.readColorRect(0, 0, 31, 23, actual.data()));
    int rows = 0;
    for (int y = 0; y <= 23; ++y) {
      bool visible = false;
      for (int x = 0; x <= 31; ++x) {
        const bool opaque = mask.coverage(x, y) == 255;
        visible = visible || opaque;
        const uint8_t alpha = source == &solid ? 128 : 64;
        EXPECT_EQ(actual[y * 32 + x],
                  opaque ? Color(alpha, 0x20, 0x40, 0x80) : Color(0));
      }
      if (visible) ++rows;
    }
    EXPECT_EQ(press.queries, rows);
    press.queries = 0;
    ASSERT_TRUE(overlay.readColorRect(0, 0, 1, 1, actual.data()));
    EXPECT_EQ(actual[0], Color(0));
    EXPECT_EQ(press.queries, 0);
  }
}

// Verifies decorated interiors resolve to one color before the generic point
// sampler, while outlines and antialiased corners retain their pixel values.
TEST(PaintEffectRasterizerTest, RoundedDecorationKeepsUniformInterior) {
  const Box bounds(0, 0, 31, 23);
  const BorderStyle border(8, 1.5f);
  RoundedClip mask;
  mask.reset(&mask, bounds, border);
  const SpanCountingPress press(16, 12, 100, Color(0x806040C0));
  const PaintEffect effect(nullptr, bounds, Color(0), &press);
  const Color background(0xFF42685C);
  const Color outline(0xFFCC0033);
  const Decoration decoration(bounds, 0, OverlaySpec(), nullptr, background,
                              border.corner_radii(), border.outline_width(),
                              outline, true);
  const RoundedDecoration rounded(decoration, &mask, background, outline,
                                  &effect);
  std::vector<Color> pixels(bounds.area(), Color(0x12345678));
  ASSERT_TRUE(rounded.readColorRect(12, 8, 19, 15, pixels.data()));
  EXPECT_EQ(press.queries, 1);
  EXPECT_EQ(pixels[0], PaintEffectStack(&effect).apply(12, 8, background));
  for (int i = 1; i < 64; ++i) EXPECT_EQ(pixels[i], Color(0x12345678));
  ASSERT_FALSE(rounded.readColorRect(0, 0, 31, 23, pixels.data()));
  for (int i = 0; i < bounds.area(); ++i) {
    const int16_t x = i % 32;
    const int16_t y = i / 32;
    Color expected;
    rounded.readColors(&x, &y, 1, &expected);
    EXPECT_EQ(pixels[i], expected);
  }
}

}  // namespace
}  // namespace roo_windows::internal
