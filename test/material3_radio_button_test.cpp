#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/radio_button/radio_button.h"

namespace roo_windows {
namespace material3 {
namespace {

// Verifies that the radio button contributes zero padding and zero margins,
// so its laid-out bounds map directly to its visual footprint.
TEST(Material3RadioButton, UsesZeroDefaultInsets) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RadioButton radio(env, false);

  EXPECT_EQ(0, radio.getPadding().left());
  EXPECT_EQ(0, radio.getPadding().top());
  EXPECT_EQ(0, radio.getPadding().right());
  EXPECT_EQ(0, radio.getPadding().bottom());

  EXPECT_EQ(0, radio.getMargins().left());
  EXPECT_EQ(0, radio.getMargins().top());
  EXPECT_EQ(0, radio.getMargins().right());
  EXPECT_EQ(0, radio.getMargins().bottom());
}

// Verifies that the radio button advertises a POINT overlay anchored at the
// geometric center of its bounds, ensuring press ripples emanate from the
// dot rather than the top-left corner.
TEST(Material3RadioButton, UsesCenteredPointOverlay) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RadioButton radio(env, false);
  radio.layout(Rect(0, 0, Scaled(20) - 1, Scaled(20) - 1));

  EXPECT_EQ(Widget::OVERLAY_POINT, radio.getOverlayType());

  roo_display::FpPoint focus = radio.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(20) - 1), focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(20) - 1), focus.y);
}

// Verifies that the radio button advertises the Material 3 prescribed 20x20
// dp minimum dimensions used by parent layouts during measurement.
TEST(Material3RadioButton, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RadioButton radio(env, false);
  Dimensions dims = radio.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(20), dims.width());
  EXPECT_EQ(Scaled(20), dims.height());
}

// Verifies that effectiveContainerRole resolves to kSurface while off and
// kPrimary while on, so the rendered ring and dot pick up the selected
// accent color.
TEST(Material3RadioButton, EffectiveContainerRoleTracksSelectionState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  RadioButton radio(env, false);
  EXPECT_EQ(ColorRole::kSurface, radio.effectiveContainerRole());

  radio.setOn();
  EXPECT_EQ(ColorRole::kPrimary, radio.effectiveContainerRole());
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows