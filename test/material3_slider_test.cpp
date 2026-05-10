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

  void paint(const Canvas& canvas) const override { canvas.clear(); }

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

  Slider& addSlider(uint16_t pos) {
    auto slider = std::make_unique<Slider>(env_, pos);
    slider_ = slider.get();
    app_.add(std::move(slider),
             roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                              kSliderY + kSliderHeight - 1));
    EXPECT_TRUE(app_.refresh());
    return *slider_;
  }

  Slider& addSlider(SliderRange range, float value,
                    SliderVariant variant = SliderVariant::kStandard) {
    auto slider = std::make_unique<Slider>(env_, range, value, variant);
    slider_ = slider.get();
    app_.add(std::move(slider),
             roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                              kSliderY + kSliderHeight - 1));
    EXPECT_TRUE(app_.refresh());
    return *slider_;
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
    start_values.push_back(start);
    end_values.push_back(end);
  }

  std::vector<const char*> events;
  std::vector<int> active_thumbs;
  std::vector<float> start_values;
  std::vector<float> end_values;
};

TEST(Material3Slider, UsesZeroDefaultInsets) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_EQ(Padding(0), slider.getPadding());
  EXPECT_EQ(Margins(0), slider.getMargins());
}

TEST(Material3Slider, UsesHandleCenteredPointOverlay) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, 0);
  slider.layout(Rect(0, 0, Scaled(96) - 1, Scaled(44) - 1));

  EXPECT_EQ(Widget::OVERLAY_POINT, slider.getOverlayType());

  roo_display::FpPoint start_focus = slider.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(4) - 1), start_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(44) - 1), start_focus.y);

  slider.setPos(32768);
  roo_display::FpPoint mid_focus = slider.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(96) - 1), mid_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(44) - 1), mid_focus.y);

  slider.setPos(65535);
  roo_display::FpPoint end_focus = slider.getPointOverlayFocus();
  EXPECT_FLOAT_EQ((float)(Scaled(96) - 1) - 0.5f * (float)(Scaled(4) - 1),
                  end_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(44) - 1), end_focus.y);
}

TEST(Material3Slider, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);
  Dimensions dims = slider.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(44), dims.width());
  EXPECT_EQ(Scaled(44), dims.height());
}

TEST(Material3Slider, ReportsNaturalMeasureAsFortyFourByFortyFour) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  Dimensions measured =
      slider.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_EQ(Scaled(44), measured.width());
  EXPECT_EQ(Scaled(44), measured.height());
}

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

TEST(Material3Slider, EffectiveContainerRoleIsPrimary) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_EQ(ColorRole::kPrimary, slider.effectiveContainerRole());
}

TEST(Material3Slider, PressStateDoesNotUseOverlayOrClickAnimation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_FALSE(slider.useOverlayOnPress());
  EXPECT_FALSE(slider.showClickAnimation());
}

TEST(Material3Slider, CompatibilityConstructorUsesUnitRangeAndValueMapping) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, 32768);

  EXPECT_FLOAT_EQ(0.0f, slider.range().from);
  EXPECT_FLOAT_EQ(1.0f, slider.range().to);
  EXPECT_FLOAT_EQ(0.0f, slider.range().step);
  EXPECT_FLOAT_EQ(32768.0f / 65535.0f, slider.value());
  EXPECT_EQ(32768, slider.getPos());
}

TEST(Material3Slider, SemanticConstructorMapsValueToNormalizedPosition) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_FLOAT_EQ(10.0f, slider.range().from);
  EXPECT_FLOAT_EQ(40.0f, slider.range().to);
  EXPECT_FLOAT_EQ(25.0f, slider.value());
  EXPECT_EQ(32768, slider.getPos());
}

TEST(Material3Slider, SetValueClampsIntoConfiguredDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_TRUE(slider.setValue(100.0f));
  EXPECT_FLOAT_EQ(40.0f, slider.value());
  EXPECT_EQ(65535, slider.getPos());
}

TEST(Material3Slider, DiscreteConstructorSnapsInitialValue) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 3.6f);

  EXPECT_FLOAT_EQ(4.0f, slider.value());
  EXPECT_EQ((uint16_t)52428, slider.getPos());
}

TEST(Material3Slider, SetValueSnapsToNearestDiscreteStep) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);

  EXPECT_TRUE(slider.setValue(3.6f));
  EXPECT_FLOAT_EQ(4.0f, slider.value());
  EXPECT_EQ((uint16_t)52428, slider.getPos());
}

