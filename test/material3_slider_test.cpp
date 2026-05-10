#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/slider/slider.h"

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
  EXPECT_EQ(primary, pixelAt(kSliderX + 20, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(inactive,
            pixelAt(kSliderX + 70, kSliderY + kSliderHeight / 2));
  EXPECT_EQ(primary, pixelAt(kSliderX + 47, kSliderY + 5));
  EXPECT_EQ(background, pixelAt(kSliderX + 20, kSliderY + 5));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows