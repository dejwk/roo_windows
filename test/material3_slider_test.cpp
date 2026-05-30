#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/slider/range_slider.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/material3/slider/slider_internal.h"
#include "roo_windows/material3/slider/slider_size_internal.h"
#include "roo_windows/material3/slider/value_indicator.h"
#include "roo_windows/widgets/resources/circle.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;

Color QuantizeToArgb4444(Color color) {
  roo_display::Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

class SolidBackdrop : public BasicSurfaceWidget {
 public:
  SolidBackdrop(const Environment& env, Color color, Dimensions dims)
      : BasicSurfaceWidget(env), color_(color), dims_(dims) {}

  Color background() const override { return color_; }

  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class Material3SliderAppTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 120;
  static constexpr int16_t kHeight = 80;
  static constexpr int16_t kSliderX = 12;
  static constexpr int16_t kSliderY = 18;
  static constexpr int16_t kSliderWidth = Scaled(96);
  static constexpr int16_t kSliderHeight = Scaled(44);

  Material3SliderAppTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_),
        slider_(nullptr) {}

  Slider& addSlider(std::unique_ptr<Slider> slider,
                    roo_display::Box box = roo_display::Box(
                        kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                        kSliderY + kSliderHeight - 1)) {
    slider_ = slider.get();
    app_.add(std::move(slider), box);
    EXPECT_TRUE(app_.refresh());
    return *slider_;
  }

  Slider& addSlider(float value) {
    return addSlider(std::make_unique<Slider>(env_, SliderRange{}, value));
  }

  Slider& addSlider(SliderRange range, float value,
                    SliderVariant variant = SliderVariant::kStandard) {
    return addSlider(std::make_unique<Slider>(env_, range, value, variant));
  }

  Slider& slider() {
    EXPECT_NE(nullptr, slider_);
    return *slider_;
  }

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  void fillScreen(Color color) {
    roo_display::Surface surface(display_.output(), 0, 0, display_.extents(),
                                 /*is_write_once=*/false,
                                 display_.getBackgroundColor(),
                                 roo_display::FillMode::kVisible,
                                 roo_display::BlendingMode::kSourceOver);
    Canvas canvas(&surface);
    canvas.fillRect(display_.extents(), color);
  }

  template <typename Widget>
  void paintWidgetContentsForTest(Widget& widget) {
    roo_display::Surface surface(display_.output(), 0, 0, display_.extents(),
                                 /*is_write_once=*/false,
                                 display_.getBackgroundColor(),
                                 roo_display::FillMode::kVisible,
                                 roo_display::BlendingMode::kSourceOver);
    Canvas canvas(&surface);
    widget.paintWidgetContentsForTest(canvas);
  }

  ::testing::AssertionResult ExpectPaintConfinedTo(
      std::initializer_list<Rect> allowed_regions,
      Color untouched_color) const {
    int changed_pixels = 0;
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        bool allowed = false;
        for (const Rect& region : allowed_regions) {
          if (region.contains(x, y)) {
            allowed = true;
            break;
          }
        }
        Color pixel = pixelAt(x, y);
        if (!allowed && pixel != untouched_color) {
          return ::testing::AssertionFailure()
                 << "unexpected paint at (" << x << ", " << y
                 << ") with color 0x" << std::hex << pixel.asArgb() << std::dec;
        }
        if (allowed && pixel != untouched_color) {
          ++changed_pixels;
        }
      }
    }
    if (changed_pixels == 0) {
      return ::testing::AssertionFailure()
             << "expected at least one painted pixel inside the allowed region";
    }
    return ::testing::AssertionSuccess();
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
  Slider* slider_;
};

class Material3SliderRenderTest : public Material3SliderAppTest {};

class TrackingSlider : public Slider {
 public:
  using Slider::Slider;

  void onValueChange(float value, bool from_user) override {
    events.push_back(from_user ? "value:user:" : "value:program:");
    values.push_back(value);
  }

  void onInteractionStart() override { events.push_back("start"); }

  void onInteractionEnd(float value) override {
    events.push_back("end");
    values.push_back(value);
  }

  std::vector<const char*> events;
  std::vector<float> values;
};

class TrackingRangeSlider : public RangeSlider {
 public:
  using RangeSlider::RangeSlider;

  void onValueChange(float start, float end, int active_thumb,
                     bool from_user) override {
    events.push_back(from_user ? "value:user:" : "value:program:");
    active_thumbs.push_back(active_thumb);
    start_values.push_back(start);
    end_values.push_back(end);
  }

  void onInteractionStart(int active_thumb) override {
    events.push_back("start");
    active_thumbs.push_back(active_thumb);
  }

  void onInteractionEnd(float start, float end) override {
    events.push_back("end");
    end_active_thumbs.push_back(activeThumbIndex());
    start_values.push_back(start);
    end_values.push_back(end);
  }

  std::vector<const char*> events;
  std::vector<int> active_thumbs;
  std::vector<int> end_active_thumbs;
  std::vector<float> start_values;
  std::vector<float> end_values;
};

class ContentPaintSlider : public Slider {
 public:
  using Slider::Slider;

  void paintWidgetContentsForTest(const Canvas& parent_canvas) {
    roo_windows::internal::ClipperState clipper_state;
    Clipper clipper(clipper_state, parent_canvas.out(),
                    roo_time::Uptime::Max());
    PaintContext ctx(prepareCanvas(parent_canvas), clipper);
    clipper.pushOverlaySpec(*this, ctx.canvas());
    paintWidgetContents(ctx);
    clipper.popOverlaySpec();
  }
};

Rect ResolveCurrentIndicatorBoundsForTest(const Slider& slider,
                                          const Environment& env) {
  const SliderStyle& style = slider.style();
  internal::SliderAxisMetrics axis(
      slider.width(), slider.height(),
      style.orientation == SliderOrientation::kVertical,
      IsSliderDirectionInverted(style));
  float center = axis.displayCenterFromValue(slider.range().from,
                                             slider.range().to, slider.value());

  char scratch[64];
  roo::string_view label =
      slider.formatLabel(slider.value(), scratch, sizeof(scratch));
  ValueIndicatorBubble bubble(env.theme(), slider.isEnabled());
  bool clamp =
      style.value_indicator == SliderValueIndicatorBehavior::kWithinBounds;
  bool laid_out = bubble.layout(slider.width(), slider.height(), center,
                                style.orientation, label, clamp);
  EXPECT_TRUE(laid_out);
  return laid_out ? bubble.bounds() : Rect(0, 0, -1, -1);
}

float CenterFromValueForTest(const internal::SliderAxisMetrics& axis,
                             const SliderRange& range, float value) {
  return axis.centerFromValue(range.from, range.to, value);
}

float DisplayCenterFromValueForTest(const internal::SliderAxisMetrics& axis,
                                    const SliderRange& range, float value) {
  return axis.displayCenterFromValue(range.from, range.to, value);
}

Rect InvalidationRectForValueChangeForTest(
    const internal::SliderAxisMetrics& axis, const SliderRange& range,
    float old_value, float new_value) {
  return axis.invalidationRectForCenterChange(
      CenterFromValueForTest(axis, range, old_value),
      CenterFromValueForTest(axis, range, new_value));
}

int16_t ReducedTrackRadiusTowardStartForTest(
    const internal::SliderAxisMetrics& axis, float max_primary,
    int16_t track_radius) {
  int16_t max_pixel = (int16_t)floorf(max_primary);
  if (max_pixel < 0) return 0;
  if (max_pixel >= axis.primarySpan()) max_pixel = axis.primarySpan() - 1;
  int16_t visible_span = max_pixel + 1;
  return visible_span < track_radius ? visible_span : track_radius;
}

int16_t ReducedTrackRadiusTowardEndForTest(
    const internal::SliderAxisMetrics& axis, float min_primary,
    int16_t track_radius) {
  int16_t min_pixel = (int16_t)ceilf(min_primary);
  if (min_pixel < 0) min_pixel = 0;
  if (min_pixel >= axis.primarySpan()) return 0;
  int16_t visible_span = axis.primarySpan() - min_pixel;
  return visible_span < track_radius ? visible_span : track_radius;
}

Rect ExpandedSingleSliderInvalidationRectForTest(
    const internal::SliderAxisMetrics& axis, SliderStyle style,
    const SliderRange& range, float old_value, float new_value, Rect rect) {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style.size);
  internal::SliderVisualMetrics old_layout =
      internal::ResolveSliderVisualMetricsForValue(
          axis, range, old_value, internal::kHandleWidth,
          size_metrics.track_height, internal::kTrackHandleGap,
          size_metrics.handle_height);
  internal::SliderVisualMetrics new_layout =
      internal::ResolveSliderVisualMetricsForValue(
          axis, range, new_value, internal::kHandleWidth,
          size_metrics.track_height, internal::kTrackHandleGap,
          size_metrics.handle_height);
  if (ReducedTrackRadiusTowardStartForTest(
          axis, old_layout.active_track_max_primary,
          size_metrics.track_radius) < size_metrics.track_radius ||
      ReducedTrackRadiusTowardStartForTest(
          axis, new_layout.active_track_max_primary,
          size_metrics.track_radius) < size_metrics.track_radius) {
    rect = Rect::Extent(
        rect, Rect(axis.boxFromPrimaryCross(0, 0, 0, axis.crossSpan() - 1)));
  }
  if (ReducedTrackRadiusTowardEndForTest(
          axis, old_layout.inactive_track_min_primary,
          size_metrics.track_radius) < size_metrics.track_radius ||
      ReducedTrackRadiusTowardEndForTest(
          axis, new_layout.inactive_track_min_primary,
          size_metrics.track_radius) < size_metrics.track_radius) {
    rect =
        Rect::Extent(rect, Rect(axis.boxFromPrimaryCross(
                               axis.primarySpan() - 1, 0,
                               axis.primarySpan() - 1, axis.crossSpan() - 1)));
  }
  return rect;
}

Rect InvalidationRectForRangeValueChangeForTest(
    const internal::SliderAxisMetrics& axis, const SliderRange& range,
    float old_start_value, float old_end_value, float new_start_value,
    float new_end_value) {
  Rect start_rect = old_start_value == new_start_value
                        ? Rect(0, 0, -1, -1)
                        : InvalidationRectForValueChangeForTest(
                              axis, range, old_start_value, new_start_value);
  Rect end_rect = old_end_value == new_end_value
                      ? Rect(0, 0, -1, -1)
                      : InvalidationRectForValueChangeForTest(
                            axis, range, old_end_value, new_end_value);
  if (start_rect.empty()) return end_rect;
  if (end_rect.empty()) return start_rect;
  return Rect::Extent(start_rect, end_rect);
}

Rect ResolveInsetIconRectForTest(
    const SliderStyle& style, const SliderRange& range, float value,
    const roo_display::Pictogram& icon, int16_t width, int16_t height,
    SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kStart) {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style.size);
  internal::SliderAxisMetrics axis(width, height);
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis, CenterFromValueForTest(axis, range, value), internal::kHandleWidth,
      size_metrics.track_height, internal::kTrackHandleGap,
      size_metrics.handle_height);
  int16_t icon_primary_span = icon.anchorExtents().width();
  int16_t icon_cross_span = icon.anchorExtents().height();
  int16_t cross_start = layout.track_cross_start +
                        (size_metrics.track_height - icon_cross_span) / 2;
  int16_t max_left_boundary =
      (int16_t)floorf(layout.thumb_center_primary - (float)Scaled(12));
  int16_t min_right_boundary =
      (int16_t)ceilf(layout.thumb_center_primary + (float)Scaled(12));
  auto satisfies_clearance = [&](int16_t min_primary) {
    int16_t max_primary = min_primary + icon_primary_span - 1;
    return !(max_primary > max_left_boundary &&
             min_primary < min_right_boundary);
  };
  int16_t edge_candidate = anchor == SliderInsetIconAnchor::kStart
                               ? Scaled(4)
                               : width - Scaled(4) - icon_primary_span;
  int16_t min_primary;
  if (anchor == SliderInsetIconAnchor::kStart) {
    min_primary =
        edge_candidate + icon_primary_span - 1 <=
                    (int16_t)floorf(layout.active_track_max_primary) &&
                satisfies_clearance(edge_candidate)
            ? edge_candidate
            : std::max<int16_t>(
                  (int16_t)ceilf(layout.inactive_track_min_primary),
                  min_right_boundary);
  } else {
    min_primary =
        edge_candidate >= (int16_t)ceilf(layout.inactive_track_min_primary) &&
                satisfies_clearance(edge_candidate)
            ? edge_candidate
            : std::min<int16_t>(
                  (int16_t)floorf(layout.active_track_max_primary) -
                      icon_primary_span + 1,
                  max_left_boundary - icon_primary_span + 1);
  }
  return Rect(axis.boxFromPrimaryCross(min_primary, cross_start,
                                       min_primary + icon_primary_span - 1,
                                       cross_start + icon_cross_span - 1));
}