TEST(Material3Slider, SetPosSnapsToNearestDiscreteStep) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);

  EXPECT_TRUE(slider.setPos((uint16_t)47185));
  EXPECT_FLOAT_EQ(4.0f, slider.value());
  EXPECT_EQ((uint16_t)52428, slider.getPos());
}

TEST(Material3Slider, SetRangeClampsCurrentValueIntoNewDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_TRUE(slider.setRange(SliderRange{30.0f, 50.0f}));
  EXPECT_FLOAT_EQ(30.0f, slider.range().from);
  EXPECT_FLOAT_EQ(50.0f, slider.range().to);
  EXPECT_FLOAT_EQ(30.0f, slider.value());
  EXPECT_EQ(0, slider.getPos());
}

TEST(Material3Slider, InvalidRangeIsRejectedWithoutChangingState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{10.0f, 40.0f}, 25.0f);

  EXPECT_FALSE(slider.setRange(SliderRange{40.0f, 10.0f}));
  EXPECT_FLOAT_EQ(10.0f, slider.range().from);
  EXPECT_FLOAT_EQ(40.0f, slider.range().to);
  EXPECT_FLOAT_EQ(25.0f, slider.value());
  EXPECT_EQ(32768, slider.getPos());
}

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

TEST(Material3Slider, NegativeStepIsRejected) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 2.0f);

  EXPECT_FALSE(slider.setRange(SliderRange{0.0f, 5.0f, -1.0f}));
  EXPECT_FLOAT_EQ(1.0f, slider.range().step);
}

TEST(Material3Slider, CenteredVariantPreservesValueMapping) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{-100.0f, 100.0f}, -20.0f,
                SliderVariant::kCentered);

  EXPECT_EQ(SliderVariant::kCentered, slider.variant());
  EXPECT_FLOAT_EQ(-20.0f, slider.value());
  EXPECT_EQ((uint16_t)26214, slider.getPos());
}

TEST(Material3Slider, SetVariantUpdatesSemanticVariantOnly) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{-100.0f, 100.0f}, -20.0f);

  EXPECT_TRUE(slider.setVariant(SliderVariant::kCentered));
  EXPECT_EQ(SliderVariant::kCentered, slider.variant());
  EXPECT_FLOAT_EQ(-20.0f, slider.value());
  EXPECT_EQ((uint16_t)26214, slider.getPos());
}

TEST(Material3Slider, CenteredDiscreteVariantStillSnapsValues) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, SliderRange{-5.0f, 5.0f, 0.5f}, 0.0f,
                SliderVariant::kCentered);

  EXPECT_TRUE(slider.setValue(1.3f));
  EXPECT_FLOAT_EQ(1.5f, slider.value());
  EXPECT_EQ((uint16_t)42598, slider.getPos());
}

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

TEST(Material3Slider, ProgrammaticPositionChangeFiresOnlyValueChangeHook) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  TrackingSlider slider(env, SliderRange{0.0f, 10.0f}, 0.0f);

  EXPECT_TRUE(slider.setPos((uint16_t)32768));
  ASSERT_EQ(1u, slider.events.size());
  EXPECT_STREQ("value:program:", slider.events[0]);
  ASSERT_EQ(1u, slider.values.size());
  EXPECT_FLOAT_EQ(5.0000763f, slider.values[0]);
}

TEST(Material3RangeSlider, ConstructorOrdersAndSnapsInitialValues) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 5.0f, 1.0f}, 4.6f, 1.2f);

  EXPECT_FLOAT_EQ(0.0f, slider.range().from);
  EXPECT_FLOAT_EQ(5.0f, slider.range().to);
  EXPECT_FLOAT_EQ(1.0f, slider.startValue());
  EXPECT_FLOAT_EQ(5.0f, slider.endValue());
}

TEST(Material3RangeSlider, SetValuesClampsSnapsAndOrdersIntoDomain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{10.0f, 40.0f, 5.0f}, 15.0f, 30.0f);

  EXPECT_TRUE(slider.setValues(42.0f, 11.0f));
  EXPECT_FLOAT_EQ(10.0f, slider.startValue());
  EXPECT_FLOAT_EQ(40.0f, slider.endValue());
}

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

TEST(Material3RangeSlider, NegativeMinimumSeparationIsRejected) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 10.0f}, 2.0f, 8.0f);

  EXPECT_FALSE(slider.setMinSeparation(-1.0f));
  EXPECT_FLOAT_EQ(0.0f, slider.minSeparation());
}

TEST(Material3RangeSlider, PressStateDoesNotUseOverlayOrClickAnimation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RangeSlider slider(env, SliderRange{0.0f, 10.0f}, 2.0f, 8.0f);

  EXPECT_FALSE(slider.useOverlayOnPress());
  EXPECT_FALSE(slider.showClickAnimation());
}

