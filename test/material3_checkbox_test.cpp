#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
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