Rect ResolveInsetIconReservedRectForTest(
    const SliderStyle& style, const SliderRange& range, float value,
    const roo_display::Pictogram& icon, int16_t width, int16_t height,
    SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kStart) {
  Rect icon_rect = ResolveInsetIconRectForTest(style, range, value, icon, width,
                                               height, anchor);
  return Rect(std::max<XDim>(0, icon_rect.xMin() - Scaled(4)), icon_rect.yMin(),
              std::min<XDim>(width - 1, icon_rect.xMax() + Scaled(4)),
              icon_rect.yMax());
}

Rect ResolveLegacyOverlayClipForTest(const internal::SliderAxisMetrics& axis,
                                     const SliderRange& range, float old_value,
                                     float new_value, int16_t overlay_radius) {
  float old_center = CenterFromValueForTest(axis, range, old_value);
  float new_center = CenterFromValueForTest(axis, range, new_value);
  int16_t min_primary = (int16_t)ceilf(std::min(old_center, new_center) -
                                       0.5f * (float)internal::kHandleWidth) -
                        overlay_radius;
  int16_t max_primary = (int16_t)floorf(std::max(old_center, new_center) +
                                        0.5f * (float)internal::kHandleWidth) +
                        overlay_radius;
  return Rect(axis.boxFromPrimaryCross(min_primary, 0, max_primary,
                                       axis.crossSpan() - 1));
}

class ContentPaintRangeSlider : public RangeSlider {
 public:
  using RangeSlider::RangeSlider;

  void paintWidgetContentsForTest(const Canvas& parent_canvas) {
    roo_windows::internal::ClipperState clipper_state;
    Clipper clipper(clipper_state, parent_canvas.out(),
                    roo_time::Uptime::Max());
    PaintContext ctx(prepareCanvas(parent_canvas), clipper);
    clipper.pushOverlaySpec(*this, ctx.canvas());
    paintWidgetContents(ctx);
    clipper.popOverlaySpec();
  }
};

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(const Environment& env) : Panel(env) {}

  using Panel::add;

  std::vector<Rect> invalidated_regions;

 protected:
  void childShown(const Widget* child) override { (void)child; }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    (void)child;
    (void)rect;
  }

  void childInvalidatedRegion(const Widget* child, Rect rect) override {
    (void)child;
    invalidated_regions.push_back(rect);
  }
};

// Verifies that the slider contributes no default padding or margins, so its
// visual geometry is controlled entirely by its own layout and paint logic.
TEST(Material3Slider, UsesZeroDefaultInsets) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_EQ(Padding(0), slider.getPadding());
  EXPECT_EQ(Margins(0), slider.getMargins());
}

// Verifies that Material 3 sliders disable the generic overlay path and that
// the overlay focus point follows the thumb center across the full travel.
TEST(Material3Slider, UsesNoOverlayAndHandleCenteredFocus) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{}, 0.0f);
  slider.layout(Rect(0, 0, Scaled(96) - 1, Scaled(44) - 1));

  EXPECT_EQ(Widget::OVERLAY_NONE, slider.getOverlayType());

  roo_display::FpPoint start_focus = slider.getPointOverlayFocus();
  EXPECT_FLOAT_EQ((float)Scaled(6) - 0.5f, start_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(44) - 1), start_focus.y);

  slider.setValue(0.5f);
  roo_display::FpPoint mid_focus = slider.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(96) - 1), mid_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(44) - 1), mid_focus.y);

  slider.setValue(1.0f);
  roo_display::FpPoint end_focus = slider.getPointOverlayFocus();
  EXPECT_FLOAT_EQ((float)(Scaled(96) - Scaled(6)) - 0.5f, end_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(44) - 1), end_focus.y);
}

// Verifies that the default slider advertises the Material 3 minimum touch
// target size rather than a tighter visual-only bound.
TEST(Material3Slider, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);
  Dimensions dims = slider.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(44), dims.width());
  EXPECT_EQ(Scaled(44), dims.height());
}

// Verifies that horizontal sloppy-touch bounds expand mainly along the primary
// axis so drags are easier to keep engaged near the track ends.
TEST(Material3Slider, HorizontalSloppyTouchBoundsExtendPrimaryAxis) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{}, 1.0f);
  slider.layout(Rect(0, 0, Scaled(96) - 1, Scaled(44) - 1));

  Rect touch_bounds = slider.getSloppyTouchBounds();

  EXPECT_EQ(-Scaled(20), touch_bounds.xMin());
  EXPECT_EQ(Scaled(96) - 1 + Scaled(20), touch_bounds.xMax());
  EXPECT_EQ(-3, touch_bounds.yMin());
  EXPECT_EQ(Scaled(44) - 1 + 3, touch_bounds.yMax());
}

// Verifies that the vertical range slider reports the same sloppy-touch region
// in local and parent coordinates, with only the expected translation applied.
TEST(Material3Slider, VerticalRangeSliderSloppyBoundsTrackParentOverride) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;

  RangeSlider slider(env, SliderRange{}, 0.25f, 0.75f, style);
  slider.layout(Rect(7, 9, 7 + Scaled(44) - 1, 9 + Scaled(96) - 1));

  Rect local_touch = slider.getSloppyTouchBounds();
  Rect parent_touch = slider.getSloppyTouchParentBounds();

  EXPECT_EQ(
      Rect(-3, -Scaled(20), Scaled(44) - 1 + 3, Scaled(96) - 1 + Scaled(20)),
      local_touch);
  EXPECT_EQ(local_touch.translate(slider.offsetLeft(), slider.offsetTop()),
            parent_touch);
}

// Verifies that an unconstrained measure resolves to the slider's default
// Material 3 square footprint instead of collapsing to zero.
TEST(Material3Slider, ReportsNaturalMeasureAsFortyFourByFortyFour) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  Dimensions measured =
      slider.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_EQ(Scaled(44), measured.width());
  EXPECT_EQ(Scaled(44), measured.height());
}

// Verifies the preferred-size contract for the default horizontal slider:
// match parent on the main axis and exact sizing on the cross axis.
TEST(Material3Slider, ReportsMatchParentPreferredWidthAndExactHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);
  PreferredSize preferred = slider.getPreferredSize();

  EXPECT_TRUE(preferred.width().isMatchParent());
  EXPECT_FALSE(preferred.width().isExact());
  EXPECT_TRUE(preferred.height().isExact());
  EXPECT_EQ(Scaled(44), preferred.height().value());
}

// Verifies that selecting a larger size preset updates all sizing surfaces,
// including suggested minimums, measured size, and preferred size.
TEST(Material3Slider, SizePresetUpdatesMeasuredAndPreferredDimensions) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  SliderStyle style{};
  style.size = SliderSize::kLarge;
  Slider slider(env, SliderRange{0.0f, 100.0f}, 50.0f, SliderVariant::kStandard,
                style);

  Dimensions suggested = slider.getSuggestedMinimumDimensions();
  Dimensions measured =
      slider.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  PreferredSize preferred = slider.getPreferredSize();

  EXPECT_EQ(Scaled(68), suggested.width());
  EXPECT_EQ(Scaled(68), suggested.height());
  EXPECT_EQ(Scaled(68), measured.width());
  EXPECT_EQ(Scaled(68), measured.height());
  EXPECT_TRUE(preferred.width().isMatchParent());
  EXPECT_TRUE(preferred.height().isExact());
  EXPECT_EQ(Scaled(68), preferred.height().value());
}

// Verifies that a vertical slider swaps the sizing policy so the preset drives
// the exact width while the height stays match-parent.
TEST(Material3Slider, VerticalSizePresetUsesExactWidthFromPreset) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  style.size = SliderSize::kMedium;
  Slider slider(env, SliderRange{0.0f, 100.0f}, 50.0f, SliderVariant::kStandard,
                style);

  PreferredSize preferred = slider.getPreferredSize();

  EXPECT_TRUE(preferred.width().isExact());
  EXPECT_EQ(Scaled(52), preferred.width().value());
  EXPECT_TRUE(preferred.height().isMatchParent());
}

// Verifies that the slider's container visuals resolve against the primary
// color role, which drives its track and thumb color selection.
TEST(Material3Slider, EffectiveContainerRoleIsPrimary) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_EQ(ColorRole::kPrimary, slider.effectiveContainerRole());
}

// Verifies that pressed-state feedback is rendered directly by the slider and
// does not opt into the framework overlay or click-animation paths.
TEST(Material3Slider, PressStateDoesNotUseOverlayOrClickAnimation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_FALSE(slider.useOverlayOnPress());
  EXPECT_FALSE(slider.showClickAnimation());
}

// Verifies that a unit-range slider preserves the supplied semantic value.
TEST(Material3Slider, UnitRangeConstructorPreservesValue) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{}, 32768.0f / 65535.0f);

  EXPECT_FLOAT_EQ(0.0f, slider.range().from);
  EXPECT_FLOAT_EQ(1.0f, slider.range().to);
  EXPECT_FLOAT_EQ(0.0f, slider.range().step);
  EXPECT_FLOAT_EQ(32768.0f / 65535.0f, slider.value());
}

// Verifies that the semantic constructor preserves the requested domain value.
TEST(Material3Slider, SemanticConstructorPreservesValue) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_FLOAT_EQ(10.0f, slider.range().from);
  EXPECT_FLOAT_EQ(40.0f, slider.range().to);
  EXPECT_FLOAT_EQ(25.0f, slider.value());
}

// Verifies that setting a semantic value outside the configured range clamps it
// to the nearest endpoint.
TEST(Material3Slider, SetValueClampsIntoConfiguredDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_TRUE(slider.setValue(100.0f));
  EXPECT_FLOAT_EQ(40.0f, slider.value());
}

// Verifies that discrete sliders snap their initial semantic value to the
// nearest valid step.
TEST(Material3Slider, DiscreteConstructorSnapsInitialValue) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 3.6f);

  EXPECT_FLOAT_EQ(4.0f, slider.value());
}

// Verifies that a programmatic semantic value update on a discrete slider uses
// the same nearest-step snapping logic as construction.
TEST(Material3Slider, SetValueSnapsToNearestDiscreteStep) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);

  EXPECT_TRUE(slider.setValue(3.6f));
  EXPECT_FLOAT_EQ(4.0f, slider.value());
}

// Verifies that replacing the semantic range clamps the current value into the
// new domain instead of leaving an out-of-range selection behind.
TEST(Material3Slider, SetRangeClampsCurrentValueIntoNewDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_TRUE(slider.setRange(SliderRange{30.0f, 50.0f}));
  EXPECT_FLOAT_EQ(30.0f, slider.range().from);
  EXPECT_FLOAT_EQ(50.0f, slider.range().to);
  EXPECT_FLOAT_EQ(30.0f, slider.value());
}

// Verifies that an invalid range update is rejected atomically and leaves the
// slider's previous range and value untouched.
TEST(Material3Slider, InvalidRangeIsRejectedWithoutChangingState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_FALSE(slider.setRange(SliderRange{40.0f, 10.0f}));
  EXPECT_FLOAT_EQ(10.0f, slider.range().from);
  EXPECT_FLOAT_EQ(40.0f, slider.range().to);
  EXPECT_FLOAT_EQ(25.0f, slider.value());
}

// Verifies that constructing a slider with an invalid range fails fast instead
// of creating a partially initialized widget with inconsistent invariants.
TEST(Material3Slider, InvalidConstructorRangeFailsFast) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  EXPECT_DEATH_IF_SUPPORTED(
      {
        Slider slider(env, SliderRange{40.0f, 10.0f}, 25.0f);
        (void)slider;
      },
      "Invalid slider range");
}

// Verifies that changing to a discrete step that does not tile the range is
// rejected, preserving the existing discrete configuration.
TEST(Material3Slider, IncompatibleDiscreteStepIsRejected) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);

  EXPECT_FALSE(slider.setRange(SliderRange{0.0f, 5.0f, 2.0f}));
  EXPECT_FLOAT_EQ(0.0f, slider.range().from);
  EXPECT_FLOAT_EQ(5.0f, slider.range().to);
  EXPECT_FLOAT_EQ(1.0f, slider.range().step);
  EXPECT_FLOAT_EQ(2.0f, slider.value());
}

