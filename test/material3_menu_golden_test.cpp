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

TEST_F(Material3MenuGoldenTest, SelectedRowWithTrailingAdornments) {
  StandardMenuItemInit init;
  init.headline = "Filtration mode";
  init.supporting = "Automatic";
  init.selectable = true;
  init.selected = true;
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

}  // namespace
}  // namespace roo_windows::material3
