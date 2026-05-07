#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/slider/slider.h"

namespace roo_windows {
namespace material3 {
namespace {

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

TEST(Material3Slider, EffectiveContainerRoleIsPrimary) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env);

  EXPECT_EQ(ColorRole::kPrimary, slider.effectiveContainerRole());
}

TEST(Material3Slider, HorizontalScrollUpdatesPosition) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Slider slider(env, 0);
  slider.layout(Rect(0, 0, Scaled(96) - 1, Scaled(44) - 1));

  EXPECT_TRUE(slider.onScroll(Scaled(96) - 1, Scaled(22), Scaled(12), 0));
  EXPECT_GT(slider.getPos(), 60000);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows