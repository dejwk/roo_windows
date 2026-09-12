#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace material3 {
namespace {

using test_support::RooWindowsRenderTestSized;

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

class TestToggleIconButton : public ToggleIconButton {
 public:
  using ToggleIconButton::ToggleIconButton;

  bool selectionAnimationActive() const {
    return context().animations().contains(*this, kSelection);
  }
};

class ToggleIconButtonAnimationTest
    : public RooWindowsRenderTestSized<Scaled(80), Scaled(56)> {
 protected:
  TestToggleIconButton* AddButton() {
    auto button = std::make_unique<TestToggleIconButton>(
        context(), ic_outlined_24_action_done());
    TestToggleIconButton* result = button.get();
    app_.add(std::move(button),
             roo_display::Box(Scaled(8), Scaled(8), Scaled(48) - 1,
                              Scaled(48) - 1));
    return result;
  }
};

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

// Verifies a programmatic selection morphs from the prior resting shape.
TEST_F(ToggleIconButtonAnimationTest, ProgrammaticSelectionUsesValueTrack) {
  TestToggleIconButton* button = AddButton();
  ASSERT_TRUE(refresh());

  button->setSelected(true);
  EXPECT_TRUE(button->selectionAnimationActive());
  EXPECT_EQ(0xFF, button->getBorderStyle().top_left_corner_radius());

  ASSERT_TRUE(refresh());
  delay(50);
  ASSERT_TRUE(refresh());
  uint8_t midpoint = button->getBorderStyle().top_left_corner_radius();
  EXPECT_GT(midpoint, Scaled(12));
  EXPECT_LT(midpoint, 0xFF);

  delay(60);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button->selectionAnimationActive());
  EXPECT_EQ(Scaled(12), button->getBorderStyle().top_left_corner_radius());
}

// Verifies an input-driven selection starts at pressed shape and reaches rest.
TEST_F(ToggleIconButtonAnimationTest, ReleaseMorphsFromPressedShapeToRest) {
  TestToggleIconButton* button = AddButton();
  ASSERT_TRUE(refresh());

  button->onClicked();
  EXPECT_TRUE(button->isSelected());
  EXPECT_TRUE(button->selectionAnimationActive());
  EXPECT_EQ(Scaled(8), button->getBorderStyle().top_left_corner_radius());

  ASSERT_TRUE(refresh());
  delay(110);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button->selectionAnimationActive());
  EXPECT_EQ(Scaled(12), button->getBorderStyle().top_left_corner_radius());
}

// Verifies click appearance keeps precedence while the selection track runs.
TEST_F(ToggleIconButtonAnimationTest, ClickAndSelectionAnimationsOverlap) {
  TestToggleIconButton* button = AddButton();
  ASSERT_TRUE(refresh());

  button->onSingleTapUp(button->width() / 2, button->height() / 2);
  EXPECT_TRUE(button->isSelected());
  EXPECT_TRUE(button->isClicking());
  EXPECT_TRUE(button->selectionAnimationActive());

  ASSERT_TRUE(refresh());
  delay(50);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(button->isClicking());
  EXPECT_TRUE(button->selectionAnimationActive());

  delay(60);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(button->isClicking());
  EXPECT_FALSE(button->selectionAnimationActive());

  delay(250);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button->isClicking());
  EXPECT_EQ(Scaled(12), button->getBorderStyle().top_left_corner_radius());
}

// Verifies rapid state changes retarget from the last applied radius.
TEST_F(ToggleIconButtonAnimationTest, RapidSelectionRetargetsContinuously) {
  TestToggleIconButton* button = AddButton();
  ASSERT_TRUE(refresh());

  button->setSelected(true);
  ASSERT_TRUE(refresh());
  delay(40);
  ASSERT_TRUE(refresh());
  uint8_t midpoint = button->getBorderStyle().top_left_corner_radius();
  ASSERT_GT(midpoint, Scaled(12));
  ASSERT_LT(midpoint, 0xFF);

  button->setSelected(false);
  EXPECT_TRUE(button->selectionAnimationActive());
  EXPECT_EQ(midpoint, button->getBorderStyle().top_left_corner_radius());
  ASSERT_TRUE(refresh());
  delay(40);
  ASSERT_TRUE(refresh());
  EXPECT_GT(button->getBorderStyle().top_left_corner_radius(), midpoint);

  delay(70);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button->selectionAnimationActive());
  EXPECT_EQ(0xFF, button->getBorderStyle().top_left_corner_radius());
}

// Verifies hiding cancels and snaps without resuming stale selection work.
TEST_F(ToggleIconButtonAnimationTest, HiddenButtonSnapsWithoutResume) {
  TestToggleIconButton* button = AddButton();
  ASSERT_TRUE(refresh());
  button->setSelected(true);
  ASSERT_TRUE(refresh());
  delay(40);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(button->selectionAnimationActive());

  button->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button->selectionAnimationActive());
  EXPECT_EQ(Scaled(12), button->getBorderStyle().top_left_corner_radius());

  button->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  delay(120);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button->selectionAnimationActive());
  EXPECT_EQ(Scaled(12), button->getBorderStyle().top_left_corner_radius());
}

// Verifies detaching a borrowed button cancels and snaps its selection track.
TEST_F(ToggleIconButtonAnimationTest, DetachedButtonSnapsWithoutResume) {
  TestToggleIconButton button(context(), ic_outlined_24_action_done());
  Task& task = app_.addTaskFullScreen(button);
  ASSERT_TRUE(refresh());
  button.setSelected(true);
  ASSERT_TRUE(refresh());
  delay(40);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(button.selectionAnimationActive());

  task.navigation().clear();
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(button.selectionAnimationActive());
  EXPECT_EQ(Scaled(12), button.getBorderStyle().top_left_corner_radius());
  delay(120);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(Scaled(12), button.getBorderStyle().top_left_corner_radius());
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
  ToggleIconButton round_off(context, ic_outlined_24_action_done());
  ToggleIconButton round_on(context, ic_outlined_24_action_done(), nullptr,
                            IconButtonStyle::kFilled, true);
  ToggleIconButton square_off(context, ic_outlined_24_action_done());
  ToggleIconButton square_on(context, ic_outlined_24_action_done(), nullptr,
                             IconButtonStyle::kFilled, true);
  square_off.setShape(ButtonShape::kSquare);
  square_on.setShape(ButtonShape::kSquare);

  EXPECT_EQ(0xFF, round_off.getBorderStyle().top_left_corner_radius());
  EXPECT_EQ(Scaled(12), round_on.getBorderStyle().top_left_corner_radius());
  EXPECT_EQ(Scaled(12), square_off.getBorderStyle().top_left_corner_radius());
  EXPECT_EQ(0xFF, square_on.getBorderStyle().top_left_corner_radius());
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
