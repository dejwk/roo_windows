#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/menu/menu.h"

namespace roo_windows::material3 {
namespace {

class MenuGoldenPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    if (children().empty()) return Dimensions(0, 0);
    return children().front()->measure(width, height);
  }

  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    if (!children().empty()) {
      children().front()->layout(
          Rect(0, 0, rect.width() - 1, rect.height() - 1));
    }
  }
};

class Material3MenuGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 280;
  static constexpr int16_t kHeight = 72;

  Material3MenuGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        environment_(scheduler_),
        app_(&environment_, display_),
        panel_(app_.context()),
        owner_(app_.addTaskFullScreen(panel_)) {}

  roo_display::Offscreen<roo_display::Rgb888> Capture() const {
    return ::roo_windows::test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth,
                                           kHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
  MenuGoldenPanel panel_;
  Task& owner_;
};

class GoldenMenuEntry final : public MenuEntry {
 public:
  using MenuEntry::MenuEntry;
  void Tap() { onSingleTapUp(1, 1); }
};

class GoldenSubmenuItem final : public StandardMenuItem {
 public:
  explicit GoldenSubmenuItem(ApplicationContext& context)
      : StandardMenuItem(StandardMenuItemInit{"Display", {}}),
        context_(context) {}

  bool hasSubmenu() const override { return true; }

  void populateSubmenu(MenuLevelBuilder& builder) override {
    auto group = std::make_unique<MenuGroup>(context_);
    StandardMenuItemInit brightness;
    brightness.headline = "Brightness";
    StandardMenuItemInit contrast;
    contrast.headline = "High contrast";
    group->add(
        std::make_unique<MenuRow<StandardMenuItem>>(context_, brightness));
    group->add(std::make_unique<MenuRow<StandardMenuItem>>(context_, contrast));
    builder.addGroup(std::move(group));
  }

 private:
  ApplicationContext& context_;
};

TEST_F(Material3MenuGoldenTest, SelectedRowWithTrailingAdornments) {
  StandardMenuItemInit init;
  init.headline = "Filtration mode";
  init.supporting = "Automatic";
  init.flags =
      StandardMenuItemFlags::kSelectable | StandardMenuItemFlags::kSelected;
  StandardMenuItem item(init);
  item.setShortcut("Ctrl+F");
  item.setBadgeText("3");
  item.setTrailingIcon(&ic_filled_24_navigation_chevron_right());
  MenuEntry row(app_.context());
  row.setMenuItem(item);
  panel_.add(WidgetRef(row));
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_menu/selected_adornments.ppm",
      "material3_menu_selected_adornments"));
  panel_.removeLast();
}

TEST_F(Material3MenuGoldenTest, ActiveCascadingSubmenu) {
  GoldenSubmenuItem item(app_.context());
  GoldenMenuEntry row(app_.context());
  row.setMenuItem(item);
  MenuGroup group(app_.context());
  group.add(row);
  Menu menu(app_.context());
  menu.addGroup(group);
  ASSERT_EQ(MenuShowResult::kShown,
            menu.showFromRect(owner_, Rect(8, 8, 8, 8)));
  row.Tap();
  ASSERT_TRUE(app_.refresh());

  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_menu/active_submenu.ppm",
      "material3_menu_active_submenu"));
  menu.dismissChain();
  ASSERT_TRUE(app_.refresh());
  ASSERT_TRUE(app_.refresh());
  menu.clearGroups();
  group.clear();
}

}  // namespace
}  // namespace roo_windows::material3