// Verifies that negative discrete step sizes are rejected outright rather than
// silently normalizing them into some other interpretation.
TEST(Material3Slider, NegativeStepIsRejected) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);

  EXPECT_FALSE(slider.setRange(SliderRange{0.0f, 5.0f, -1.0f}));
  EXPECT_FLOAT_EQ(1.0f, slider.range().step);
}

// Verifies that the centered variant changes only paint semantics, not the
// current semantic value.
TEST(Material3Slider, CenteredVariantPreservesValue) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{-100.0f, 100.0f}, -20.0f,
                SliderVariant::kCentered);

  EXPECT_EQ(SliderVariant::kCentered, slider.variant());
  EXPECT_FLOAT_EQ(-20.0f, slider.value());
}

// Verifies that switching variants updates centered-vs-standard semantics
// without perturbing the current semantic value.
TEST(Material3Slider, SetVariantUpdatesSemanticVariantOnly) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{-100.0f, 100.0f}, -20.0f);

  EXPECT_TRUE(slider.setVariant(SliderVariant::kCentered));
  EXPECT_EQ(SliderVariant::kCentered, slider.variant());
  EXPECT_FLOAT_EQ(-20.0f, slider.value());
}

// Verifies that medium sliders with a configured inset icon actually render
// the icon into the active track with the expected foreground color.
TEST_F(Material3SliderRenderTest, MediumStandardInsetIconPaintsWhenConfigured) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;

  const roo_display::Pictogram* icon = &circle_24();

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f}, 50.0f, SliderVariant::kStandard, style);
  slider->setIcon(icon);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.onPrimary),
            pixelAt(kSliderX + 12, kSliderY + 26));
}

// Verifies that size presets which do not support inset icons ignore the icon
// request and leave the sampled track pixel unchanged.
TEST_F(Material3SliderRenderTest, SmallSliderIgnoresInsetIcon) {
  SliderStyle style{};
  style.size = SliderSize::kSmall;

  const roo_display::Pictogram* icon = &circle_24();

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f}, 50.0f, SliderVariant::kStandard, style);
  slider->setIcon(icon);

  addSlider(std::move(slider));

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.primary),
            pixelAt(kSliderX + 12, kSliderY + 22));
}

// Verifies that an inset icon anchored on the active side jumps past the thumb
// at the minimum value instead of colliding with the handle geometry.
TEST_F(Material3SliderRenderTest, InsetIconJumpsPastHandleAtMinimumValue) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;

  const roo_display::Pictogram* icon = &circle_24();

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f}, 0.0f, SliderVariant::kStandard, style);
  slider->setIcon(icon);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style.size);
  internal::SliderAxisMetrics axis(kSliderWidth, Scaled(52));
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis, CenterFromValueForTest(axis, SliderRange{0.0f, 100.0f}, 0.0f),
      internal::kHandleWidth, size_metrics.track_height,
      internal::kTrackHandleGap, size_metrics.handle_height);
  Rect jumped_icon = ResolveInsetIconRectForTest(
      style, SliderRange{0.0f, 100.0f}, 0.0f, *icon, kSliderWidth, Scaled(52));
  int jumped_icon_x = kSliderX + jumped_icon.xMin() + jumped_icon.width() / 2;

  EXPECT_NE(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            pixelAt(kSliderX + 6, kSliderY + 26));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            pixelAt(jumped_icon_x, kSliderY + 26));
  EXPECT_GE(jumped_icon.xMin(),
            (int16_t)ceilf(layout.thumb_center_primary + (float)Scaled(12)));
}

// Verifies that an inset icon anchored on the inactive side paints into the
// inactive run and uses the inactive track foreground color.
TEST_F(Material3SliderRenderTest, InactiveSideInsetIconPaintsWhenConfigured) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;

  const roo_display::Pictogram* icon = &circle_24();
  SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kEnd;

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f}, 50.0f, SliderVariant::kStandard, style);
  slider->setIcon(icon, anchor);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  Rect icon_rect =
      ResolveInsetIconRectForTest(style, SliderRange{0.0f, 100.0f}, 50.0f,
                                  *icon, kSliderWidth, Scaled(52), anchor);
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            pixelAt(kSliderX + icon_rect.xMin() + icon_rect.width() / 2,
                    kSliderY + icon_rect.yMin() + icon_rect.height() / 2));
}

// Verifies that an inactive-side inset icon also jumps away from the thumb at
// the maximum value, preserving the configured clearance around the handle.
TEST_F(Material3SliderRenderTest,
       InactiveSideInsetIconJumpsPastHandleAtMaximumValue) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;

  const roo_display::Pictogram* icon = &circle_24();
  SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kEnd;

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f}, 100.0f, SliderVariant::kStandard, style);
  slider->setIcon(icon, anchor);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style.size);
  internal::SliderAxisMetrics axis(kSliderWidth, Scaled(52));
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis, CenterFromValueForTest(axis, SliderRange{0.0f, 100.0f}, 100.0f),
      internal::kHandleWidth, size_metrics.track_height,
      internal::kTrackHandleGap, size_metrics.handle_height);
  Rect jumped_icon =
      ResolveInsetIconRectForTest(style, SliderRange{0.0f, 100.0f}, 100.0f,
                                  *icon, kSliderWidth, Scaled(52), anchor);
  int jumped_icon_x = kSliderX + jumped_icon.xMin() + jumped_icon.width() / 2;

  EXPECT_NE(QuantizeToArgb4444(env_.theme().color.onPrimary),
            pixelAt(kSliderX + kSliderWidth - 6, kSliderY + 26));
  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.onPrimary),
            pixelAt(jumped_icon_x, kSliderY + 26));
  EXPECT_LE(jumped_icon.xMax(),
            (int16_t)floorf(layout.thumb_center_primary - (float)Scaled(12)));
}

// Verifies that the reserved padding around an active-side inset icon
// suppresses nearby stop marks so the icon does not visually collide with
// ticks.
TEST_F(Material3SliderRenderTest,
       DiscreteInsetIconPaddingStaysClearOfStopMarks) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;
  style.tick_mode = SliderTickMode::kShowTicks;

  const roo_display::Pictogram* icon = &circle_24();

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 1.0f, 0.25f}, 0.5f, SliderVariant::kStandard,
      style);
  SliderWithInsetIcon* slider_ptr = slider.get();
  slider->setIcon(icon);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  Rect icon_rect = ResolveInsetIconRectForTest(
      style, slider_ptr->range(), slider_ptr->value(), *icon,
      slider_ptr->width(), slider_ptr->height());
  Rect reserved_rect = ResolveInsetIconReservedRectForTest(
      style, slider_ptr->range(), slider_ptr->value(), *icon,
      slider_ptr->width(), slider_ptr->height());
  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t stop_center_x =
      (int16_t)roundf(CenterFromValueForTest(axis, slider_ptr->range(), 0.25f));
  int16_t sample_local_x =
      std::max<int16_t>(icon_rect.xMax() + 1, stop_center_x);

  ASSERT_LE(sample_local_x, reserved_rect.xMax());
  ASSERT_GT(sample_local_x, icon_rect.xMax());

  EXPECT_EQ(
      QuantizeToArgb4444(env_.theme().color.primary),
      pixelAt(kSliderX + sample_local_x, kSliderY + slider_ptr->height() / 2));
}

// Verifies the same stop-suppression behavior for an inactive-side inset icon,
// ensuring reserved padding stays free of icon-colored stop marks.
TEST_F(Material3SliderRenderTest,
       InactiveSideDiscreteInsetIconPaddingStaysClearOfStopMarks) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;
  style.tick_mode = SliderTickMode::kShowTicks;

  const roo_display::Pictogram* icon = &circle_24();
  SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kEnd;

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 1.0f, 0.25f}, 0.5f, SliderVariant::kStandard,
      style);
  SliderWithInsetIcon* slider_ptr = slider.get();
  slider->setIcon(icon, anchor);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  Rect icon_rect = ResolveInsetIconRectForTest(
      style, slider_ptr->range(), slider_ptr->value(), *icon,
      slider_ptr->width(), slider_ptr->height(), anchor);
  Rect reserved_rect = ResolveInsetIconReservedRectForTest(
      style, slider_ptr->range(), slider_ptr->value(), *icon,
      slider_ptr->width(), slider_ptr->height(), anchor);
  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t stop_center_x =
      (int16_t)roundf(CenterFromValueForTest(axis, slider_ptr->range(), 0.75f));
  int16_t sample_local_x =
      std::min<int16_t>(icon_rect.xMin() - 1, stop_center_x);

  ASSERT_GE(sample_local_x, reserved_rect.xMin());
  ASSERT_LT(sample_local_x, icon_rect.xMin());

  Color sample =
      pixelAt(kSliderX + sample_local_x, kSliderY + slider_ptr->height() / 2);
  EXPECT_NE(QuantizeToArgb4444(env_.theme().color.onPrimary), sample);
  EXPECT_NE(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            sample);
}

// Verifies that when an inset icon relocates after a value change, the old
// location is repainted away and the new location is painted in one refresh.
TEST_F(Material3SliderRenderTest,
       InsetIconJumpRepaintsOldAndNewLocationsOnValueChange) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;

  const roo_display::Pictogram* icon = &circle_24();

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f}, 0.0f, SliderVariant::kStandard, style);
  SliderWithInsetIcon* slider_ptr = slider.get();
  slider->setIcon(icon);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  float old_value = 0.0f;
  Rect old_icon =
      ResolveInsetIconRectForTest(style, slider_ptr->range(), old_value, *icon,
                                  slider_ptr->width(), slider_ptr->height());
  int16_t old_local_center_x = old_icon.xMin() + old_icon.width() / 2;
  int16_t old_local_center_y = old_icon.yMin() + old_icon.height() / 2;
  float new_value = old_value + 0.1f;
  Rect new_icon =
      ResolveInsetIconRectForTest(style, slider_ptr->range(), new_value, *icon,
                                  slider_ptr->width(), slider_ptr->height());
  while (new_value < slider_ptr->range().to &&
         new_icon.contains(old_local_center_x, old_local_center_y)) {
    new_value += 0.1f;
    new_icon = ResolveInsetIconRectForTest(
        style, slider_ptr->range(), new_value, *icon, slider_ptr->width(),
        slider_ptr->height());
  }

  ASSERT_LT(new_value, slider_ptr->range().to);
  ASSERT_FALSE(new_icon.contains(old_local_center_x, old_local_center_y));

  int16_t old_center_x = kSliderX + old_local_center_x;
  int16_t old_center_y = kSliderY + old_local_center_y;
  int16_t new_center_x = kSliderX + new_icon.xMin() + new_icon.width() / 2;
  int16_t new_center_y = kSliderY + new_icon.yMin() + new_icon.height() / 2;
  Color expected_new_icon_color =
      new_icon.xMin() == Scaled(4)
          ? QuantizeToArgb4444(env_.theme().color.onPrimary)
          : QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            pixelAt(old_center_x, old_center_y));
  ASSERT_TRUE(slider_ptr->setValue(new_value));
  ASSERT_TRUE(app_.refresh());

  EXPECT_NE(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            pixelAt(old_center_x, old_center_y));
  EXPECT_EQ(expected_new_icon_color, pixelAt(new_center_x, new_center_y));
}

// Verifies that revealing a stop mark previously hidden by inset-icon padding
// repaints the full stop when the icon moves away at the minimum value.
TEST_F(Material3SliderRenderTest,
       InsetIconRevealOfTwentyPercentStopRepaintsFullStopAtMinimumValue) {
  SliderStyle style{};
  style.size = SliderSize::kMedium;

  const roo_display::Pictogram* icon = &circle_24();

  auto slider = std::make_unique<SliderWithInsetIcon>(
      env_, SliderRange{0.0f, 100.0f, 5.0f}, 5.0f, SliderVariant::kStandard,
      style);
  SliderWithInsetIcon* slider_ptr = slider.get();
  slider->setIcon(icon);

  addSlider(std::move(slider),
            roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                             kSliderY + Scaled(52) - 1));

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  auto stop = roo_display::SmoothFilledCircle(
      roo_display::FpPoint{
          CenterFromValueForTest(axis, slider_ptr->range(), 20.0f),
          0.5f * (float)axis.crossSpan() - 0.5f},
      Scaled(2), env_.theme().color.onSecondaryContainer);
  int stop_rightmost_x = kSliderX + stop.extents().xMax();
  int center_y = kSliderY + slider_ptr->height() / 2;

  ASSERT_TRUE(slider_ptr->setValue(0.0f));
  ASSERT_TRUE(app_.refresh());

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.onSecondaryContainer),
            pixelAt(stop_rightmost_x, center_y));
}

