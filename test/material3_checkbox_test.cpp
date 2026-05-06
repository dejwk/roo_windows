#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/checkbox/checkbox.h"

namespace roo_windows {
namespace material3 {
namespace {

TEST(Material3Checkbox, UsesCenteredPointOverlay) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Checkbox checkbox(env, OnOffState::kOff);
  checkbox.layout(Rect(0, 0, Scaled(18) - 1, Scaled(18) - 1));

  EXPECT_EQ(Widget::OVERLAY_POINT, checkbox.getOverlayType());

  roo_display::FpPoint focus = checkbox.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(18) - 1), focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(18) - 1), focus.y);
}

TEST(Material3Checkbox, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Checkbox checkbox(env, OnOffState::kOff);
  Dimensions dims = checkbox.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(18), dims.width());
  EXPECT_EQ(Scaled(18), dims.height());
}

TEST(Material3Checkbox, ReportsNaturalMeasureAsEighteenByEighteen) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Checkbox checkbox(env, OnOffState::kOff);

  Dimensions measured =
      checkbox.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_EQ(Scaled(18), measured.width());
  EXPECT_EQ(Scaled(18), measured.height());
}

TEST(Material3Checkbox, UsesZeroInsetsAndMeasuredContentBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Checkbox checkbox(env, OnOffState::kOff);
  Dimensions measured =
      checkbox.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  checkbox.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));

  EXPECT_EQ(Padding(0), checkbox.getPadding());
  EXPECT_EQ(Margins(0), checkbox.getMargins());
  EXPECT_EQ(Rect(0, 0, measured.width() - 1, measured.height() - 1),
            checkbox.bounds());
  EXPECT_EQ(Rect(0, 0, measured.width() - 1, measured.height() - 1),
            checkbox.getContentBounds());
}

TEST(Material3Checkbox, EffectiveContainerRoleTracksSelectionState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Checkbox checkbox(env, OnOffState::kOff);
  EXPECT_EQ(ColorRole::kSurface, checkbox.effectiveContainerRole());

  checkbox.setOnOffState(OnOffState::kIndeterminate);
  EXPECT_EQ(ColorRole::kPrimary, checkbox.effectiveContainerRole());

  checkbox.setOn();
  EXPECT_EQ(ColorRole::kPrimary, checkbox.effectiveContainerRole());
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows