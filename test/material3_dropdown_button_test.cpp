#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/filled/navigation.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/utilities/dropdown_button.h"

namespace roo_windows::material3 {
namespace {

class TestPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class DropdownButtonTest : public testing::Test {
 protected:
  DropdownButtonTest()
      : device_(320, 240, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        content_(app_.context()),
        owner_(app_.addTaskFullScreen(content_)) {}

  void attach(DropdownButton& button) {
    content_.add(WidgetRef(button), Rect(20, 20, 219, 67));
    app_.refresh();
  }

  void drain() {
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           4);
    app_.refresh();
  }

  void invokeNextMenuRow() {
    Widget* focused = owner_.focus().focused();
    ASSERT_NE(nullptr, focused);
    ASSERT_TRUE(focused->onKeyEvent(
        KeyEvent{KeyPhase::kDown, KeyCode::kDown, 0, PhysicalKey::kNone, 0}));
    focused = owner_.focus().focused();
    ASSERT_NE(nullptr, focused);
    focused->onSingleTapUp(1, 1);
    app_.refresh();
    app_.refresh();
  }

  roo::byte raster_[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment environment_;
  Application app_;
  TestPanel content_;
  Task& owner_;
};

// Verifies constant arrays start at zero and keep the widest-label width.
TEST_F(DropdownButtonTest, StableNaturalWidthAndSilentSelection) {
  static constexpr const char* choices[] = {"A", "Automatic", "Off"};
  DropdownButton button(app_.context(), choices);
  EXPECT_EQ(3u, button.itemCount());
  EXPECT_EQ(0u, button.selectedIndex());
  int width = button.getNaturalDimensions().width();
  int changes = 0;
  button.setOnInteractiveChange([&] { ++changes; });
  EXPECT_TRUE(button.setSelectedIndex(1));
  EXPECT_EQ("Automatic", button.selectedText());
  EXPECT_EQ(width, button.getNaturalDimensions().width());
  EXPECT_TRUE(button.setSelectedIndex(2));
  EXPECT_EQ(width, button.getNaturalDimensions().width());
  EXPECT_EQ(0, changes);
  EXPECT_FALSE(button.setSelectedIndex(3));
  EXPECT_EQ(2u, button.selectedIndex());
}

// Verifies each button size reserves its own token slot around the single
// artwork variant selected for the configured display zoom.
TEST_F(DropdownButtonTest, ChevronUsesScaledArtworkAndSizeTokenSlot) {
  DropdownButton button(app_.context(), nullptr, 0);
  const MonoIcon& chevron = SCALED_ROO_ICON(filled, navigation_expand_more);
  for (ButtonSize size :
       {ButtonSize::kExtraSmall, ButtonSize::kSmall, ButtonSize::kMedium,
        ButtonSize::kLarge, ButtonSize::kExtraLarge}) {
    button.setSize(size);
    const internal::ButtonGeometryTokens& tokens =
        internal::ButtonGeometryTokensFor(size);
    EXPECT_EQ(std::max<int>(Scaled(tokens.icon_size_dp),
                            chevron.anchorExtents().width()),
              button.getSuggestedMinimumDimensions().width());
  }
}

// Verifies replacing choices releases old borrows and recalculates width.
TEST_F(DropdownButtonTest, ReplacesBorrowedChoicesAndRejectsInvalidInput) {
  static constexpr const char* first[] = {"First", "Second"};
  static constexpr const char* longer[] = {"One", "Very long option", "One"};
  DropdownButton button(app_.context(), first);
  int original_width = button.getNaturalDimensions().width();
  EXPECT_TRUE(button.setItems(longer, 2));
  EXPECT_EQ(3u, button.itemCount());
  EXPECT_EQ(2u, button.selectedIndex());
  EXPECT_GT(button.getNaturalDimensions().width(), original_width);
  EXPECT_FALSE(button.setItems(longer, 3, 3));
  EXPECT_EQ(2u, button.selectedIndex());
  EXPECT_FALSE(button.setItems(nullptr, 2, 0));
  EXPECT_TRUE(button.setItems(nullptr, 0, DropdownButton::kNoSelection));
  EXPECT_EQ(DropdownButton::kNoSelection, button.selectedIndex());
  EXPECT_FALSE(button.isClickable());
  EXPECT_EQ(MenuShowResult::kInteractionOwnerUnavailable, button.showMenu());
}

// Verifies menu invocation commits only after detach and notifies once.
TEST_F(DropdownButtonTest, MenuCommitsOneSelectionAndCanReopen) {
  static constexpr const char* choices[] = {"First", "Second", "Third"};
  DropdownButton button(app_.context(), choices);
  attach(button);
  int changes = 0;
  button.setOnInteractiveChange([&] {
    EXPECT_FALSE(button.isMenuOpen());
    EXPECT_EQ(1u, button.selectedIndex());
    ++changes;
  });
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  ASSERT_TRUE(button.isMenuOpen());
  invokeNextMenuRow();
  drain();
  EXPECT_EQ(1u, button.selectedIndex());
  EXPECT_EQ(1, changes);
  EXPECT_EQ(MenuShowResult::kShown, button.showMenu());
  button.dismissMenu();
  drain();
  EXPECT_EQ(1, changes);
  content_.removeLast();
}

// Verifies a table swap cancels a live menu before old strings can be freed.
TEST_F(DropdownButtonTest, ReplacingWhileOpenCancelsOldRows) {
  auto first = std::make_unique<std::string>("Temporary");
  const char* borrowed[] = {first->c_str(), "Static"};
  static constexpr const char* replacement[] = {"Only"};
  DropdownButton button(app_.context(), borrowed);
  attach(button);
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  ASSERT_TRUE(button.setItems(replacement));
  first.reset();
  drain();
  EXPECT_FALSE(button.isMenuOpen());
  EXPECT_EQ("Only", button.selectedText());
  EXPECT_EQ(MenuShowResult::kShown, button.showMenu());
  button.dismissMenu();
  content_.removeLast();
}

// Verifies an explicit replacement cancels a selected row before delivery.
TEST_F(DropdownButtonTest, ReplacementCancelsPendingSelection) {
  static constexpr const char* choices[] = {"First", "Second"};
  static constexpr const char* replacement[] = {"New"};
  DropdownButton button(app_.context(), choices);
  attach(button);
  int changes = 0;
  button.setOnInteractiveChange([&] { ++changes; });
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  invokeNextMenuRow();
  EXPECT_EQ(0u, button.selectedIndex());
  ASSERT_TRUE(button.setItems(replacement));
  drain();
  EXPECT_EQ(0, changes);
  EXPECT_EQ("New", button.selectedText());
  content_.removeLast();
}

// Verifies terminal cleanup precedes a handler that opens the next session.
TEST_F(DropdownButtonTest, SelectionHandlerCanReopen) {
  static constexpr const char* choices[] = {"First", "Second"};
  DropdownButton button(app_.context(), choices);
  attach(button);
  MenuShowResult reopened = MenuShowResult::kAlreadyPresented;
  button.setOnInteractiveChange([&] { reopened = button.showMenu(); });
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  invokeNextMenuRow();
  drain();
  EXPECT_EQ(MenuShowResult::kShown, reopened);
  EXPECT_TRUE(button.isMenuOpen());
  button.dismissMenu();
  content_.removeLast();
}

// Verifies callback dispatch survives removal of its active registration.
TEST_F(DropdownButtonTest, SelectionHandlerCanDestroyButton) {
  static constexpr const char* choices[] = {"First", "Second"};
  auto button = std::make_unique<DropdownButton>(app_.context(), choices);
  DropdownButton* source = button.get();
  content_.add(WidgetRef(std::move(button)), Rect(20, 20, 219, 67));
  app_.refresh();
  int changes = 0;
  source->setOnInteractiveChange([&] {
    ++changes;
    content_.removeLast();
  });
  ASSERT_EQ(MenuShowResult::kShown, source->showMenu());
  invokeNextMenuRow();
  drain();
  EXPECT_EQ(1, changes);
  EXPECT_EQ(nullptr, owner_.focus().focused());
}

// Verifies inherited and explicit density affect trigger geometry independently
// of selection, while invalidating an open session for the next policy.
TEST_F(DropdownButtonTest, DensityAndAppearanceSettersCloseMenu) {
  static constexpr const char* choices[] = {"Automatic", "Off"};
  DropdownButton button(app_.context(), choices);
  attach(button);
  int standard_height = button.getNaturalDimensions().height();
  button.setDensityOverride(DensityOverride::Explicit(Density::kMinus5));
  EXPECT_LE(button.getNaturalDimensions().height(), standard_height);
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  button.setSize(ButtonSize::kMedium);
  EXPECT_FALSE(button.isMenuOpen());
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  button.setShape(ButtonShape::kSquare);
  EXPECT_FALSE(button.isMenuOpen());
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  button.setVariant(ButtonVariant::kElevated);
  EXPECT_FALSE(button.isMenuOpen());
  content_.removeLast();
}

}  // namespace
}  // namespace roo_windows::material3
