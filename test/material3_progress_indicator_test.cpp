#include <cmath>
#include <limits>
#include <type_traits>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_windows/material3/progress_indicator/progress_geometry.h"
#include "roo_windows/material3/progress_indicator/progress_indicator.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material3 {
namespace {
using test_support::RooWindowsRenderTestSized;

void ExpectDimensions(Dimensions expected, Dimensions actual) {
  EXPECT_EQ(expected.width(), actual.width());
  EXPECT_EQ(expected.height(), actual.height());
}

class ProgressTest : public RooWindowsRenderTestSized<260, 120> {};

// Verifies invalid input leaves mode/value untouched and finite input clamps.
TEST_F(ProgressTest, StateAndMeasurement) {
  LinearProgressIndicator linear(context());
  CircularProgressIndicator circular(context());
  EXPECT_EQ(ProgressIndicatorMode::kDeterminate, linear.mode());
  EXPECT_TRUE(linear.motionEnabled());
  EXPECT_TRUE(linear.setProgress(0.5f));
  linear.setMotionEnabled(false);
  linear.setIndeterminate();
  for (float invalid : {std::numeric_limits<float>::infinity(),
                        -std::numeric_limits<float>::infinity(),
                        std::numeric_limits<float>::quiet_NaN()}) {
    EXPECT_FALSE(linear.setProgress(invalid));
    EXPECT_FLOAT_EQ(0.5f, linear.progress());
    EXPECT_EQ(ProgressIndicatorMode::kIndeterminate, linear.mode());
  }
  EXPECT_TRUE(linear.setProgress(2));
  EXPECT_FLOAT_EQ(1, linear.progress());
  EXPECT_TRUE(linear.setProgress(-2));
  EXPECT_FLOAT_EQ(0, linear.progress());
  ExpectDimensions(
      Dimensions(Scaled(240), Scaled(4)),
      linear.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0)));
  ExpectDimensions(Dimensions(7, 2),
                   linear.measure(WidthSpec::AtMost(7), HeightSpec::AtMost(2)));
  ExpectDimensions(
      Dimensions(250, 20),
      linear.measure(WidthSpec::Exactly(250), HeightSpec::Exactly(20)));
  ExpectDimensions(
      Dimensions(Scaled(48), Scaled(48)),
      circular.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0)));
  ExpectDimensions(Dimensions(2, 1), circular.measure(WidthSpec::AtMost(2),
                                                      HeightSpec::AtMost(1)));
  static_assert(!std::is_copy_constructible<LinearProgressIndicator>::value);
  static_assert(!std::is_move_constructible<CircularProgressIndicator>::value);
  EXPECT_LE(sizeof(LinearProgressIndicator) - sizeof(Widget), 12u);
  EXPECT_LE(sizeof(CircularProgressIndicator) - sizeof(Widget), 12u);
}

// Verifies gap subtraction handles overlaps and all endpoint transitions.
TEST(ProgressGeometryTest, ContinuousDomains) {
  auto zero = internal::LinearDeterminate(240, 4, 4, 0);
  EXPECT_EQ(0, zero.active_count);
  EXPECT_EQ(1, zero.track_count);
  EXPECT_FLOAT_EQ(0, zero.track[0].start);
  EXPECT_FLOAT_EQ(236, zero.stop.start);
  auto tiny = internal::LinearDeterminate(240, 4, 4, 0.001f);
  EXPECT_FLOAT_EQ(0.24f, tiny.active[0].end);
  EXPECT_FLOAT_EQ(4.24f, tiny.track[0].start);
  auto full = internal::LinearDeterminate(240, 4, 4, 1);
  EXPECT_EQ(0, full.track_count);
  EXPECT_FLOAT_EQ(full.stop.start, full.stop.end);
  auto merged = internal::LinearSegments(240, 4, {20, 80}, {82, 120});
  EXPECT_EQ(2, merged.active_count);
  EXPECT_EQ(2, merged.track_count);
  EXPECT_FLOAT_EQ(16, merged.track[0].end);
  EXPECT_FLOAT_EQ(124, merged.track[1].start);
  auto overlap = internal::LinearSegments(240, 4, {20, 80}, {40, 120});
  EXPECT_EQ(1, overlap.active_count);
  EXPECT_FLOAT_EQ(120, overlap.active[0].end);
  auto arc = internal::FitProgressArc(0, 0.001f, 18, 4);
  EXPECT_LT(arc.thickness, 0.01f);
  EXPECT_GT(arc.start, 0);
  EXPECT_LT(arc.end, 0.001f);
  EXPECT_FLOAT_EQ(0, internal::CircularTrack(0.99f, 18, 4, 4).thickness);
}

/// Surface with independently colored stripes behind all transparent geometry.
class Pattern : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
  void paint(PaintContext& ctx) const override {
    for (int x = 0; x < width(); x += 5)
      ctx.fillRect(x, 0, std::min<int>(x + 4, width() - 1), height() - 1,
                   x % 10 ? roo_display::Color(0xFFCCDDEE)
                          : roo_display::Color(0xFFEECCAA));
  }
};

// Verifies value changes restore gaps/centers exactly as a fresh full repaint,
// while no-op setters produce no dirty frame or layout request.
TEST_F(ProgressTest, PatternRestorationAndNoOp) {
  LinearProgressIndicator linear(context());
  CircularProgressIndicator circular(context());
  auto owner = std::make_unique<Pattern>(context());
  Pattern& pattern = *owner;
  pattern.add(linear, Rect(10, 10, 249, 29));
  pattern.add(circular, Rect(100, 40, 159, 99));
  app_.add(std::move(owner), roo_display::Box(0, 0, 259, 119));
  ASSERT_TRUE(refresh());
  for (float p : {0.0001f, 0.5f, 0.98f, 1.0f, 0.0f}) {
    linear.setProgress(p);
    circular.setProgress(p);
    ASSERT_TRUE(refresh());
    auto partial = ::roo_windows::test::CaptureRgb(offscreen_.raster(), 0, 0,
                                                   kWidth, kHeight);
    pattern.invalidateInterior();
    ASSERT_TRUE(refresh());
    auto full = ::roo_windows::test::CaptureRgb(offscreen_.raster(), 0, 0,
                                                kWidth, kHeight);
    for (int y = 0; y < kHeight; ++y)
      for (int x = 0; x < kWidth; ++x) {
        int16_t px = x, py = y;
        roo_display::Color a, b;
        partial.readColors(&px, &py, 1, &a);
        full.readColors(&px, &py, 1, &b);
        ASSERT_EQ(a, b) << x << "," << y << " progress=" << p;
      }
    linear.setProgress(p);
    circular.setProgress(p);
    EXPECT_FALSE(linear.isDirty());
    EXPECT_FALSE(circular.isDirty());
    EXPECT_FALSE(linear.isLayoutRequested());
  }
  pattern.removeAll();
}

// Verifies zero/tiny/half/near-one/full and static unknown geometry at each
// supported acceptance zoom with both linear directions over a pattern.
TEST_F(ProgressTest, StaticGoldens) {
  for (bool dark : {false, true}) {
    Material3Theme material = DefaultTheme().material3Theme();
    if (dark) {
      material.color.primary = roo_display::Color(0xFFD0BCFF);
      material.color.secondaryContainer = roo_display::Color(0xFF4A4458);
    }
    Theme theme = DefaultTheme();
    theme.material3_theme = &material;
    Environment environment(scheduler_, theme);
    Application app(&environment, display_);
    Pattern pattern(app.context());
    // Theme selection remains an application concern.
    std::vector<std::unique_ptr<LinearProgressIndicator>> lines;
    std::vector<std::unique_ptr<CircularProgressIndicator>> circles;
    int i = 0;
    for (float value : {0.0f, 0.001f, 0.5f, 0.98f, 1.0f, -1.0f}) {
      auto line = std::make_unique<LinearProgressIndicator>(app.context());
      auto circle = std::make_unique<CircularProgressIndicator>(app.context());
      line->setMotionEnabled(false);
      circle->setMotionEnabled(false);
      if (value < 0) {
        line->setIndeterminate();
        circle->setIndeterminate();
      } else {
        line->setProgress(value);
        circle->setProgress(value);
      }
      if (dark) line->setLayoutDirection(LayoutDirection::kRightToLeft);
      pattern.add(*line, Rect(10, i * 10 + 2, 249, i * 10 + 7));
      pattern.add(*circle, Rect(i * 42, 65, i * 42 + 39, 112));
      lines.push_back(std::move(line));
      circles.push_back(std::move(circle));
      ++i;
    }
    Task& task = app.addTaskFullScreen(pattern);
    EXPECT_TRUE(app.refresh());
    std::string name = std::string(dark ? "rtl" : "ltr") + "_" +
                       std::to_string(ROO_WINDOWS_ZOOM);
    EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
        offscreen_.raster(), "test/goldens/progress/" + name + ".ppm",
        "progress_" + name));
    pattern.removeAll();
    task.navigation().clear();
    app.refresh();
  }
}
}  // namespace
}  // namespace roo_windows::material3
