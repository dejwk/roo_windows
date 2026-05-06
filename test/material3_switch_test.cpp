#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/switch/switch.h"

namespace roo_windows {
namespace material3 {
namespace {

TEST(Material3Switch, UsesThumbCenteredPointOverlay) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Switch sw(env, false);
  sw.layout(Rect(0, 0, Scaled(52) - 1, Scaled(32) - 1));

  EXPECT_EQ(Widget::OVERLAY_POINT, sw.getOverlayType());

  roo_display::FpPoint off_focus = sw.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(32) - 1), off_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(32) - 1), off_focus.y);

  sw.setOn(true);
  roo_display::FpPoint on_focus = sw.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(
      0.5f * (float)(Scaled(32) - 1) + (float)(Scaled(52) - Scaled(32)),
      on_focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(32) - 1), on_focus.y);
}

TEST(Material3Switch, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Switch sw(env, false);
  Dimensions dims = sw.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(52), dims.width());
  EXPECT_EQ(Scaled(32), dims.height());
}

TEST(Material3Switch, EffectiveContainerRoleTracksState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Switch sw(env, false);
  EXPECT_EQ(ColorRole::kSurfaceContainerHighest, sw.effectiveContainerRole());

  sw.setOn(true);
  EXPECT_EQ(ColorRole::kPrimary, sw.effectiveContainerRole());
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows