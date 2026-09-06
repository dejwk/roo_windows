#include <type_traits>

#include "gtest/gtest.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/menu/menu_tokens.h"

namespace roo_windows::material3 {
namespace {

static_assert(std::is_base_of<ListItem, MenuItem>::value,
              "menu items reuse the Material 3 list content contract");
static_assert(std::is_base_of<ListEntry, MenuEntry>::value,
              "menu rows reuse the Material 3 list row substrate");
static_assert(sizeof(MenuEntry) <= sizeof(ListEntry) + 24,
              "row adornments and transient binding stay bounded");

TEST(Material3MenuRow, TokenTablesCoverAllVisualFamilies) {
  EXPECT_EQ(112, internal::kBaselineMenuTokens.min_width_dp);
  EXPECT_EQ(280, internal::kExpressiveStandardMenuTokens.max_width_dp);
  EXPECT_EQ(20, internal::kExpressiveVibrantMenuTokens.icon_size_dp);
  EXPECT_EQ(ColorToken::kSurfaceContainer,
            internal::kExpressiveStandardMenuTokens.panel_container);
  EXPECT_EQ(ColorToken::kTertiaryContainer,
            internal::kExpressiveVibrantMenuTokens.panel_container);
  EXPECT_EQ(0, internal::kBaselineMenuTokens.group_gap_dp);
  EXPECT_EQ(2, internal::kExpressiveStandardMenuTokens.group_gap_dp);
  EXPECT_EQ(0, internal::kBaselineMenuTokens.content_padding_dp);
  EXPECT_EQ(4, internal::kExpressiveStandardMenuTokens.content_padding_dp);
  EXPECT_EQ(4, internal::kExpressiveVibrantMenuTokens.content_padding_dp);
}

TEST(Material3MenuRow, StandardItemExposesStableContentAndMutableState) {
  StandardMenuItem item(
      StandardMenuItemInit{"Pump", "Automatic", nullptr, true, true, false});

  EXPECT_EQ("Pump", item.headlineText());
  EXPECT_EQ("Automatic", item.supportingText());
  EXPECT_TRUE(item.isEnabled());
  EXPECT_TRUE(item.isSelectable());
  EXPECT_FALSE(item.isSelected());

  item.setSelectedFromMenu(true);
  item.setEnabled(false);
  EXPECT_TRUE(item.isSelected());
  EXPECT_FALSE(item.isEnabled());
}

TEST(Material3MenuRow, TrailingPayloadAllocatesAndReleasesByUse) {
  StandardMenuItemInit init;
  init.headline = "Inspect";
  StandardMenuItem item(init);
  EXPECT_TRUE(item.trailingAffordances().shortcut.empty());

  item.setShortcut("Ctrl+I");
  item.setBadgeValue(1200);
  item.setTrailingIcon(&ic_filled_24_navigation_chevron_right());
  MenuTrailingAffordances trailing = item.trailingAffordances();
  EXPECT_EQ("Ctrl+I", trailing.shortcut);
  EXPECT_EQ("999+", trailing.badge.text);
  EXPECT_EQ(BadgeMode::kText, trailing.badge.mode);
  EXPECT_NE(nullptr, trailing.icon);

  item.clearShortcut();
  item.clearBadge();
  item.clearTrailingIcon();
  trailing = item.trailingAffordances();
  EXPECT_TRUE(trailing.shortcut.empty());
  EXPECT_EQ(BadgeMode::kHidden, trailing.badge.mode);
  EXPECT_EQ(nullptr, trailing.icon);
}

TEST(Material3MenuRow, BindingReflectsStateAndReservesTrailingLane) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  StandardMenuItemInit plain_init;
  plain_init.headline = "Open";
  StandardMenuItem plain(plain_init);
  StandardMenuItemInit decorated_init;
  decorated_init.headline = "Open";
  decorated_init.selectable = true;
  decorated_init.selected = true;
  StandardMenuItem decorated(decorated_init);
  decorated.setShortcut("Enter");
  decorated.setBadgeDot();

  MenuEntry plain_row(context);
  MenuEntry decorated_row(context);
  plain_row.setMenuItem(plain);
  decorated_row.setMenuItem(decorated);

  EXPECT_EQ(&plain, plain_row.menuItem());
  EXPECT_FALSE(plain_row.isClickable());
  EXPECT_TRUE(decorated_row.visualContext().selected);
  EXPECT_GE(
      decorated_row
          .measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
          .width(),
      plain_row.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
          .width());
}

class DestructionItem : public MenuItem {
 public:
  explicit DestructionItem(bool& destroyed) : destroyed_(destroyed) {}
  ~DestructionItem() override { destroyed_ = true; }
  roo::string_view headlineText() const override { return "Owned"; }

 private:
  bool& destroyed_;
};

TEST(Material3MenuRow, InlineItemIsUnboundBeforeItsDestructorRuns) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  bool destroyed = false;
  {
    MenuRow<DestructionItem> row(context, destroyed);
  }
  EXPECT_TRUE(destroyed);
}

}  // namespace
}  // namespace roo_windows::material3
