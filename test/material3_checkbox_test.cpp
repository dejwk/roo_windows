#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/checkbox/checkbox.h"

namespace roo_windows {
namespace material3 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

// Verifies that the checkbox advertises a POINT overlay anchored at the
// geometric center of its laid-out bounds, so press ripples expand from the
// box's middle rather than the top-left corner.
TEST(Material3Checkbox, UsesCenteredPointOverlay) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Checkbox checkbox(context, OnOffState::kOff);
  checkbox.layout(Rect(0, 0, Scaled(18) - 1, Scaled(18) - 1));

  EXPECT_EQ(Widget::OVERLAY_POINT, checkbox.getOverlayType());

  roo_display::FpPoint focus = checkbox.getPointOverlayFocus();
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(18) - 1), focus.x);
  EXPECT_FLOAT_EQ(0.5f * (float)(Scaled(18) - 1), focus.y);
}

// Verifies that the checkbox advertises the Material 3 prescribed 18x18 dp
// minimum dimensions, used by parent layouts during measurement.
TEST(Material3Checkbox, ReportsMaterial3MinimumSize) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Checkbox checkbox(context, OnOffState::kOff);
  Dimensions dims = checkbox.getSuggestedMinimumDimensions();

  EXPECT_EQ(Scaled(18), dims.width());
  EXPECT_EQ(Scaled(18), dims.height());
}

// Verifies that under Unspecified width/height measure specs the checkbox
// measures itself at its natural 18x18 dp footprint without inflating to fill
// the unspecified parent.
TEST(Material3Checkbox, ReportsNaturalMeasureAsEighteenByEighteen) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Checkbox checkbox(context, OnOffState::kOff);

  Dimensions measured =
      checkbox.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_EQ(Scaled(18), measured.width());
  EXPECT_EQ(Scaled(18), measured.height());
}

// Verifies that the checkbox contributes no padding or margins of its own and
// that after layout its content bounds coincide with its laid-out bounds,
// so callers can predict the painted region from layout dimensions alone.
TEST(Material3Checkbox, UsesZeroInsetsAndMeasuredContentBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Checkbox checkbox(context, OnOffState::kOff);
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

// Verifies that effectiveContainerRole reports kSurface while off (neutral
// outline color) and kPrimary while indeterminate or on, so theming picks up
// the selected accent for both filled states.
TEST(Material3Checkbox, EffectiveContainerRoleTracksSelectionState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Checkbox checkbox(context, OnOffState::kOff);
  EXPECT_EQ(ColorRole::kSurface, checkbox.effectiveContainerRole());

  checkbox.setOnOffState(OnOffState::kIndeterminate);
  EXPECT_EQ(ColorRole::kPrimary, checkbox.effectiveContainerRole());

  checkbox.setOn();
  EXPECT_EQ(ColorRole::kPrimary, checkbox.effectiveContainerRole());
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows