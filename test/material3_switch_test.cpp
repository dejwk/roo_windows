#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/switch/switch.h"

namespace roo_windows {
namespace material3 {
namespace {

// Verifies that the Material 3 switch advertises a POINT overlay anchored at
// the thumb center and that the focus point tracks the thumb as the switch
// toggles between off (left) and on (right) positions.
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

// Verifies that the switch reports the Material 3 prescribed minimum size
// (52x32 dp) for layout, independent of its current on/off state.
TEST(Material3Switch, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Switch sw(env, false);
  Dimensions dims = sw.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(52), dims.width());
  EXPECT_EQ(Scaled(32), dims.height());
}

// Verifies that the effective container color role flips with the switch
// state: surfaceContainerHighest while off (so the track reads as a neutral
// inset) and primary while on (so the track adopts the selected accent).
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