TEST_F(Material3SliderAppTest,
       RangeSliderOverlayFocusTracksTappedAndDraggedThumb) {
  auto tracking_slider = std::make_unique<TrackingRangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);
  TrackingRangeSlider* tracking = tracking_slider.get();
  app_.add(std::move(tracking_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(tracking->width(), tracking->height(),
                                   Scaled(4), Scaled(20));

  XDim tap_x = Scaled(84);
  ASSERT_TRUE(tracking->onSingleTapUp(tap_x, tracking->height() / 2));
  roo_display::FpPoint tapped_focus = tracking->getPointOverlayFocus();
  EXPECT_FLOAT_EQ(axis.centerFromPos(internal::SliderPosFromValue(
                      0.0f, 100.0f, tracking->endValue())),
                  tapped_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(tracking->height() - 1), tapped_focus.y);

  XDim start_thumb_center = (XDim)roundf(axis.centerFromPos(
      internal::SliderPosFromValue(0.0f, 100.0f, tracking->startValue())));
  tracking->onShowPress(start_thumb_center, tracking->height() / 2);
  ASSERT_TRUE(tracking->onScroll(Scaled(42), tracking->height() / 2,
                                 Scaled(8), 0));

  roo_display::FpPoint dragged_focus = tracking->getPointOverlayFocus();
  EXPECT_FLOAT_EQ(axis.centerFromPos(internal::SliderPosFromValue(
                      0.0f, 100.0f, tracking->startValue())),
                  dragged_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(tracking->height() - 1), dragged_focus.y);
}

TEST_F(Material3SliderAppTest, HorizontalScrollUpdatesPosition) {
  Slider& slider = addSlider(0);

  EXPECT_TRUE(slider.onScroll(Scaled(96) - 1, Scaled(22), Scaled(12), 0));
  EXPECT_GT(slider.getPos(), 60000);
}

TEST_F(Material3SliderAppTest,
       VerticalDominantScrollDoesNotCaptureWhenNotDragging) {
  Slider& slider = addSlider(12345);

  EXPECT_FALSE(slider.onScroll(Scaled(48), Scaled(22), 1, 6));
  EXPECT_EQ(12345, slider.getPos());
}

TEST_F(Material3SliderAppTest, TapToJumpUsesCurrentNormalizedMapping) {
  Slider& slider = addSlider(0);

  EXPECT_TRUE(slider.onSingleTapUp(Scaled(48) - 1, slider.height() / 2));
  EXPECT_EQ(32768, slider.getPos());
}

TEST_F(Material3SliderAppTest, TapToJumpSnapsToNearestDiscreteStep) {
  auto discrete_slider = std::make_unique<Slider>(
      env_, SliderRange{0.0f, 5.0f, 1.0f}, 0.0f);
  slider_ = discrete_slider.get();
  app_.add(std::move(discrete_slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(slider().onSingleTapUp(Scaled(70), slider().height() / 2));
  EXPECT_FLOAT_EQ(4.0f, slider().value());
  EXPECT_EQ((uint16_t)52428, slider().getPos());
}

TEST_F(Material3SliderAppTest,
       InteractiveChangeFiresOnlyForUserOriginatedPositionChanges) {
  Slider& slider = addSlider(0);

  int interactive_change_count = 0;
  slider.setOnInteractiveChange([&]() { ++interactive_change_count; });

  EXPECT_TRUE(slider.setPos(1000));
  EXPECT_EQ(0, interactive_change_count);

  slider.setPos(0);
  EXPECT_EQ(0, interactive_change_count);

  EXPECT_TRUE(
      slider.onSingleTapUp(Scaled(48) - 1, slider.height() / 2));
  EXPECT_EQ(1, interactive_change_count);
  EXPECT_EQ(32768, slider.getPos());

  EXPECT_TRUE(
      slider.onSingleTapUp(Scaled(48) - 1, slider.height() / 2));
  EXPECT_EQ(1, interactive_change_count);

  EXPECT_TRUE(slider.onScroll(Scaled(96) - 1, slider.height() / 2,
                              Scaled(12), 0));
  EXPECT_EQ(2, interactive_change_count);
  EXPECT_GT(slider.getPos(), 60000);
}

TEST_F(Material3SliderAppTest,
       TapLifecycleFiresStartValueCompatibilityEndInOrder) {
  auto tracking_slider = std::make_unique<TrackingSlider>(env_, 0);
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
  EXPECT_FLOAT_EQ(32768.0f / 65535.0f, tracking->values[0]);
  EXPECT_FLOAT_EQ(32768.0f / 65535.0f, tracking->values[1]);
}

TEST_F(Material3SliderAppTest,
       DragLifecycleFiresSingleStartAndEndsOnCancel) {
  auto tracking_slider = std::make_unique<TrackingSlider>(env_, 0);
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

  internal::SliderAxisMetrics axis(tracking->width(), tracking->height(),
                                   Scaled(4), 0);
  XDim start_thumb_center = (XDim)roundf(axis.centerFromPos(
      internal::SliderPosFromValue(0.0f, 100.0f, tracking->startValue())));
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

TEST_F(Material3SliderAppTest,
       OverlappingRangeThumbsWaitForDirectionalIntent) {
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

TEST_F(Material3SliderRenderTest,
       PaintsCurrentTrackAndHandleGeometryAtMidpointPosition) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(
      env_, kBackdropColor, Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(env_, 32768);
  slider_ = slider.get();

  app_.add(std::move(backdrop), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
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
  EXPECT_EQ(inactive,
            pixelAt(kSliderX + 70, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 47, kSliderY + 5));
  EXPECT_EQ(background, pixelAt(kSliderX + 20, kSliderY + 5));
}

TEST_F(Material3SliderRenderTest,
       CenteredVariantPaintsActiveTrackFromVisualMidpoint) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(
      env_, kBackdropColor, Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<Slider>(
      env_, SliderRange{-100.0f, 100.0f}, -20.0f, SliderVariant::kCentered);
  slider_ = slider.get();

  app_.add(std::move(backdrop), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive = QuantizeToArgb4444(env_.theme().color.secondaryContainer);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(inactive, pixelAt(kSliderX + 25, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(background, pixelAt(kSliderX + 45, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 47, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(inactive, pixelAt(kSliderX + 60, kSliderY + kSliderHeight / 2));
}

TEST_F(Material3SliderRenderTest,
       RangeSliderPaintsActiveTrackBetweenThumbs) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(
      env_, kBackdropColor, Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<RangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);

  app_.add(std::move(backdrop), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color inactive = QuantizeToArgb4444(env_.theme().color.secondaryContainer);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(background, pixelAt(kSliderX, kSliderY + 14));
  EXPECT_EQ(inactive, pixelAt(kSliderX + 10, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 48, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(inactive, pixelAt(kSliderX + 85, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 24, kSliderY + 5));
  EXPECT_EQ(primary, pixelAt(kSliderX + 70, kSliderY + 5));
}

TEST_F(Material3SliderRenderTest,
       PressedSliderNarrowsThumbAndTightensTrackGap) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(
      env_, kBackdropColor, Dimensions(kWidth, kHeight));
  auto pressed_slider = std::make_unique<Slider>(env_, 32768);
  slider_ = pressed_slider.get();

  app_.add(std::move(backdrop), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
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

TEST_F(Material3SliderRenderTest,
       PressedRangeSliderNarrowsOnlyActiveThumbAndTightensGap) {
  constexpr Color kBackdropColor(0xFFF3EFE7);

  auto backdrop = std::make_unique<SolidBackdrop>(
      env_, kBackdropColor, Dimensions(kWidth, kHeight));
  auto slider = std::make_unique<RangeSlider>(
      env_, SliderRange{0.0f, 100.0f}, 25.0f, 75.0f);
  RangeSlider* slider_ptr = slider.get();

  app_.add(std::move(backdrop), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app_.add(std::move(slider),
           roo_display::Box(kSliderX, kSliderY, kSliderX + kSliderWidth - 1,
                            kSliderY + kSliderHeight - 1));

  ASSERT_TRUE(app_.refresh());

  internal::SliderAxisMetrics axis(slider_ptr->width(), slider_ptr->height(),
                                   Scaled(4), Scaled(20));
  XDim start_thumb_center = (XDim)roundf(axis.centerFromPos(
      internal::SliderPosFromValue(0.0f, 100.0f, slider_ptr->startValue())));
  slider_ptr->onShowPress(start_thumb_center, slider_ptr->height() / 2);
  ASSERT_TRUE(app_.refresh());

  Color primary = QuantizeToArgb4444(env_.theme().color.primary);
  Color background = QuantizeToArgb4444(env_.theme().color.background);

  EXPECT_EQ(background, pixelAt(kSliderX + 22, kSliderY + 5));
  EXPECT_EQ(background, pixelAt(kSliderX + 30, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 48, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 70, kSliderY + 5));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows