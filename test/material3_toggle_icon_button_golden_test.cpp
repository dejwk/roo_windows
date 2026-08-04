#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/48/action.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/button/toggle_icon_button.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;

class SolidBackdrop : public BasicSurfaceWidget {
 public:
  SolidBackdrop(ApplicationContext& context, Color color, Dimensions dims)
      : BasicSurfaceWidget(context), color_(color), dims_(dims) {}

  Color background() const override { return color_; }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class Material3ToggleIconButtonGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 264;
  static constexpr int16_t kHeight = 216;
  static constexpr Color kBackdropColor = Color(0xFFF3EFE7);

  Material3ToggleIconButtonGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  void AddBackdrop(Application& app) {
    app.add(std::make_unique<SolidBackdrop>(app.context(), kBackdropColor,
                                            Dimensions(kWidth, kHeight)),
            roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  }

  ToggleIconButton* AddButton(Application& app, int16_t x, int16_t y,
                              IconButtonStyle style, bool selected,
                              ButtonSize size = ButtonSize::kSmall,
                              ButtonShape shape = ButtonShape::kRound,
                              bool enabled = true,
                              const MonoIcon* selected_icon = nullptr) {
    auto button = std::make_unique<ToggleIconButton>(
        app.context(), ic_outlined_24_action_bookmark(), selected_icon, style,
        selected);
    button->setSize(size);
    button->setShape(shape);
    button->setEnabled(enabled);
    ToggleIconButton* result = button.get();
    Dimensions dimensions = result->getNaturalDimensions();
    app.add(std::move(button),
            roo_display::Box(x, y, x + dimensions.width() - 1,
                             y + dimensions.height() - 1));
    return result;
  }

  roo_display::Offscreen<roo_display::Rgb888> Capture() const {
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, kHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Verifies selected and unselected treatments for every Material 3 style.
TEST_F(Material3ToggleIconButtonGoldenTest, SelectedAndUnselectedStyles) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kStandard, false);
  AddButton(app, 68, 12, IconButtonStyle::kFilled, false);
  AddButton(app, 124, 12, IconButtonStyle::kFilledTonal, false);
  AddButton(app, 180, 12, IconButtonStyle::kOutlined, false);
  AddButton(app, 12, 84, IconButtonStyle::kStandard, true);
  AddButton(app, 68, 84, IconButtonStyle::kFilled, true);
  AddButton(app, 124, 84, IconButtonStyle::kFilledTonal, true);
  AddButton(app, 180, 84, IconButtonStyle::kOutlined, true);

  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_toggle_icon_button/styles.ppm",
      "material3_toggle_icon_button_styles"));
}

// Verifies same-icon recoloring, alternate icons, and both selected shapes.
TEST_F(Material3ToggleIconButtonGoldenTest, IconsAndSelectedShapes) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kStandard, false,
            ButtonSize::kMedium);
  AddButton(app, 84, 12, IconButtonStyle::kStandard, true, ButtonSize::kMedium);
  AddButton(app, 156, 12, IconButtonStyle::kFilled, true, ButtonSize::kMedium,
            ButtonShape::kRound, true, &ic_outlined_48_action_favorite());
  AddButton(app, 12, 96, IconButtonStyle::kFilledTonal, true,
            ButtonSize::kMedium, ButtonShape::kSquare);
  AddButton(app, 100, 96, IconButtonStyle::kOutlined, true, ButtonSize::kMedium,
            ButtonShape::kRound);

  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(),
      "test/goldens/material3_toggle_icon_button/icons_and_shapes.ppm",
      "material3_toggle_icon_button_icons_and_shapes"));
}

// Verifies disabled treatment and that selected outlined buttons remove border.
TEST_F(Material3ToggleIconButtonGoldenTest, DisabledAndOutlinedSelection) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kStandard, false, ButtonSize::kMedium,
            ButtonShape::kSquare, false);
  AddButton(app, 84, 12, IconButtonStyle::kFilled, true, ButtonSize::kMedium,
            ButtonShape::kSquare, false);
  AddButton(app, 156, 12, IconButtonStyle::kOutlined, false,
            ButtonSize::kMedium, ButtonShape::kSquare);
  AddButton(app, 12, 96, IconButtonStyle::kOutlined, true, ButtonSize::kMedium,
            ButtonShape::kSquare);
  AddButton(app, 84, 96, IconButtonStyle::kOutlined, true, ButtonSize::kMedium,
            ButtonShape::kSquare, false);

  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(),
      "test/goldens/material3_toggle_icon_button/disabled_and_outlined.ppm",
      "material3_toggle_icon_button_disabled_and_outlined"));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
