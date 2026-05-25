#include "roo_windows/material3/card/flex_card.h"

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace material3 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

// Verifies that each of the three card styles (elevated, filled, outlined)
// seeds its container role, resting elevation, corner radius, and outline
// width to the values prescribed by the Material 3 spec.
TEST(FlexCard, ConstructorSeedsStyleDefaults) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  FlexCard elevated(context, FlexCard::Style::kElevated);
  EXPECT_EQ(ColorRole::kSurfaceContainerLow, elevated.containerRole());
  EXPECT_EQ(3, elevated.getElevation());
  EXPECT_EQ((uint8_t)Scaled(12),
            elevated.getBorderStyle().corner_radii().max());
  EXPECT_EQ(SmallNumber(0), elevated.getBorderStyle().outline_width());

  FlexCard filled(context, FlexCard::Style::kFilled);
  EXPECT_EQ(ColorRole::kSurfaceContainerHighest, filled.containerRole());
  EXPECT_EQ(0, filled.getElevation());
  EXPECT_EQ((uint8_t)Scaled(12), filled.getBorderStyle().corner_radii().max());
  EXPECT_EQ(SmallNumber(0), filled.getBorderStyle().outline_width());

  FlexCard outlined(context, FlexCard::Style::kOutlined);
  EXPECT_EQ(ColorRole::kSurface, outlined.containerRole());
  EXPECT_EQ(0, outlined.getElevation());
  EXPECT_EQ((uint8_t)Scaled(12),
            outlined.getBorderStyle().corner_radii().max());
  EXPECT_EQ(SmallNumber::Of16ths(Scaled(16)),
            outlined.getBorderStyle().outline_width());
}

// Verifies that calling setStyle() on a card with no per-property overrides
// updates every style-derived value (container role, elevation, outline
// width, corner radius) to the new style's defaults.
TEST(FlexCard, StyleChangeUpdatesNonOverriddenValues) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  FlexCard card(context, FlexCard::Style::kFilled);
  card.setStyle(FlexCard::Style::kOutlined);

  EXPECT_EQ(ColorRole::kSurface, card.containerRole());
  EXPECT_EQ(0, card.getElevation());
  EXPECT_EQ((uint8_t)Scaled(12), card.getBorderStyle().corner_radii().max());
  EXPECT_EQ(SmallNumber::Of16ths(Scaled(16)),
            card.getBorderStyle().outline_width());
}

// Verifies that explicit per-property overrides (container role, outline
// role, elevation, outline width, corner radius) persist across a subsequent
// setStyle() call, so the user's customizations are not silently reset.
TEST(FlexCard, OverridesPersistAcrossStyleChanges) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  FlexCard card(context, FlexCard::Style::kFilled);
  card.setContainerRole(ColorRole::kPrimaryContainer);
  card.setOutlineRole(ColorRole::kOutline);
  card.setElevation(7);
  card.setOutlineWidth(SmallNumber::Of16ths(9));
  card.setCornerRadius(5);

  card.setStyle(FlexCard::Style::kOutlined);

  EXPECT_EQ(ColorRole::kPrimaryContainer, card.containerRole());
  EXPECT_EQ(ColorRole::kOutline, card.outlineRoleOverride());
  EXPECT_EQ(7, card.getElevation());
  EXPECT_EQ(SmallNumber::Of16ths(9), card.getBorderStyle().outline_width());
  EXPECT_EQ(5, card.getBorderStyle().corner_radii().max());
}

// Verifies that clearing each per-property override returns the card to the
// defaults of the currently-selected style, even if those defaults differ
// from the originally-configured style.
TEST(FlexCard, ClearOverrideRecomputesFromCurrentStyle) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  FlexCard card(context, FlexCard::Style::kFilled);

  card.setStyle(FlexCard::Style::kElevated);
  card.setContainerRole(ColorRole::kPrimary);
  card.setOutlineRole(ColorRole::kSecondary);
  card.setElevation(11);
  card.setOutlineWidth(SmallNumber::Of16ths(3));
  card.setCornerRadius(2);

  card.setStyle(FlexCard::Style::kOutlined);

  card.clearContainerRoleOverride();
  card.clearOutlineRoleOverride();
  card.clearElevationOverride();
  card.clearOutlineWidthOverride();
  card.clearCornerRadiusOverride();

  EXPECT_EQ(ColorRole::kSurface, card.containerRole());
  EXPECT_EQ(ColorRole::kOutlineVariant, card.outlineRoleOverride());
  EXPECT_EQ(0, card.getElevation());
  EXPECT_EQ(SmallNumber::Of16ths(Scaled(16)),
            card.getBorderStyle().outline_width());
  EXPECT_EQ((uint8_t)Scaled(12), card.getBorderStyle().corner_radii().max());
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
