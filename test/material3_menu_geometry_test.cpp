#include <memory>
#include <type_traits>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/menu/menu_geometry.h"
#include "roo_windows/material3/menu/menu_surface.h"

namespace roo_windows::material3 {
namespace {

TEST(Material3MenuGeometry, RootPreferencesFitBeforeClamping) {
  const Rect viewport(8, 8, 311, 231);
  const Rect anchor(40, 40, 79, 63);
  const Dimensions desired(112, 96);

  auto below = internal::ResolveRootMenuPlacement(
      viewport, anchor, desired, MenuPlacement::kBelowStart,
      LayoutDirection::kLeftToRight);
  EXPECT_EQ(Rect(40, 64, 151, 159), below.bounds);
  EXPECT_FALSE(below.scrolls);

  auto above = internal::ResolveRootMenuPlacement(
      viewport, anchor, desired, MenuPlacement::kAboveStart,
      LayoutDirection::kLeftToRight);
  EXPECT_EQ(Rect(40, 64, 151, 159), above.bounds);
}

TEST(Material3MenuGeometry, StartAlignmentMirrorsInRtl) {
  const Rect viewport(0, 0, 319, 239);
  const Rect anchor(160, 20, 199, 43);
  const Dimensions desired(112, 48);
  auto ltr = internal::ResolveRootMenuPlacement(viewport, anchor, desired,
                                                MenuPlacement::kBelowStart,
                                                LayoutDirection::kLeftToRight);
  auto rtl = internal::ResolveRootMenuPlacement(viewport, anchor, desired,
                                                MenuPlacement::kBelowStart,
                                                LayoutDirection::kRightToLeft);
  EXPECT_EQ(160, ltr.bounds.xMin());
  EXPECT_EQ(88, rtl.bounds.xMin());
}

TEST(Material3MenuGeometry, OversizeContentConstrainsBeforeClamp) {
  const Rect viewport(8, 8, 151, 111);
  auto result = internal::ResolveRootMenuPlacement(
      viewport, Rect(140, 100, 143, 103), Dimensions(280, 400),
      MenuPlacement::kBelowEnd, LayoutDirection::kLeftToRight);
  EXPECT_EQ(viewport, result.bounds);
  EXPECT_TRUE(result.scrolls);
}

TEST(Material3MenuGeometry, SubmenuFallsBackInPlaceWhenNeitherSideFits) {
  const Rect viewport(0, 0, 159, 239);
  const Rect opener(24, 40, 135, 87);
  const Rect parent(20, 20, 139, 180);
  auto result = internal::ResolveSubmenuPlacement(
      viewport, opener, parent, Dimensions(112, 100), 112, 4,
      LayoutDirection::kLeftToRight);
  EXPECT_FALSE(result.cascading);
  EXPECT_EQ(parent, result.bounds);
}

TEST(Material3MenuGeometry, SubmenuUsesMirroredAfterSideInRtl) {
  const Rect viewport(0, 0, 399, 239);
  const Rect opener(200, 40, 311, 87);
  auto result = internal::ResolveSubmenuPlacement(
      viewport, opener, Rect(196, 20, 315, 180), Dimensions(112, 100), 112, 4,
      LayoutDirection::kRightToLeft);
  EXPECT_TRUE(result.cascading);
  EXPECT_LT(result.bounds.xMax(), opener.xMin());
}

class TrackingEntry final : public MenuEntry {
 public:
  TrackingEntry(ApplicationContext& context, bool& destroyed)
      : MenuEntry(context), destroyed_(destroyed) {}
  ~TrackingEntry() override { destroyed_ = true; }

 private:
  bool& destroyed_;
};

TEST(Material3MenuGeometry, GroupsDetachBorrowedAndDeleteAdoptedRows) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  bool adopted_destroyed = false;
  MenuEntry borrowed(context);
  {
    MenuGroup group(context);
    group.add(borrowed);
    group.add(std::make_unique<TrackingEntry>(context, adopted_destroyed));
    EXPECT_EQ(&group, borrowed.parent());
    group.clear();
    EXPECT_EQ(nullptr, borrowed.parent());
    EXPECT_TRUE(adopted_destroyed);
  }
}

TEST(Material3MenuGeometry, PanelCoercesHeightToPersistentScrolling) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  internal::MenuPanel panel(context);
  auto group = std::make_unique<MenuGroup>(context);
  for (int i = 0; i < 6; ++i) {
    auto row = std::make_unique<MenuRow<StandardMenuItem>>(context);
    group->add(std::move(row));
  }
  panel.addGroup(std::move(group));
  Dimensions size =
      panel.measure(WidthSpec::AtMost(200), HeightSpec::AtMost(100));
  EXPECT_EQ(100, size.height());
  EXPECT_TRUE(panel.isScrolling());
  panel.clearGroups();
}

TEST(Material3MenuGeometry, OverlayRemainsTransparent) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  internal::MenuOverlay overlay(context);
  EXPECT_EQ(roo_display::color::Transparent, overlay.background());
  EXPECT_FALSE(overlay.fullyCoversBoundsWithOpaqueColors());
  EXPECT_LE(sizeof(internal::MenuOverlay), sizeof(Container) + 32U);
}

static_assert(!std::is_copy_constructible<MenuLevelBuilder>::value,
              "submenu builders are scoped and non-copyable");
static_assert(!std::is_move_constructible<MenuLevelBuilder>::value,
              "submenu builders cannot escape population by moving");

}  // namespace
}  // namespace roo_windows::material3
