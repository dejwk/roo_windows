#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/card/flex_card.h"

namespace roo_windows::material3 {
namespace {

ApplicationContext MakeContext(Environment& environment) {
  return ApplicationContext(environment.scheduler(), environment.theme(),
                            environment.keyboardColorTheme());
}

class ColoredParent : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;

  Color background() const override { return Color(127, 12, 34, 56); }
};

static_assert(sizeof(FlexCard) == sizeof(FlexLayout) + 8,
              "Component defaults must not grow FlexCard instances.");

// Verifies each card style resolves its live surface role from the shared
// component theme and owns that fill instead of inheriting its parent's color.
TEST(Material3CardThemeTest, UsesThemeRolesForOwnedCardAndChildSurfaces) {
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = DefaultTheme().material3Theme();
  material.components.card.elevatedContainer = ColorToken::kSurfaceContainer;
  material.components.card.filledContainer = ColorToken::kSurfaceContainerLow;
  material.components.card.outlinedContainer =
      ColorToken::kSurfaceContainerHigh;
  material.color.surfaceContainer = Color(255, 1, 2, 3);
  material.color.surfaceContainerLow = Color(255, 4, 5, 6);
  material.color.surfaceContainerHigh = Color(255, 7, 8, 9);
  Theme theme = DefaultTheme();
  theme.material3_theme = &material;
  Environment environment(scheduler, theme);
  ApplicationContext context = MakeContext(environment);
  ColoredParent parent(context);
  FlexCard elevated(context, FlexCard::Style::kElevated);
  FlexCard filled(context, FlexCard::Style::kFilled);
  FlexCard outlined(context, FlexCard::Style::kOutlined);
  Panel child(context);

  parent.add(elevated);
  elevated.add(child);

  EXPECT_EQ(ColorToken::kSurfaceContainer, elevated.containerRole());
  EXPECT_EQ(material.color.surfaceContainer, elevated.background());
  EXPECT_EQ(elevated.background(), child.background());
  EXPECT_NE(parent.background(), elevated.background());
  EXPECT_EQ(ColorToken::kSurfaceContainerLow, filled.containerRole());
  EXPECT_EQ(material.color.surfaceContainerLow, filled.background());
  EXPECT_EQ(ColorToken::kSurfaceContainerHigh, outlined.containerRole());
  EXPECT_EQ(material.color.surfaceContainerHigh, outlined.background());

  elevated.setContainerRole(ColorToken::kNone);
  EXPECT_EQ(parent.background(), elevated.background());
  EXPECT_EQ(ColorToken::kBackground, elevated.effectiveContainerRole());
}

// Verifies explicit card roles take precedence and clearing restores the live
// component default, including after a style change.
TEST(Material3CardThemeTest, PreservesOverridesAndRestoresLiveThemeDefaults) {
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = DefaultTheme().material3Theme();
  material.components.card.filledContainer = ColorToken::kSurfaceContainerLow;
  material.components.card.outlinedContainer =
      ColorToken::kSurfaceContainerHigh;
  Theme theme = DefaultTheme();
  theme.material3_theme = &material;
  Environment environment(scheduler, theme);
  ApplicationContext context = MakeContext(environment);
  FlexCard card(context, FlexCard::Style::kFilled);

  EXPECT_EQ(ColorToken::kSurfaceContainerLow, card.containerRoleOverride());
  card.setContainerRole(ColorToken::kPrimaryContainer);
  card.setStyle(FlexCard::Style::kOutlined);
  EXPECT_EQ(ColorToken::kPrimaryContainer, card.containerRole());
  EXPECT_EQ(ColorToken::kPrimaryContainer, card.containerRoleOverride());

  card.clearContainerRoleOverride();
  EXPECT_EQ(ColorToken::kSurfaceContainerHigh, card.containerRole());
  material.components.card.outlinedContainer = ColorToken::kSurface;
  EXPECT_EQ(ColorToken::kSurface, card.containerRole());
}

}  // namespace
}  // namespace roo_windows::material3
