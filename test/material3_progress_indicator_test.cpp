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
  pattern.add(linear, Rect(10, 10, 249, 10 + Scaled(4) - 1));
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
/// Exposes published component phase for deterministic registry integration
/// tests.
template <typename Indicator>
class AnimatedProgress : public Indicator {
 public:
  using Indicator::Indicator;
  uint16_t phase() const { return this->phaseMillis(); }
  bool active() const {
    return this->context().animations().contains(*this, 0);
  }
  AnimationStatus seek(int64_t millis) {
    return this->context().animations().seek(*this, 0,
                                             roo_time::Millis(millis));
  }
};

// Verifies actual registry admission/cancellation, dormant subscription, phase
// wrapping, no-op reconciliation, reduced motion and immediate reattachment.
TEST_F(ProgressTest, AnimationLifecycle) {
  AnimatedProgress<LinearProgressIndicator> linear(context());
  Pattern parent(context());
  parent.add(linear, Rect(0, 0, 239, 3));
  linear.setProgress(0.4f);
  linear.setIndeterminate();
  EXPECT_FALSE(linear.active());
  Task& task = app_.addTaskFullScreen(parent);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(linear.active());
  EXPECT_EQ(0, linear.phase());
  ASSERT_EQ(AnimationStatus::kOk, linear.seek(1800LL * 10000000 + 700));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(700, linear.phase());
  linear.setIndeterminate();
  linear.setMotionEnabled(true);
  linear.layout(linear.parent_bounds());
  EXPECT_EQ(700, linear.phase());
  parent.setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(linear.active());
  parent.setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(linear.active());
  EXPECT_EQ(0, linear.phase());
  linear.layout(Rect(0, 0, -1, -1));
  EXPECT_FALSE(linear.active());
  linear.layout(Rect(0, 0, 239, 3));
  EXPECT_TRUE(linear.active());
  ASSERT_EQ(AnimationStatus::kOk, linear.seek(500));
  refresh();
  parent.removeAll();
  parent.add(linear, Rect(0, 0, 239, 3));
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(linear.active());
  EXPECT_EQ(0, linear.phase());
  linear.setMotionEnabled(false);
  EXPECT_FALSE(linear.active());
  EXPECT_EQ(ProgressIndicatorMode::kIndeterminate, linear.mode());
  EXPECT_FLOAT_EQ(0.4f, linear.progress());
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(linear.isDirty());
  linear.setMotionEnabled(true);
  EXPECT_TRUE(linear.active());
  linear.setProgress(0.7f);
  EXPECT_FALSE(linear.active());
  parent.removeAll();
  task.navigation().clear();
  refresh();
}

/// Independent double-precision Bezier reference with a tighter root solve.
double ReferenceChannel(double t, double delay, double duration, double x1,
                        double x2) {
  t = std::clamp((t - delay) / duration, 0.0, 1.0);
  if (t == 0 || t == 1) return t;
  double low = 0, high = 1;
  for (int i = 0; i < 60; ++i) {
    double u = (low + high) / 2;
    double x =
        3 * (1 - u) * (1 - u) * u * x1 + 3 * (1 - u) * u * u * x2 + u * u * u;
    if (x < t)
      low = u;
    else
      high = u;
  }
  double u = (low + high) / 2;
  return 3 * (1 - u) * u * u + u * u * u;
}

// Verifies every millisecond, including all delay/duration boundaries, against
// an independent reference; error remains below half a nominal pixel.
TEST(ProgressGeometryTest, MaterialWaveforms) {
  constexpr double turn = 6.283185307179586;
  for (unsigned t = 0; t < 5400; ++t) {
    auto circular = internal::CircularIndeterminate(t);
    double start = 1520.0 * t / 5400 - 20;
    double end = 1520.0 * t / 5400;
    for (int i = 0; i < 4; ++i) {
      start += 250 * ReferenceChannel(t, 667 + 1350 * i, 667, 0.4, 0.2);
      end += 250 * ReferenceChannel(t, 1350 * i, 667, 0.4, 0.2);
    }
    EXPECT_LT(std::abs(circular.start - start * turn / 360) * 18, 0.5);
    EXPECT_LT(std::abs(circular.end - end * turn / 360) * 18, 0.5);
    EXPECT_GT(circular.end, circular.start);
    if (t >= 1800) continue;
    auto linear = internal::LinearIndeterminate(240, 4, t);
    internal::ProgressInterval expected[] = {
        {static_cast<float>(240 * ReferenceChannel(t, 1267, 533, 0.2, 0.8)),
         static_cast<float>(240 * ReferenceChannel(t, 1000, 567, 0.4, 1))},
        {static_cast<float>(240 * ReferenceChannel(t, 333, 850, 0, 0.65)),
         static_cast<float>(240 * ReferenceChannel(t, 0, 750, 0.1, 0.45))}};
    // Compare interval union membership at subpixel probe points independently
    // of helper sorting/merging and zero-length channel removal.
    for (float x = 0.25f; x < 240; x += 0.5f) {
      bool wanted = false, actual = false, boundary = false;
      for (auto interval : expected) {
        wanted |= x >= interval.start && x <= interval.end;
        boundary |= std::abs(x - interval.start) < 0.01f ||
                    std::abs(x - interval.end) < 0.01f;
      }
      for (int i = 0; i < linear.active_count; ++i)
        actual |= x >= linear.active[i].start && x <= linear.active[i].end;
      if (!boundary) {
        ASSERT_EQ(wanted, actual) << t << ":" << x;
      }
    }
  }
  EXPECT_FLOAT_EQ(internal::CircularIndeterminate(0).start,
                  internal::CircularIndeterminate(5400).start);
}

// Verifies phase is published by the real registry and painting does not
// advance it; reference images include the two-segment crossover and circular
// extremes.
TEST_F(ProgressTest, MotionGoldens) {
  AnimatedProgress<LinearProgressIndicator> linear(context());
  AnimatedProgress<CircularProgressIndicator> circular(context());
  Pattern pattern(context());
  pattern.add(linear, Rect(10, 5, 249, 24));
  pattern.add(circular, Rect(106, 40, 153, 87));
  linear.setIndeterminate();
  circular.setIndeterminate();
  Task& task = app_.addTaskFullScreen(pattern);
  ASSERT_TRUE(refresh());
  for (int phase : {0, 333, 750, 1100, 1500, 1799, 2700, 5399}) {
    ASSERT_EQ(AnimationStatus::kOk, linear.seek(phase));
    ASSERT_EQ(AnimationStatus::kOk, circular.seek(phase));
    refresh();
    EXPECT_EQ(phase % 1800, linear.phase());
    EXPECT_EQ(phase % 5400, circular.phase());
    std::string name = "phase_" + std::to_string(phase) + "_" +
                       std::to_string(ROO_WINDOWS_ZOOM);
    EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
        offscreen_.raster(), "test/goldens/progress/" + name + ".ppm", name));
  }
  pattern.removeAll();
  task.navigation().clear();
  refresh();
}
/// Counts emitted pixels and optionally makes each output window expensive.
class ProgressDisplay
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  ProgressDisplay(roo::byte* data)
      : OffscreenDevice(260, 120, data, roo_display::Argb4444()) {}
  void write(Color* colors, uint32_t count) override {
    pixels += count;
    OffscreenDevice::write(colors, count);
  }
  void fill(Color color, uint32_t count) override {
    pixels += count;
    OffscreenDevice::fill(color, count);
  }
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override {
    if (slow) delay(1);
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }
  uint32_t pixels = 0;
  bool slow = false;
};

// Verifies output is confined to the thin band/ring envelope and a slow paint
// continuation keeps the published phase even when another seek is pending.
TEST(ProgressAcceptanceTest, BoundedWritesAndCoherentContinuation) {
  roo::byte raster[260 * 120 * 2] = {};
  ProgressDisplay device(raster);
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  AnimatedProgress<LinearProgressIndicator> linear(app.context());
  AnimatedProgress<CircularProgressIndicator> circular(app.context());
  Pattern pattern(app.context());
  pattern.add(linear, Rect(10, 10, 249, 10 + Scaled(4) - 1));
  pattern.add(circular, Rect(106, 40, 153, 87));
  Task& task = app.addTaskFullScreen(pattern);
  linear.setIndeterminate();
  circular.setIndeterminate();
  ASSERT_TRUE(app.refresh());
  device.pixels = 0;
  linear.seek(700);
  ASSERT_TRUE(app.refresh());
  EXPECT_GT(device.pixels, 0u);
  EXPECT_LE(device.pixels, 240u * Scaled(4));
  device.pixels = 0;
  circular.seek(700);
  ASSERT_TRUE(app.refresh());
  EXPECT_GT(device.pixels, 0u);
  EXPECT_LE(device.pixels, 48u * 48);
  device.pixels = 0;
  ASSERT_TRUE(app.refresh());
  EXPECT_EQ(0u, device.pixels);
  linear.seek(1000);
  circular.seek(1000);
  device.slow = true;
  bool complete = app.refresh(roo_time::Uptime::Now() + roo_time::Millis(2));
  EXPECT_FALSE(complete);
  EXPECT_EQ(1000, circular.phase());
  circular.seek(1600);
  unsigned attempts = 0;
  while (!complete && attempts++ < 200) {
    complete = app.refresh(roo_time::Uptime::Now() + roo_time::Millis(2));
    EXPECT_EQ(1000, circular.phase());
  }
  EXPECT_TRUE(complete);
  device.slow = false;
  ASSERT_TRUE(app.refresh());
  EXPECT_EQ(1600, circular.phase());
  pattern.removeAll();
  task.navigation().clear();
  app.refresh();
}
}  // namespace
}  // namespace roo_windows::material3