// Verifies that centered discrete sliders still apply nearest-step snapping to
// semantic values even though the visual fill is centered around the midpoint.
TEST(Material3Slider, CenteredDiscreteVariantStillSnapsValues) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{-5.0f, 5.0f, 0.5f}, 0.0f,
                SliderVariant::kCentered);

  EXPECT_TRUE(slider.setValue(1.3f));
  EXPECT_FLOAT_EQ(1.5f, slider.value());
}

// Verifies that a programmatic value change triggers only the value-change hook
// and does not emit interaction lifecycle callbacks meant for user gestures.
TEST(Material3Slider, ProgrammaticValueChangeFiresOnlyValueChangeHook) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  TrackingSlider slider(env, SliderRange{0.0f, 10.0f}, 2.0f);

  EXPECT_TRUE(slider.setValue(4.0f));
  ASSERT_EQ(1u, slider.events.size());
  EXPECT_STREQ("value:program:", slider.events[0]);
  ASSERT_EQ(1u, slider.values.size());
  EXPECT_FLOAT_EQ(4.0f, slider.values[0]);
}

// Verifies that the range-slider constructor orders the endpoints and snaps
// both initial values onto the discrete grid before exposing them.
TEST(Material3RangeSlider, ConstructorOrdersAndSnapsInitialValues) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 4.6f, 1.2f);

  EXPECT_FLOAT_EQ(0.0f, slider.range().from);
  EXPECT_FLOAT_EQ(5.0f, slider.range().to);
  EXPECT_FLOAT_EQ(1.0f, slider.startValue());
  EXPECT_FLOAT_EQ(5.0f, slider.endValue());
}

// Verifies that programmatic range-slider updates clamp, snap, and reorder the
// supplied endpoints into a valid in-domain selection.
TEST(Material3RangeSlider, SetValuesClampsSnapsAndOrdersIntoDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{10.0f, 40.0f, 5.0f}, 15.0f, 30.0f);

  EXPECT_TRUE(slider.setValues(42.0f, 11.0f));
  EXPECT_FLOAT_EQ(10.0f, slider.startValue());
  EXPECT_FLOAT_EQ(40.0f, slider.endValue());
}

// Verifies that changing the range-slider domain clamps both thumbs into the
// new range rather than preserving now-invalid endpoint values.
TEST(Material3RangeSlider, SetRangeClampsExistingValuesIntoNewDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{10.0f, 40.0f}, 15.0f, 35.0f);

  EXPECT_TRUE(slider.setRange(SliderRange{20.0f, 30.0f}));
  EXPECT_FLOAT_EQ(20.0f, slider.range().from);
  EXPECT_FLOAT_EQ(30.0f, slider.range().to);
  EXPECT_FLOAT_EQ(20.0f, slider.startValue());
  EXPECT_FLOAT_EQ(30.0f, slider.endValue());
}

// Verifies that an invalid range update on the range slider is rejected
// atomically and leaves the previous state intact.
TEST(Material3RangeSlider, InvalidRangeIsRejectedWithoutChangingState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{10.0f, 40.0f}, 15.0f, 35.0f);

  EXPECT_FALSE(slider.setRange(SliderRange{40.0f, 10.0f}));
  EXPECT_FLOAT_EQ(10.0f, slider.range().from);
  EXPECT_FLOAT_EQ(40.0f, slider.range().to);
  EXPECT_FLOAT_EQ(15.0f, slider.startValue());
  EXPECT_FLOAT_EQ(35.0f, slider.endValue());
}

// Verifies that impossible range-slider discrete domains fail fast at
// construction time instead of leaving broken separation invariants.
TEST(Material3RangeSlider, InvalidConstructorRangeFailsFast) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  EXPECT_DEATH_IF_SUPPORTED(
      {
        RangeSlider slider(env, SliderRange{0.0f, 5.0f, 2.0f}, 1.0f, 4.0f);
        (void)slider;
      },
      "Invalid slider range");
}

// Verifies that programmatic two-thumb updates surface as a single value-change
// callback with no active thumb, not as a user interaction sequence.
TEST(Material3RangeSlider, ProgrammaticSetValuesFiresOnlyValueChangeHook) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  TrackingRangeSlider slider(env, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);

  EXPECT_TRUE(slider.setValues(30.0f, 70.0f));
  ASSERT_EQ(1u, slider.events.size());
  EXPECT_STREQ("value:program:", slider.events[0]);
  ASSERT_EQ(1u, slider.active_thumbs.size());
  EXPECT_EQ(-1, slider.active_thumbs[0]);
  ASSERT_EQ(1u, slider.start_values.size());
  EXPECT_FLOAT_EQ(30.0f, slider.start_values[0]);
  EXPECT_FLOAT_EQ(70.0f, slider.end_values[0]);
}

// Verifies that minimum thumb separation is enforced after discrete snapping,
// potentially pushing one endpoint farther than the raw requested values.
TEST(Material3RangeSlider, MinimumSeparationIsEnforcedInDiscreteMode) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 10.0f, 2.0f}, 2.0f, 8.0f);

  EXPECT_TRUE(slider.setMinSeparation(5.0f));
  EXPECT_FLOAT_EQ(5.0f, slider.minSeparation());
  EXPECT_TRUE(slider.setValues(4.0f, 6.0f));
  EXPECT_FLOAT_EQ(4.0f, slider.startValue());
  EXPECT_FLOAT_EQ(10.0f, slider.endValue());
}

// Verifies that negative minimum-separation values are rejected without
// mutating the existing separation constraint.
TEST(Material3RangeSlider, NegativeMinimumSeparationIsRejected) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 10.0f}, 2.0f, 8.0f);

  EXPECT_FALSE(slider.setMinSeparation(-1.0f));
  EXPECT_FLOAT_EQ(0.0f, slider.minSeparation());
}

// Verifies that the range slider, like the single slider, owns its pressed
// visuals and avoids generic overlay or click-animation behavior.
TEST(Material3RangeSlider, PressStateDoesNotUseOverlayOrClickAnimation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 10.0f}, 2.0f, 8.0f);

  EXPECT_FALSE(slider.useOverlayOnPress());
  EXPECT_FALSE(slider.showClickAnimation());
}

// Verifies that the range slider's extra-large preset feeds through all size
// reporting surfaces, not just the paint metrics.
TEST(Material3RangeSlider, SizePresetUpdatesMeasuredAndPreferredDimensions) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  SliderStyle style{};
  style.size = SliderSize::kExtraLarge;
  RangeSlider slider(env, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f, style);

  Dimensions suggested = slider.getSuggestedMinimumDimensions();
  Dimensions measured =
      slider.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  PreferredSize preferred = slider.getPreferredSize();

  EXPECT_EQ(Scaled(108), suggested.width());
  EXPECT_EQ(Scaled(108), suggested.height());
  EXPECT_EQ(Scaled(108), measured.width());
  EXPECT_EQ(Scaled(108), measured.height());
  EXPECT_TRUE(preferred.width().isMatchParent());
  EXPECT_TRUE(preferred.height().isExact());
  EXPECT_EQ(Scaled(108), preferred.height().value());
}

// Verifies that the range slider's point-overlay focus follows whichever
// thumb becomes active through tapping and subsequent dragging.
TEST_F(Material3SliderAppTest,
       RangeSliderOverlayFocusTracksTappedAndDraggedThumb) {
  auto tracking_slider = std::make_unique<TrackingRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);
  TrackingRangeSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(tracking->width(), tracking->height());

  XDim tap_x = Scaled(84);
  ASSERT_TRUE(tracking->onSingleTapUp(tap_x, tracking->height() / 2));
  roo_display::FpPoint tapped_focus = tracking->getPointOverlayFocus();
  EXPECT_FLOAT_EQ(
      CenterFromValueForTest(axis, tracking->range(), tracking->endValue()),
      tapped_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(tracking->height() - 1), tapped_focus.y);

  XDim start_thumb_center = (XDim)roundf(
      CenterFromValueForTest(axis, tracking->range(), tracking->startValue()));
  tracking->onShowPress(start_thumb_center, tracking->height() / 2);
  ASSERT_TRUE(
      tracking->onScroll(Scaled(42), tracking->height() / 2, Scaled(8), 0));

  roo_display::FpPoint dragged_focus = tracking->getPointOverlayFocus();
  EXPECT_FLOAT_EQ(
      CenterFromValueForTest(axis, tracking->range(), tracking->startValue()),
      dragged_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(tracking->height() - 1), dragged_focus.y);
}

// Verifies that a horizontal drag along the main axis captures and moves the
// single slider toward the tapped side.
TEST_F(Material3SliderAppTest, HorizontalScrollUpdatesPosition) {
  Slider& slider = addSlider(0.0f);

  EXPECT_TRUE(slider.onScroll(Scaled(96) - 1, Scaled(22), Scaled(12), 0));
  EXPECT_GT(slider.value(), 0.9f);
}

// Verifies that a mostly vertical gesture does not get interpreted as a drag
// on a horizontal slider before any dragging interaction has started.
TEST_F(Material3SliderAppTest,
       VerticalDominantScrollDoesNotCaptureWhenNotDragging) {
  Slider& slider = addSlider(12345.0f / 65535.0f);

  EXPECT_FALSE(slider.onScroll(Scaled(48), Scaled(22), 1, 6));
  EXPECT_FLOAT_EQ(12345.0f / 65535.0f, slider.value());
}

// Verifies that vertical orientation swaps the preferred-size contract so the
// main axis becomes match-parent and the cross axis becomes exact.
TEST(Material3SliderOrientation, VerticalPreferredSizeSwapsMajorAxis) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  Slider slider(env, SliderRange{0.0f, 1.0f}, 0.5f, SliderVariant::kStandard,
                style);

  PreferredSize preferred = slider.getPreferredSize();

  EXPECT_TRUE(preferred.width().isExact());
  EXPECT_EQ(Scaled(44), preferred.width().value());
  EXPECT_TRUE(preferred.height().isMatchParent());
}

// Verifies that upward motion on a vertical slider maps to increasing values,
// matching the reversed primary-axis interpretation.
TEST_F(Material3SliderAppTest, VerticalSliderScrollUpUpdatesPosition) {
  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  auto vertical_slider = std::make_unique<Slider>(
      env_, SliderRange{0.0f, 1.0f}, 0.0f, SliderVariant::kStandard, style);
  slider_ = vertical_slider.get();
  app_.add(std::move(vertical_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + Scaled(44) - 1,
                            kSliderY + Scaled(60) - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(slider().onScroll(slider().width() / 2, 2, 0, -Scaled(12)));
  EXPECT_GT(slider().value(), 0.9f);
}

// Verifies that a mostly horizontal gesture does not capture a vertical slider
// when no drag interaction is currently in progress.
TEST_F(Material3SliderAppTest,
       VerticalSliderHorizontalDominantScrollDoesNotCaptureWhenNotDragging) {
  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  auto vertical_slider = std::make_unique<Slider>(
      env_, SliderRange{0.0f, 1.0f}, 0.4f, SliderVariant::kStandard, style);
  slider_ = vertical_slider.get();
  app_.add(std::move(vertical_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + Scaled(44) - 1,
                            kSliderY + Scaled(60) - 1));
  ASSERT_TRUE(app_.refresh());

  float old_value = slider().value();
  EXPECT_FALSE(slider().onScroll(slider().width() / 2, slider().height() / 2,
                                 Scaled(8), 1));
  EXPECT_FLOAT_EQ(old_value, slider().value());
}

// Verifies that tapping near the top of a vertical slider maps to a high value,
// proving the y-axis is inverted relative to screen coordinates.
TEST_F(Material3SliderAppTest, VerticalTapToJumpUsesReversedYMapping) {
  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  auto vertical_slider = std::make_unique<Slider>(
      env_, SliderRange{0.0f, 1.0f}, 0.0f, SliderVariant::kStandard, style);
  slider_ = vertical_slider.get();
  app_.add(std::move(vertical_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + Scaled(44) - 1,
                            kSliderY + Scaled(60) - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(slider().onSingleTapUp(slider().width() / 2, 1));
  EXPECT_GT(slider().value(), 0.95f);
}

// Verifies that vertical orientation can opt out of the default inverted
// mapping when direction is configured explicitly.
TEST_F(Material3SliderAppTest, VerticalNormalDirectionUsesForwardYMapping) {
  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  style.direction = SliderDirection::kNormal;
  auto vertical_slider = std::make_unique<Slider>(
      env_, SliderRange{0.0f, 1.0f}, 1.0f, SliderVariant::kStandard, style);
  slider_ = vertical_slider.get();
  app_.add(std::move(vertical_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + Scaled(44) - 1,
                            kSliderY + Scaled(60) - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(slider().onSingleTapUp(slider().width() / 2, 1));
  EXPECT_LT(slider().value(), 0.05f);
}

// Verifies that horizontal orientation can request an inverted mapping
// independently of the travel axis.
TEST_F(Material3SliderAppTest,
       HorizontalInvertedDirectionUsesReversedXMapping) {
  SliderStyle style{};
  style.direction = SliderDirection::kInverted;
  auto inverted_slider = std::make_unique<Slider>(
      env_, SliderRange{0.0f, 1.0f}, 1.0f, SliderVariant::kStandard, style);
  slider_ = inverted_slider.get();
  app_.add(std::move(inverted_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(
      slider().onSingleTapUp(slider().width() - 2, slider().height() / 2));
  EXPECT_LT(slider().value(), 0.05f);
}

// Verifies that horizontal tap-to-jump lands at the midpoint when tapping the
// visual center.
TEST_F(Material3SliderAppTest, TapToJumpLandsAtMidpointValue) {
  Slider& slider = addSlider(0.0f);

  EXPECT_TRUE(slider.onSingleTapUp(Scaled(48) - 1, slider.height() / 2));
  EXPECT_FLOAT_EQ(0.5f, slider.value());
}

// Verifies that tap-to-jump on a discrete slider resolves to the nearest valid
// step rather than an arbitrary interpolated position.
TEST_F(Material3SliderAppTest, TapToJumpSnapsToNearestDiscreteStep) {
  auto discrete_slider =
      std::make_unique<Slider>(env_, SliderRange{0.0f, 5.0f, 1.0f}, 0.0f);
  slider_ = discrete_slider.get();
  app_.add(std::move(discrete_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(slider().onSingleTapUp(Scaled(70), slider().height() / 2));
  EXPECT_FLOAT_EQ(4.0f, slider().value());
}

// Verifies that the interactive-change callback is reserved for genuine user
// actions and not for programmatic value updates.
TEST_F(Material3SliderAppTest,
       InteractiveChangeFiresOnlyForUserOriginatedValueChanges) {
  Slider& slider = addSlider(0.0f);

  int interactive_change_count = 0;
  slider.setOnInteractiveChange([&]() { ++interactive_change_count; });

  EXPECT_TRUE(slider.setValue(1000.0f));
  EXPECT_EQ(0, interactive_change_count);

  slider.setValue(0.0f);
  EXPECT_EQ(0, interactive_change_count);

  EXPECT_TRUE(slider.onSingleTapUp(Scaled(48) - 1, slider.height() / 2));
  EXPECT_EQ(1, interactive_change_count);
  EXPECT_FLOAT_EQ(0.5f, slider.value());

  EXPECT_TRUE(slider.onSingleTapUp(Scaled(48) - 1, slider.height() / 2));
  EXPECT_EQ(1, interactive_change_count);

  EXPECT_TRUE(
      slider.onScroll(Scaled(96) - 1, slider.height() / 2, Scaled(12), 0));
  EXPECT_EQ(2, interactive_change_count);
  EXPECT_GT(slider.value(), 0.9f);
}

// Verifies the tap lifecycle order for the single slider: interaction start,
// user value change, and interaction end, with the final semantic value
// echoed back on completion.
TEST_F(Material3SliderAppTest,
       TapLifecycleFiresStartValueCompatibilityEndInOrder) {
  auto tracking_slider =
      std::make_unique<TrackingSlider>(env_, SliderRange{}, 0.0f);
  slider_ = tracking_slider.get();
  int interactive_change_count = 0;
  slider_->setOnInteractiveChange([&]() { ++interactive_change_count; });
  TrackingSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(tracking->onSingleTapUp(Scaled(48) - 1, tracking->height() / 2));

  ASSERT_EQ(3u, tracking->events.size());
  EXPECT_STREQ("start", tracking->events[0]);
  EXPECT_STREQ("value:user:", tracking->events[1]);
  EXPECT_STREQ("end", tracking->events[2]);
  EXPECT_EQ(1, interactive_change_count);
  ASSERT_EQ(2u, tracking->values.size());
  EXPECT_FLOAT_EQ(0.5f, tracking->values[0]);
  EXPECT_FLOAT_EQ(0.5f, tracking->values[1]);
}

// Verifies that drag gestures emit only one interaction-start callback and
// still terminate cleanly when canceled rather than released.
TEST_F(Material3SliderAppTest, DragLifecycleFiresSingleStartAndEndsOnCancel) {
  auto tracking_slider =
      std::make_unique<TrackingSlider>(env_, SliderRange{}, 0.0f);
  slider_ = tracking_slider.get();
  TrackingSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  tracking->onShowPress((XDim)tracking->getPointOverlayFocus().x,
                        tracking->height() / 2);
  ASSERT_EQ(1u, tracking->events.size());
  EXPECT_STREQ("start", tracking->events[0]);

  EXPECT_TRUE(tracking->onScroll(Scaled(96) - 1, tracking->height() / 2,
                                 Scaled(12), 0));
  ASSERT_EQ(2u, tracking->events.size());
  EXPECT_STREQ("value:user:", tracking->events[1]);

  tracking->onCancel();
  ASSERT_EQ(3u, tracking->events.size());
  EXPECT_STREQ("end", tracking->events[2]);
  ASSERT_EQ(2u, tracking->values.size());
  EXPECT_GT(tracking->values[0], 0.9f);
  EXPECT_FLOAT_EQ(tracking->values[0], tracking->values[1]);
}

// Verifies that releasing an active drag is consumed by the slider, clears the
// pressed state, and emits a matching interaction-end callback.
TEST_F(Material3SliderAppTest, DragTouchUpIsConsumedAndEndsInteraction) {
  auto tracking_slider =
      std::make_unique<TrackingSlider>(env_, SliderRange{}, 0.0f);
  slider_ = tracking_slider.get();
  TrackingSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  tracking->onShowPress((XDim)tracking->getPointOverlayFocus().x,
                        tracking->height() / 2);
  ASSERT_EQ(1u, tracking->events.size());
  EXPECT_STREQ("start", tracking->events[0]);

  EXPECT_TRUE(tracking->onScroll(Scaled(96) - 1, tracking->height() / 2,
                                 Scaled(12), 0));
  ASSERT_EQ(2u, tracking->events.size());
  EXPECT_STREQ("value:user:", tracking->events[1]);

  EXPECT_TRUE(tracking->onTouchUp(Scaled(96) - 1, tracking->height() / 2));
  ASSERT_EQ(3u, tracking->events.size());
  EXPECT_STREQ("end", tracking->events[2]);
  ASSERT_EQ(2u, tracking->values.size());
  EXPECT_FLOAT_EQ(tracking->values[0], tracking->values[1]);
  EXPECT_FALSE(tracking->isPressed());
}

// Verifies that range-slider drags honor the configured minimum separation,
// keep the active-thumb bookkeeping accurate, and still emit lifecycle hooks.
TEST_F(Material3SliderAppTest,
       RangeSliderDragRespectsMinimumSeparationAndReportsActiveThumb) {
  auto tracking_slider = std::make_unique<TrackingRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);
  TrackingRangeSlider* tracking = tracking_slider.get();
  tracking->setMinSeparation(20.0f);
  int interactive_change_count = 0;
  tracking->setOnInteractiveChange([&]() { ++interactive_change_count; });
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(tracking->width(), tracking->height());
  XDim start_thumb_center = (XDim)roundf(
      CenterFromValueForTest(axis, tracking->range(), tracking->startValue()));
  tracking->onShowPress(start_thumb_center, kSliderHeight / 2);
  ASSERT_FALSE(tracking->events.empty());
  EXPECT_STREQ("start", tracking->events[0]);
  ASSERT_FALSE(tracking->active_thumbs.empty());
  EXPECT_EQ(0, tracking->active_thumbs[0]);
  EXPECT_EQ(0, tracking->activeThumbIndex());

  EXPECT_TRUE(tracking->onScroll(Scaled(90), kSliderHeight / 2, Scaled(20), 0));
  ASSERT_GE(tracking->events.size(), 2u);
  EXPECT_STREQ("value:user:", tracking->events.back());
  ASSERT_GE(tracking->active_thumbs.size(), 2u);
  EXPECT_EQ(0, tracking->active_thumbs.back());
  EXPECT_GE(interactive_change_count, 1);
  EXPECT_FLOAT_EQ(55.0f, tracking->startValue());
  EXPECT_FLOAT_EQ(75.0f, tracking->endValue());

  tracking->onCancel();
  ASSERT_GE(tracking->events.size(), 3u);
  EXPECT_STREQ("end", tracking->events.back());
  EXPECT_EQ(-1, tracking->activeThumbIndex());
  ASSERT_FALSE(tracking->start_values.empty());
  EXPECT_FLOAT_EQ(55.0f, tracking->start_values.back());
  EXPECT_FLOAT_EQ(75.0f, tracking->end_values.back());
}

// Verifies that lifting a drag on the range slider is consumed, ends the
// interaction, and clears the active-thumb state.
TEST_F(Material3SliderAppTest, RangeSliderDragTouchUpIsConsumedAndEnds) {
  auto tracking_slider = std::make_unique<TrackingRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);
  TrackingRangeSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(tracking->width(), tracking->height());
  XDim start_thumb_center = (XDim)roundf(
      CenterFromValueForTest(axis, tracking->range(), tracking->startValue()));
  tracking->onShowPress(start_thumb_center, kSliderHeight / 2);
  ASSERT_FALSE(tracking->events.empty());
  EXPECT_STREQ("start", tracking->events[0]);

  EXPECT_TRUE(tracking->onScroll(Scaled(90), kSliderHeight / 2, Scaled(20), 0));
  ASSERT_GE(tracking->events.size(), 2u);
  EXPECT_STREQ("value:user:", tracking->events.back());

  EXPECT_TRUE(tracking->onTouchUp(Scaled(90), kSliderHeight / 2));
  ASSERT_GE(tracking->events.size(), 3u);
  EXPECT_STREQ("end", tracking->events.back());
  ASSERT_FALSE(tracking->end_active_thumbs.empty());
  EXPECT_EQ(-1, tracking->end_active_thumbs.back());
  EXPECT_EQ(-1, tracking->activeThumbIndex());
  EXPECT_FALSE(tracking->isPressed());
}

// Verifies that when both thumbs overlap, the range slider delays thumb choice
// until drag direction reveals which side the user intends to move.
TEST_F(Material3SliderAppTest, OverlappingRangeThumbsWaitForDirectionalIntent) {
  auto tracking_slider = std::make_unique<TrackingRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 50.0f, 50.0f);
  TrackingRangeSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  tracking->onShowPress(Scaled(48) - 1, kSliderHeight / 2);
  EXPECT_TRUE(tracking->events.empty());
  EXPECT_EQ(-1, tracking->activeThumbIndex());

  EXPECT_TRUE(tracking->onScroll(Scaled(70), kSliderHeight / 2, Scaled(12), 0));
  ASSERT_EQ(2u, tracking->events.size());
  EXPECT_STREQ("start", tracking->events[0]);
  EXPECT_STREQ("value:user:", tracking->events[1]);
  ASSERT_EQ(2u, tracking->active_thumbs.size());
  EXPECT_EQ(1, tracking->active_thumbs[0]);
  EXPECT_EQ(1, tracking->active_thumbs[1]);
  EXPECT_EQ(1, tracking->activeThumbIndex());
  EXPECT_FLOAT_EQ(50.0f, tracking->startValue());
  EXPECT_GT(tracking->endValue(), 70.0f);

  tracking->onCancel();
  ASSERT_EQ(3u, tracking->events.size());
  EXPECT_STREQ("end", tracking->events[2]);
  EXPECT_EQ(-1, tracking->activeThumbIndex());
}

// Verifies the baseline painted geometry of a midpoint slider, including active
// run, inactive run, thumb, and untouched background around the track.
TEST_F(Material3SliderRenderTest,
       PaintsCurrentTrackAndHandleGeometryAtMidpointPosition) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, SliderRange{}, 0.5f);
  slider_ = slider.get();

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive = QuantizeToArgb4444(env_.theme().color.secondaryContainer);
  Color backdrop_color = QuantizeToArgb4444(kBackdropColor);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(backdrop_color, pixelAt(2, 2));
  EXPECT_EQ(background, pixelAt(kSliderX, kSliderY + 14));
  EXPECT_EQ(primary, pixelAt(kSliderX + 20, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(inactive, pixelAt(kSliderX + 70, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 47, kSliderY + 5));
  EXPECT_EQ(background, pixelAt(kSliderX + 20, kSliderY + 5));
}

// Verifies that the centered variant paints its active segment relative to the
// visual midpoint instead of from the track origin.
TEST_F(Material3SliderRenderTest,
       CenteredVariantPaintsActiveTrackFromVisualMidpoint) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, SliderRange{-100.0f, 100.0f},
                                         -40.0f, SliderVariant::kCentered);
  slider_ = slider.get();

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive = QuantizeToArgb4444(env_.theme().color.secondaryContainer);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  internal::SliderAxisMetrics axis(slider_->width(), slider_->height());
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis, CenterFromValueForTest(axis, slider_->range(), slider_->value()),
      Scaled(4), Scaled(16), Scaled(6), Scaled(44));
  float center_anchor_primary = CenterFromValueForTest(
      axis, slider_->range(),
      0.5f * (slider_->range().from + slider_->range().to));

  int left_inactive_x =
      kSliderX + (int)roundf(0.5f * layout.active_track_max_primary);
  int gap_x =
      kSliderX + (int)roundf(0.5f * (layout.thumb_max_primary +
                                     layout.inactive_track_min_primary));
  int active_x =
      kSliderX + (int)roundf(0.5f * (layout.inactive_track_min_primary +
                                     center_anchor_primary));
  int right_inactive_x =
      kSliderX +
      (int)roundf(0.5f * (center_anchor_primary + (float)(kSliderWidth - 1)));

  EXPECT_EQ(inactive, pixelAt(left_inactive_x, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(background, pixelAt(gap_x, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(background, pixelAt(gap_x, kSliderY + 2));
  EXPECT_EQ(background, pixelAt(gap_x, kSliderY + kSliderHeight - 3));
  EXPECT_EQ(primary, pixelAt(active_x, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(inactive, pixelAt(right_inactive_x, kSliderY + kSliderHeight / 2));
}

// Verifies that a range slider paints its active segment strictly between the
// two thumbs while leaving inactive segments on both outer sides.
TEST_F(Material3SliderRenderTest, RangeSliderPaintsActiveTrackBetweenThumbs) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<RangeSlider>(env_, SliderRange{0.0f, 100.0f},
                                              25.0f, 75.0f);
  RangeSlider* slider_ptr = slider.get();

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive = QuantizeToArgb4444(env_.theme().color.secondaryContainer);
  Color background = QuantizeToArgb4444(env_.theme().color.background);
  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int start_handle_x =
      kSliderX + (int)roundf(CenterFromValueForTest(axis, slider_ptr->range(),
                                                    slider_ptr->startValue()));
  int end_handle_x =
      kSliderX + (int)roundf(CenterFromValueForTest(axis, slider_ptr->range(),
                                                    slider_ptr->endValue()));

  EXPECT_EQ(background, pixelAt(kSliderX, kSliderY + 14));
  EXPECT_EQ(inactive, pixelAt(kSliderX + 10, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 48, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(inactive, pixelAt(kSliderX + 85, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(start_handle_x, kSliderY + 5));
  EXPECT_EQ(primary, pixelAt(end_handle_x, kSliderY + 5));
}

// Verifies that visible discrete ticks are painted across both active and
// inactive tracks.
TEST_F(Material3SliderRenderTest,
       DiscreteSliderPaintsTicksOnActiveAndInactiveTracks) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.tick_mode = SliderTickMode::kShowTicks;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f, 0.2f},
                                         0.4f, SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t center_y = kSliderY + slider_ptr->height() / 2;
  int active_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                     axis, slider_ptr->range(), 0.2f));
  int inactive_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                       axis, slider_ptr->range(), 0.8f));

  Color active_stop = QuantizeToArgb4444(env_.theme().color.onPrimary);
  Color inactive_stop =
      QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);

  EXPECT_EQ(active_stop, pixelAt(active_stop_x, center_y));
  EXPECT_EQ(inactive_stop, pixelAt(inactive_stop_x, center_y));
}

// Verifies that stop indicators stay enabled by default even when explicit
// tick rendering is hidden.
TEST_F(Material3SliderRenderTest,
       DiscreteSliderDefaultsToStopIndicatorsWhenTicksAreHidden) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.tick_mode = SliderTickMode::kHidden;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f, 0.2f},
                                         0.4f, SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t center_y = kSliderY + slider_ptr->height() / 2;
  int active_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                     axis, slider_ptr->range(), 0.2f));
  int inactive_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                       axis, slider_ptr->range(), 0.8f));

  Color active_track = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive_stop =
      QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);

  EXPECT_EQ(active_track, pixelAt(active_stop_x, center_y));
  EXPECT_EQ(inactive_stop, pixelAt(inactive_stop_x, center_y));
}

// Verifies that continuous sliders keep the endpoint stop indicator only on the
// inactive side of the track.
TEST_F(Material3SliderRenderTest,
       ContinuousSliderDefaultsToInactiveEndpointStopIndicator) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.tick_mode = SliderTickMode::kHidden;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f, 0.0f},
                                         0.4f, SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t center_y = kSliderY + slider_ptr->height() / 2;
  int active_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                     axis, slider_ptr->range(), 0.0f));
  int inactive_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                       axis, slider_ptr->range(), 1.0f));

  Color active_track = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive_stop =
      QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);

  EXPECT_EQ(active_track, pixelAt(active_stop_x, center_y));
  EXPECT_EQ(inactive_stop, pixelAt(inactive_stop_x, center_y));
}

// Verifies that an inverted extra-large continuous slider still paints the
// endpoint stop indicator on the inactive display side.
TEST_F(Material3SliderRenderTest,
       InvertedXLContinuousSliderPaintsInactiveEndpointStopIndicator) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.size = SliderSize::kExtraLarge;
  style.direction = SliderDirection::kInverted;
  style.tick_mode = SliderTickMode::kHidden;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider =
      std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f, 0.0f}, 0.35f,
                               SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + Scaled(108) - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height(),
                                   false, true);
  int16_t center_y = kSliderY + slider_ptr->height() / 2;
  int active_stop_x = kSliderX + (int)roundf(DisplayCenterFromValueForTest(
                                     axis, slider_ptr->range(), 0.0f));
  int inactive_stop_x = kSliderX + (int)roundf(DisplayCenterFromValueForTest(
                                       axis, slider_ptr->range(), 1.0f));

  Color active_track = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive_stop =
      QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);

  EXPECT_EQ(active_track, pixelAt(active_stop_x, center_y));
  EXPECT_EQ(inactive_stop, pixelAt(inactive_stop_x, center_y));
}

// Verifies that centered discrete sliders suppress the stop mark at the visual
// center gap while still painting active and inactive stops on either side.
TEST_F(Material3SliderRenderTest,
       CenteredDiscreteSliderLeavesCenterGapWithoutStopMark) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.tick_mode = SliderTickMode::kShowTicks;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider =
      std::make_unique<Slider>(env_, SliderRange{-2.0f, 2.0f, 1.0f}, -2.0f,
                               SliderVariant::kCentered, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t center_y = kSliderY + slider_ptr->height() / 2;
  int active_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                     axis, slider_ptr->range(), -1.0f));
  int center_gap_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                    axis, slider_ptr->range(), 0.0f));
  int inactive_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                       axis, slider_ptr->range(), 1.0f));

  Color active_stop = QuantizeToArgb4444(env_.theme().color.onPrimary);
  Color inactive_stop =
      QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(active_stop, pixelAt(active_stop_x, center_y));
  EXPECT_EQ(background, pixelAt(center_gap_x, center_y));
  EXPECT_EQ(inactive_stop, pixelAt(inactive_stop_x, center_y));
}

// Verifies that visible discrete ticks are painted on both inactive outer runs
// and the active selected run of a range slider.
TEST_F(Material3SliderRenderTest,
       DiscreteRangeSliderPaintsTicksAcrossAllTrackSegments) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.tick_mode = SliderTickMode::kShowTicks;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<RangeSlider>(
      env_, SliderRange{0.0f, 1.0f, 0.2f}, 0.2f, 0.8f, style);
  RangeSlider* slider_ptr = slider.get();

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t center_y = kSliderY + slider_ptr->height() / 2;
  int left_inactive_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                            axis, slider_ptr->range(), 0.0f));
  int active_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                     axis, slider_ptr->range(), 0.4f));
  int right_inactive_stop_x = kSliderX + (int)roundf(CenterFromValueForTest(
                                             axis, slider_ptr->range(), 1.0f));

  Color active_stop = QuantizeToArgb4444(env_.theme().color.onPrimary);
  Color inactive_stop =
      QuantizeToArgb4444(env_.theme().color.onSecondaryContainer);

  EXPECT_EQ(inactive_stop, pixelAt(left_inactive_stop_x, center_y));
  EXPECT_EQ(active_stop, pixelAt(active_stop_x, center_y));
  EXPECT_EQ(inactive_stop, pixelAt(right_inactive_stop_x, center_y));
}

// Verifies that pressing a single slider narrows the thumb and tightens the
// track gap, producing the expected changed pixels near the handle.
TEST_F(Material3SliderRenderTest,
       PressedSliderNarrowsThumbAndTightensTrackGap) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto pressed_slider = std::make_unique<Slider>(env_, SliderRange{}, 0.5f);
  slider_ = pressed_slider.get();

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(pressed_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());
  slider().onShowPress((XDim)slider().getPointOverlayFocus().x,
                       slider().height() / 2);
  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(background, pixelAt(kSliderX + 45, kSliderY + 5));
  EXPECT_EQ(primary, pixelAt(kSliderX + 47, kSliderY + kSliderHeight / 2));
}

// Verifies that pressed-state gap tightening does not let discrete stop marks
// spill into the thumb gap near the active handle.
TEST_F(Material3SliderRenderTest,
       PressedDiscreteStopsStayInsideTrackNearThumbGap) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  SliderStyle style{};
  style.tick_mode = SliderTickMode::kShowTicks;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto pressed_slider =
      std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f, 0.1f}, 0.5f,
                               SliderVariant::kStandard, style);
  Slider* slider_ptr = pressed_slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(pressed_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());
  slider_ptr->onShowPress((XDim)slider_ptr->getPointOverlayFocus().x,
                          slider_ptr->height() / 2);
  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  int16_t pressed_thumb_width = Scaled(2);
  int16_t pressed_track_gap = Scaled(5);
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis,
      CenterFromValueForTest(axis, slider_ptr->range(), slider_ptr->value()),
      pressed_thumb_width, Scaled(16), pressed_track_gap, Scaled(44));
  int gap_x = kSliderX + (int)floorf(layout.active_track_max_primary) + 1;
  int gap_y = kSliderY + slider_ptr->height() / 2;

  Color background = QuantizeToArgb4444(env_.theme().color.background);
  EXPECT_EQ(background, pixelAt(gap_x, gap_y));
}

// Verifies that pressing one thumb of a range slider only narrows that thumb
// and tightens its adjacent gap, leaving the rest of the track styling intact.
TEST_F(Material3SliderRenderTest,
       PressedRangeSliderNarrowsOnlyActiveThumbAndTightensGap) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<RangeSlider>(env_, SliderRange{0.0f, 100.0f},
                                              25.0f, 75.0f);
  RangeSlider* slider_ptr = slider.get();

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  XDim start_thumb_center = (XDim)roundf(CenterFromValueForTest(
      axis, slider_ptr->range(), slider_ptr->startValue()));
  slider_ptr->onShowPress(start_thumb_center, slider_ptr->height() / 2);
  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(background, pixelAt(kSliderX + 22, kSliderY + 5));
  EXPECT_EQ(background, pixelAt(kSliderX + 30, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 48, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 70, kSliderY + 5));
}

