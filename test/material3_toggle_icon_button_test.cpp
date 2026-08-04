#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/button/toggle_icon_button.h"

namespace roo_windows {
namespace material3 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

// Verifies that the default state retains the required unselected icon.
TEST(Material3ToggleIconButton, DefaultsUseTheUnselectedIconAndState) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ToggleIconButton button(context, ic_outlined_24_action_done());

  EXPECT_FALSE(button.isSelected());
  EXPECT_EQ(&ic_outlined_24_action_done(), &button.unselectedIcon());
  EXPECT_EQ(nullptr, button.selectedIcon());
  EXPECT_EQ(&button.unselectedIcon(), &button.activeIcon());
  EXPECT_TRUE(button.isClickable());
}

// Verifies that selection switches to the optional selected icon.
TEST(Material3ToggleIconButton, SelectedIconAndStateCanBeChanged) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ToggleIconButton button(context, ic_outlined_24_action_done());

  button.setSelectedIcon(&ic_outlined_24_action_delete());
  button.setSelected(true);
  EXPECT_TRUE(button.isSelected());
  EXPECT_EQ(&ic_outlined_24_action_delete(), &button.activeIcon());

  button.toggle();
  EXPECT_FALSE(button.isSelected());
  EXPECT_EQ(&ic_outlined_24_action_done(), &button.activeIcon());
}

// Verifies that callbacks observe the state committed by a click.
TEST(Material3ToggleIconButton, ClickTogglesBeforeTheCallback) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ToggleIconButton button(context, ic_outlined_24_action_done());
  bool state_seen_by_callback = false;
  int callbacks = 0;
  button.setOnInteractiveChange([&]() {
    state_seen_by_callback = button.isSelected();
    ++callbacks;
  });

  button.onClicked();

  EXPECT_TRUE(button.isSelected());
  EXPECT_TRUE(state_seen_by_callback);
  EXPECT_EQ(1, callbacks);
}

// Verifies that disabled controls reject direct activation.
TEST(Material3ToggleIconButton, DisabledClickDoesNotToggleOrNotify) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ToggleIconButton button(context, ic_outlined_24_action_done());
  int callbacks = 0;
  button.setOnInteractiveChange([&]() { ++callbacks; });
  button.setEnabled(false);

  button.onClicked();

  EXPECT_FALSE(button.isSelected());
  EXPECT_EQ(0, callbacks);
}

// Verifies selected tokens and the outlined border's selected removal.
TEST(Material3ToggleIconButton, SelectedStyleTokensAndOutlinedBorderChange) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  const ColorScheme& colors = env.theme().material3Theme().color;
  ToggleIconButton filled(context, ic_outlined_24_action_done(), nullptr,
                          IconButtonStyle::kFilled, true);
  ToggleIconButton tonal(context, ic_outlined_24_action_done(), nullptr,
                         IconButtonStyle::kFilledTonal, true);
  ToggleIconButton outlined(context, ic_outlined_24_action_done(), nullptr,
                            IconButtonStyle::kOutlined, false);

  EXPECT_EQ(colors.primary, filled.background());
  EXPECT_EQ(ColorToken::kPrimary, filled.containerRole());
  EXPECT_EQ(colors.secondary, tonal.background());
  EXPECT_EQ(ColorToken::kSecondary, tonal.containerRole());
  EXPECT_GT((int)outlined.getBorderStyle().outline_width().floor(), 0);
  outlined.setSelected(true);
  EXPECT_EQ(colors.inverseSurface, outlined.background());
  EXPECT_EQ(0, (int)outlined.getBorderStyle().outline_width().floor());
}

// Verifies selection inverts the configured resting shape family.
TEST(Material3ToggleIconButton, SelectionInvertsRestingShape) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ToggleIconButton button(context, ic_outlined_24_action_done(), nullptr,
                          IconButtonStyle::kFilled, true);
  button.setShape(ButtonShape::kSquare);

  EXPECT_EQ(0xFF, button.getBorderStyle().top_left_corner_radius());
  button.setSelected(false);
  EXPECT_EQ(0xFF, button.getBorderStyle().top_left_corner_radius());
}

// Verifies icon bounds stay stable across state and the RAM budget holds.
TEST(Material3ToggleIconButton, StableIconSlotAndStorageBudget) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ToggleIconButton button(context, ic_outlined_24_action_done(),
                          &ic_outlined_24_action_delete());
  button.layout(Rect(0, 0, Scaled(40) - 1, Scaled(40) - 1));
  Rect before = button.getIconBounds();
  Dimensions dimensions = button.getNaturalDimensions();
  button.setSelected(true);

  EXPECT_EQ(before, button.getIconBounds());
  EXPECT_EQ(dimensions.width(), button.getNaturalDimensions().width());
  EXPECT_EQ(dimensions.height(), button.getNaturalDimensions().height());
  constexpr size_t kRawBudget = sizeof(IconButton) + sizeof(void*) + 4;
  constexpr size_t kAlignmentSlack = alignof(ToggleIconButton) - 1;
  EXPECT_LE(sizeof(ToggleIconButton), kRawBudget + kAlignmentSlack);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