// Verifies that vertical sliders paint the active run from the bottom toward
// the thumb, leaving the remaining upper segment inactive.
TEST_F(Material3SliderRenderTest,
       VerticalSliderPaintsActiveTrackFromBottomToThumb) {
  constexpr Color kBackdropColor(0xFFF3EFE7);
  constexpr int16_t kVerticalWidth = Scaled(44);
  constexpr int16_t kVerticalHeight = Scaled(60);

  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;

  auto backdrop = std::make_unique<SolidBackdrop>(env_, kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f}, 0.5f,
                                         SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kVerticalWidth - 1,
                            kSliderY + kVerticalHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height(),
                                   true, true);
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis,
      CenterFromValueForTest(axis, slider_ptr->range(), slider_ptr->value()),
      Scaled(4), Scaled(16), Scaled(6), Scaled(44));

  Rect active_rect(
      axis.boxFromPrimaryCross(0, layout.track_cross_start,
                               (int16_t)floorf(layout.active_track_max_primary),
                               layout.track_cross_start + Scaled(16) - 1));
  Rect inactive_rect(axis.boxFromPrimaryCross(
      (int16_t)ceilf(layout.inactive_track_min_primary),
      layout.track_cross_start, axis.primarySpan() - 1,
      layout.track_cross_start + Scaled(16) - 1));

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive = QuantizeToArgb4444(env_.theme().color.secondaryContainer);

  EXPECT_EQ(primary,
            pixelAt(kSliderX + active_rect.xMin() + active_rect.width() / 2,
                    kSliderY + active_rect.yMin() + active_rect.height() / 2));
  EXPECT_EQ(
      inactive,
      pixelAt(kSliderX + inactive_rect.xMin() + inactive_rect.width() / 2,
              kSliderY + inactive_rect.yMin() + inactive_rect.height() / 2));
}

// Verifies that kWithinBounds keeps the bubble hidden at rest, then paints the
// clamped bubble interior once the slider enters the pressed state.
TEST_F(Material3SliderRenderTest,
  WithinBoundsValueIndicatorPaintsBubbleInteriorOnlyDuringInteraction) {
  SliderStyle style{};
  style.value_indicator = SliderValueIndicatorBehavior::kWithinBounds;
  const int16_t kBubbleSliderY = kHeight - kSliderHeight;

  auto slider = std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f}, 0.5f,
                                         SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kBubbleSliderY,
                            kSliderX + kSliderWidth - 1,
                            kBubbleSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  Rect bubble =
      ResolveCurrentIndicatorBoundsForTest(*slider_ptr, env_)
          .translate(kSliderX, kBubbleSliderY);
  ASSERT_FALSE(bubble.empty());
  EXPECT_LT(bubble.yMax(), kBubbleSliderY);

  int16_t sample_x = bubble.xMin() + bubble.width() / 4;
  int16_t sample_y = bubble.yMin() + bubble.height() / 4;
  ASSERT_GE(sample_x, 0);
  ASSERT_LT(sample_x, kWidth);
  ASSERT_GE(sample_y, 0);
  ASSERT_LT(sample_y, kHeight);

  EXPECT_NE(QuantizeToArgb4444(env_.theme().color.inverseSurface),
            pixelAt(sample_x, sample_y));

  roo_display::FpPoint focus = slider_ptr->getPointOverlayFocus();
  slider_ptr->onShowPress((XDim)focus.x, (YDim)focus.y);
  ASSERT_TRUE(app_.refresh());

  EXPECT_EQ(QuantizeToArgb4444(env_.theme().color.inverseSurface),
            pixelAt(sample_x, sample_y));
}

// Verifies that a value-indicator repaint on the single slider only writes into
// the thumb dirty slice plus the current indicator bounds, not the old overlay.
TEST_F(Material3SliderRenderTest,
       ValueIndicatorValueChangePaintIsClippedToDirtySlice) {
  SliderStyle style{};
  style.value_indicator = SliderValueIndicatorBehavior::kAlways;

  auto slider = std::make_unique<ContentPaintSlider>(
      env_, SliderRange{0.0f, 1.0f}, 0.2f, SliderVariant::kStandard, style);
  ContentPaintSlider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  ASSERT_TRUE(slider_ptr->setValue(0.8f));

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  Rect thumb_rect = InvalidationRectForValueChangeForTest(
      axis, slider_ptr->range(), 0.2f, 0.8f);
  Rect expected_clip = Rect::Intersect(thumb_rect, slider_ptr->bounds())
                           .translate(kSliderX, kSliderY);
  Rect old_overlay_clip = Rect::Intersect(ResolveLegacyOverlayClipForTest(
                                              axis, slider_ptr->range(), 0.2f,
                                              0.8f, kPointOverlayDiameter / 2),
                                          slider_ptr->bounds())
                              .translate(kSliderX, kSliderY);
  Rect indicator_bounds =
      ResolveCurrentIndicatorBoundsForTest(*slider_ptr, env_)
          .translate(kSliderX, kSliderY);
  Color clear_color = QuantizeToArgb4444(Color(0xFF1357BD));

  fillScreen(clear_color);
  paintWidgetContentsForTest(*slider_ptr);

  EXPECT_TRUE(
      ExpectPaintConfinedTo({expected_clip, indicator_bounds}, clear_color));
  EXPECT_LT(expected_clip.width(), old_overlay_clip.width());

  Rect full_indicator_envelope = slider_ptr->getParentTransientPaintBounds();
  EXPECT_LT(expected_clip.width(), full_indicator_envelope.width());
}

// Verifies that when a boundary-adjacent track cap shrinks near the start of
// the range, the dirty slice expands all the way to the boundary so the cap is
// fully repainted.
TEST_F(Material3SliderRenderTest,
       NearBoundaryValueChangePaintExpandsToTrackBoundary) {
  auto slider = std::make_unique<ContentPaintSlider>(
      env_, SliderRange{0.0f, 1.0f}, 0.05f, SliderVariant::kStandard);
  ContentPaintSlider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  ASSERT_TRUE(slider_ptr->setValue(0.08f));

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  Rect thumb_rect = InvalidationRectForValueChangeForTest(
      axis, slider_ptr->range(), 0.05f, 0.08f);
  Rect old_clip = Rect::Intersect(thumb_rect, slider_ptr->bounds())
                      .translate(kSliderX, kSliderY);
  Rect expected_clip =
      ExpandedSingleSliderInvalidationRectForTest(
          axis, slider_ptr->style(), slider_ptr->range(), 0.05f, 0.08f,
          Rect::Intersect(thumb_rect, slider_ptr->bounds()))
          .translate(kSliderX, kSliderY);
  Color clear_color = QuantizeToArgb4444(Color(0xFF5C8F1A));

  fillScreen(clear_color);
  paintWidgetContentsForTest(*slider_ptr);

  EXPECT_TRUE(ExpectPaintConfinedTo({expected_clip}, clear_color));
  EXPECT_EQ(kSliderX, expected_clip.xMin());
  EXPECT_GT(expected_clip.width(), old_clip.width());
  EXPECT_NE(clear_color,
            pixelAt(kSliderX, kSliderY + slider_ptr->height() / 2));
}

// Verifies that moving only one thumb of a range slider repaints just that
// thumb's sweep instead of a broader union covering both thumbs.
TEST_F(Material3SliderRenderTest,
       RangeSliderSingleThumbMovePaintIsClippedToMovedThumbSweep) {
  auto slider = std::make_unique<ContentPaintRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);
  ContentPaintRangeSlider* slider_ptr = slider.get();

  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  ASSERT_TRUE(slider_ptr->setValues(35.0f, 75.0f));

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  Rect expected_clip = InvalidationRectForValueChangeForTest(
                           axis, slider_ptr->range(), 25.0f, 35.0f)
                           .translate(kSliderX, kSliderY);
  Rect old_broad_clip =
      Rect::Extent(
          InvalidationRectForValueChangeForTest(axis, slider_ptr->range(),
                                                25.0f, 35.0f),
          axis.invalidationRectForCenterChange(
              CenterFromValueForTest(axis, slider_ptr->range(), 75.0f),
              CenterFromValueForTest(axis, slider_ptr->range(), 75.0f)))
          .translate(kSliderX, kSliderY);

  Color clear_color = QuantizeToArgb4444(Color(0xFF3A6FB0));
  fillScreen(clear_color);
  paintWidgetContentsForTest(*slider_ptr);

  EXPECT_TRUE(ExpectPaintConfinedTo({expected_clip}, clear_color));
  EXPECT_LT(expected_clip.width(), old_broad_clip.width());
}

// Verifies that value-indicator invalidation tracks the measured old and new
// bubble envelopes rather than falling back to a larger conservative region.
TEST_F(Material3SliderRenderTest,
       ValueChangeInvalidatesMeasuredIndicatorEnvelopeOutsideBounds) {
  SliderStyle style{};
  style.value_indicator = SliderValueIndicatorBehavior::kAlways;

  auto panel = std::make_unique<RecordingPanel>(env_);
  RecordingPanel* panel_ptr = panel.get();

  auto slider = std::make_unique<Slider>(env_, SliderRange{0.0f, 1.0f}, 0.2f,
                                         SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  panel_ptr->add(std::move(slider),
                 Rect(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                      kSliderY + kSliderHeight - 1));

  app_.add(std::move(panel), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(app_.refresh());

  panel_ptr->invalidated_regions.clear();
  ASSERT_TRUE(slider_ptr->setValue(0.8f));
  ASSERT_EQ(1u, panel_ptr->invalidated_regions.size());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  float c_old = DisplayCenterFromValueForTest(axis, slider_ptr->range(), 0.2f);
  float c_new = DisplayCenterFromValueForTest(axis, slider_ptr->range(), 0.8f);

  char old_scratch[64];
  char new_scratch[64];
  roo::string_view old_text =
      slider_ptr->formatLabel(0.2f, old_scratch, sizeof(old_scratch));
  roo::string_view new_text =
      slider_ptr->formatLabel(0.8f, new_scratch, sizeof(new_scratch));
  const Theme& th = env_.theme();
  bool clamp =
      style.value_indicator == SliderValueIndicatorBehavior::kWithinBounds;
  int16_t old_bubble_width;
  int16_t old_bubble_height;
  int16_t new_bubble_width;
  int16_t new_bubble_height;
  ValueIndicatorBubble::MeasureBubbleSize(old_text, old_bubble_width,
                                          old_bubble_height);
  ValueIndicatorBubble::MeasureBubbleSize(new_text, new_bubble_width,
                                          new_bubble_height);

  ValueIndicatorBubble old_bubble(th, slider_ptr->isEnabled());
  ASSERT_TRUE(old_bubble.layout(slider_ptr->width(), slider_ptr->height(),
                                c_old, style.orientation, old_text, clamp));

  Rect old_indicator = old_bubble.bounds().translate(kSliderX, kSliderY);
  Rect new_indicator = ValueIndicatorBubble::EnvelopeForCenterRange(
                           slider_ptr->width(), slider_ptr->height(), c_new,
                           c_new, style.value_indicator, style.orientation,
                           new_bubble_width, new_bubble_height)
                           .translate(kSliderX, kSliderY);
  Rect expected = Rect::Extent(old_indicator, new_indicator);
  Rect max_width_sweep =
      ValueIndicatorBubble::EnvelopeForCenterRange(
          slider_ptr->width(), slider_ptr->height(), std::min(c_old, c_new),
          std::max(c_old, c_new), style.value_indicator, style.orientation,
          std::max(old_bubble_width, new_bubble_width),
          std::max(old_bubble_height, new_bubble_height))
          .translate(kSliderX, kSliderY);
  Rect conservative =
      ValueIndicatorBubble::EnvelopeForCenterRange(
          slider_ptr->width(), slider_ptr->height(), std::min(c_old, c_new),
          std::max(c_old, c_new), style.value_indicator, style.orientation)
          .translate(kSliderX, kSliderY);

  EXPECT_EQ(expected, panel_ptr->invalidated_regions.front());
  EXPECT_LT(expected.width(), max_width_sweep.width());
  EXPECT_LT(expected.width(), conservative.width());
}

// Verifies that pressed-state transitions repaint only the thumb slice for the
// single slider, both when pressing and when releasing.
TEST_F(Material3SliderRenderTest, PressStateChangePaintIsClippedToHandleSlice) {
  auto slider = std::make_unique<ContentPaintSlider>(env_, SliderRange{}, 0.5f);
  ContentPaintSlider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  roo_display::FpPoint focus = slider_ptr->getPointOverlayFocus();
  slider_ptr->onShowPress((XDim)focus.x, (YDim)focus.y);

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  Rect expected_clip =
      InvalidationRectForValueChangeForTest(
          axis, slider_ptr->range(), slider_ptr->value(), slider_ptr->value())
          .translate(kSliderX, kSliderY);
  Color clear_color = QuantizeToArgb4444(Color(0xFF2468AC));

  fillScreen(clear_color);
  paintWidgetContentsForTest(*slider_ptr);
  EXPECT_TRUE(ExpectPaintConfinedTo({expected_clip}, clear_color));

  ASSERT_TRUE(slider_ptr->onTouchUp((XDim)focus.x, (YDim)focus.y));

  fillScreen(clear_color);
  paintWidgetContentsForTest(*slider_ptr);
  EXPECT_TRUE(ExpectPaintConfinedTo({expected_clip}, clear_color));
}

// Verifies that showing or hiding the interaction-only value indicator expands
// parent invalidation only by the indicator envelope outside widget bounds.
TEST_F(Material3SliderRenderTest,
       PressStateChangeInvalidatesOnlyIndicatorEnvelopeOutsideBounds) {
  SliderStyle style{};
  style.value_indicator = SliderValueIndicatorBehavior::kShowOnInteraction;

  auto panel = std::make_unique<RecordingPanel>(env_);
  RecordingPanel* panel_ptr = panel.get();

  auto slider = std::make_unique<Slider>(env_, SliderRange{}, 0.5f,
                                         SliderVariant::kStandard, style);
  Slider* slider_ptr = slider.get();
  panel_ptr->add(std::move(slider),
                 Rect(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                      kSliderY + kSliderHeight - 1));

  app_.add(std::move(panel), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(app_.refresh());

  panel_ptr->invalidated_regions.clear();
  roo_display::FpPoint focus = slider_ptr->getPointOverlayFocus();
  slider_ptr->onShowPress((XDim)focus.x, (YDim)focus.y);
  ASSERT_EQ(1u, panel_ptr->invalidated_regions.size());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height());
  float center = DisplayCenterFromValueForTest(axis, slider_ptr->range(),
                                               slider_ptr->value());
  Rect expected = ValueIndicatorBubble::EnvelopeForCenterRange(
                      slider_ptr->width(), slider_ptr->height(), center, center,
                      style.value_indicator, style.orientation)
                      .translate(kSliderX, kSliderY);
  EXPECT_EQ(expected, panel_ptr->invalidated_regions.front());
  EXPECT_LT(expected.width(), slider_ptr->maxParentBounds().width());

  panel_ptr->invalidated_regions.clear();
  ASSERT_TRUE(slider_ptr->onTouchUp((XDim)focus.x, (YDim)focus.y));
  ASSERT_EQ(1u, panel_ptr->invalidated_regions.size());
  EXPECT_EQ(expected, panel_ptr->invalidated_regions.front());
}

// Verifies the same tight repaint behavior for vertical sliders: only the
// thumb slice and current indicator bounds may change after a value update.
TEST_F(Material3SliderRenderTest,
       VerticalValueIndicatorValueChangePaintIsClippedToThumbSlice) {
  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  style.value_indicator = SliderValueIndicatorBehavior::kAlways;

  auto slider = std::make_unique<ContentPaintSlider>(
      env_, SliderRange{0.0f, 1.0f}, 0.2f, SliderVariant::kStandard, style);
  ContentPaintSlider* slider_ptr = slider.get();
  slider_ = slider_ptr;

  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + Scaled(44) - 1,
                            kSliderY + Scaled(56) - 1));

  ASSERT_TRUE(app_.refresh());

  ASSERT_TRUE(slider_ptr->setValue(0.8f));

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height(),
                                   true, true);
  Rect thumb_rect = InvalidationRectForValueChangeForTest(
      axis, slider_ptr->range(), 0.2f, 0.8f);
  Rect expected_clip =
      ExpandedSingleSliderInvalidationRectForTest(
          axis, slider_ptr->style(), slider_ptr->range(), 0.2f, 0.8f,
          Rect::Intersect(thumb_rect, slider_ptr->bounds()))
          .translate(kSliderX, kSliderY);
  Rect old_overlay_clip = Rect::Intersect(ResolveLegacyOverlayClipForTest(
                                              axis, slider_ptr->range(), 0.2f,
                                              0.8f, kPointOverlayDiameter / 2),
                                          slider_ptr->bounds())
                              .translate(kSliderX, kSliderY);
  Rect indicator_bounds =
      ResolveCurrentIndicatorBoundsForTest(*slider_ptr, env_)
          .translate(kSliderX, kSliderY);
  Color clear_color = QuantizeToArgb4444(Color(0xFFB36219));

  fillScreen(clear_color);
  paintWidgetContentsForTest(*slider_ptr);

  EXPECT_TRUE(
      ExpectPaintConfinedTo({expected_clip, indicator_bounds}, clear_color));
  EXPECT_LE(expected_clip.height(), old_overlay_clip.height());

  Rect full_indicator_envelope = slider_ptr->getParentTransientPaintBounds();
  EXPECT_LT(expected_clip.height(), full_indicator_envelope.height());
}

namespace {
class FmtSlider : public Slider {
 public:
  using Slider::Slider;
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    int n = snprintf(scratch, scratch_size, "%.0f%%", value);
    if (n < 0) n = 0;
    if ((size_t)n >= scratch_size) n = scratch_size - 1;
    return roo::string_view(scratch, (size_t)n);
  }
};
}  // namespace

// Verifies the default value-indicator formatter keeps ordinary numeric values
// compact while still producing a sentinel string for non-finite values.
TEST(Material3SliderValueIndicator, DefaultFormatLabelFormatsCompactly) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Slider slider(env);
  char scratch[16];
  EXPECT_EQ("4", slider.formatLabel(4.0f, scratch, sizeof(scratch)));
  EXPECT_EQ("3.14", slider.formatLabel(3.14f, scratch, sizeof(scratch)));
  EXPECT_EQ("0.5", slider.formatLabel(0.50f, scratch, sizeof(scratch)));
  EXPECT_EQ("?", slider.formatLabel(std::numeric_limits<float>::infinity(),
                                    scratch, sizeof(scratch)));
}

// Verifies that subclasses can override value-indicator label formatting and
// that the custom formatter is used verbatim.
TEST(Material3SliderValueIndicator, CustomFormatLabelIsCalled) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  FmtSlider slider(env);
  char scratch[16];
  EXPECT_EQ("42%", slider.formatLabel(42.0f, scratch, sizeof(scratch)));
}

// Verifies that enabling visible value indicators switches the slider into the
// unclipped-parent mode needed to paint bubble overflow.
TEST(Material3SliderValueIndicator, EnabledStyleSetsUnclippedParentMode) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Slider hidden(env, SliderRange{0.0f, 1.0f}, 0.5f, SliderVariant::kStandard,
                SliderStyle{});  // default kHidden
  EXPECT_EQ(ParentClipMode::kClipped, hidden.getParentClipMode());

  SliderStyle on_interaction{};
  on_interaction.value_indicator =
      SliderValueIndicatorBehavior::kShowOnInteraction;
  Slider shown(env, SliderRange{0.0f, 1.0f}, 0.5f, SliderVariant::kStandard,
               on_interaction);
  EXPECT_EQ(ParentClipMode::kUnclipped, shown.getParentClipMode());
}

// Verifies that changing style at runtime toggles the parent clip mode only
// when the value-indicator visibility mode actually changes.
TEST(Material3SliderValueIndicator, SetStyleTogglesParentClipMode) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Slider slider(env);
  EXPECT_EQ(ParentClipMode::kClipped, slider.getParentClipMode());
  SliderStyle s{};
  s.value_indicator = SliderValueIndicatorBehavior::kAlways;
  EXPECT_TRUE(slider.setStyle(s));
  EXPECT_EQ(ParentClipMode::kUnclipped, slider.getParentClipMode());
  EXPECT_FALSE(slider.setStyle(s));
  SliderStyle hidden{};
  EXPECT_TRUE(slider.setStyle(hidden));
  EXPECT_EQ(ParentClipMode::kClipped, slider.getParentClipMode());
}

// Verifies that an always-visible value indicator expands transient paint
// bounds above the slider so the bubble can render outside widget bounds.
TEST(Material3SliderValueIndicator, TransientPaintBoundsExpandAboveWhenAlways) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  SliderStyle s{};
  s.value_indicator = SliderValueIndicatorBehavior::kAlways;
  Slider slider(env, SliderRange{0.0f, 1.0f}, 0.5f, SliderVariant::kStandard,
                s);
  slider.measure(WidthSpec::Exactly(Scaled(96)),
                 HeightSpec::Exactly(Scaled(44)));
  slider.layout(Rect(0, 0, Scaled(96) - 1, Scaled(44) - 1));
  Rect base = slider.getParentTransientPaintBounds();
  // The bubble extends above y=0 in widget-local coords; in parent coords this
  // means above offsetTop(). Since the widget is at (0,0) offsets are zero, so
  // bubble extends into negative y.
  EXPECT_LT(base.yMin(), 0);
}

// Verifies that the same always-visible indicator expansion happens for a
// vertical slider, including leftward overflow as well as upward overflow.
TEST(Material3SliderValueIndicator,
     VerticalTransientPaintBoundsExpandLeftWhenAlways) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  SliderStyle s{};
  s.orientation = SliderOrientation::kVertical;
  s.value_indicator = SliderValueIndicatorBehavior::kAlways;
  Slider slider(env, SliderRange{0.0f, 1.0f}, 0.5f, SliderVariant::kStandard,
                s);
  slider.measure(WidthSpec::Exactly(Scaled(44)),
                 HeightSpec::Exactly(Scaled(96)));
  slider.layout(Rect(0, 0, Scaled(44) - 1, Scaled(96) - 1));
  Rect base = slider.getParentTransientPaintBounds();
  EXPECT_LT(base.xMin(), 0);
  EXPECT_LT(base.yMin(), 0);
}

// Verifies that the range slider reuses the default numeric label formatting
// path for value indicators when no custom formatter is supplied.
TEST(Material3RangeSliderValueIndicator, FormatLabelDefault) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  RangeSlider slider(env, SliderRange{0.0f, 10.0f}, 0.0f, 10.0f);
  char scratch[16];
  EXPECT_EQ("7", slider.formatLabel(7.0f, scratch, sizeof(scratch)));
}

// Verifies that an interaction-only range-slider indicator does not expand the
// transient paint bounds until some thumb is actually active.
TEST(Material3RangeSliderValueIndicator,
     TransientPaintBoundsUnchangedWithoutActivity) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  SliderStyle s{};
  s.value_indicator = SliderValueIndicatorBehavior::kShowOnInteraction;
  RangeSlider slider(env, SliderRange{0.0f, 10.0f}, 2.0f, 8.0f, s);
  slider.measure(WidthSpec::Exactly(Scaled(96)),
                 HeightSpec::Exactly(Scaled(44)));
  slider.layout(Rect(0, 0, Scaled(96) - 1, Scaled(44) - 1));
  Rect bounds = slider.getParentTransientPaintBounds();
  // Not pressed/dragged and not kAlways: no extra expansion above the widget.
  EXPECT_GE(bounds.yMin(), 0);
}

// Verifies that a vertical range slider resolves a tap to the nearest thumb by
// y-position and reports that thumb as active for the ensuing change.
TEST_F(Material3SliderAppTest,
       VerticalRangeSingleTapChoosesNearestThumbByYPosition) {
  SliderStyle style{};
  style.orientation = SliderOrientation::kVertical;
  auto tracking_slider = std::make_unique<TrackingRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 20.0f, 80.0f, style);
  TrackingRangeSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + Scaled(44) - 1,
                            kSliderY + Scaled(60) - 1));
  ASSERT_TRUE(app_.refresh());

  ASSERT_TRUE(tracking->onSingleTapUp(tracking->width() / 2, 2));
  ASSERT_GE(tracking->events.size(), 2u);
  EXPECT_STREQ("start", tracking->events[0]);
  EXPECT_STREQ("value:user:", tracking->events[1]);
  ASSERT_GE(tracking->active_thumbs.size(), 2u);
  EXPECT_EQ(1, tracking->active_thumbs[0]);
  EXPECT_EQ(1, tracking->active_thumbs[1]);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
