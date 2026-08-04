#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/button/icon_button.h"

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

class Material3IconButtonGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 264;
  static constexpr int16_t kHeight = 216;
  static constexpr Color kBackdropColor = Color(0xFFF3EFE7);

  Material3IconButtonGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  void AddBackdrop(Application& app) {
    app.add(std::make_unique<SolidBackdrop>(app.context(), kBackdropColor,
                                            Dimensions(kWidth, kHeight)),
            roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  }

  IconButton* AddButton(Application& app, int16_t x, int16_t y,
                        IconButtonStyle style = IconButtonStyle::kFilled,
                        ButtonSize size = ButtonSize::kSmall,
                        ButtonShape shape = ButtonShape::kRound,
                        IconButtonWidth width = IconButtonWidth::kUniform,
                        bool enabled = true) {
    auto button = std::make_unique<IconButton>(
        app.context(), ic_outlined_24_action_done(), style);
    button->setSize(size);
    button->setShape(shape);
    button->setWidthMode(width);
    button->setEnabled(enabled);
    IconButton* result = button.get();
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

// Verifies all four icon-button color styles against a locked-down rendering.
TEST_F(Material3IconButtonGoldenTest, Styles) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kStandard);
  AddButton(app, 68, 12, IconButtonStyle::kFilled);
  AddButton(app, 124, 12, IconButtonStyle::kFilledTonal);
  AddButton(app, 180, 12, IconButtonStyle::kOutlined);

  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_icon_button/styles.ppm",
      "material3_icon_button_styles"));
}

// Verifies all five expressive size tokens against a locked-down rendering.
TEST_F(Material3IconButtonGoldenTest, Sizes) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kFilled, ButtonSize::kExtraSmall);
  AddButton(app, 56, 12, IconButtonStyle::kFilled, ButtonSize::kSmall);
  AddButton(app, 108, 12, IconButtonStyle::kFilled, ButtonSize::kMedium);
  AddButton(app, 12, 80, IconButtonStyle::kFilled, ButtonSize::kLarge);
  AddButton(app, 120, 68, IconButtonStyle::kFilled, ButtonSize::kExtraLarge);

  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_icon_button/sizes.ppm",
      "material3_icon_button_sizes"));
}

// Verifies round/square shape families and narrow/uniform/wide width tokens.
TEST_F(Material3IconButtonGoldenTest, ShapesAndWidths) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kFilled, ButtonSize::kLarge,
            ButtonShape::kRound);
  AddButton(app, 124, 12, IconButtonStyle::kFilled, ButtonSize::kLarge,
            ButtonShape::kSquare);
  AddButton(app, 12, 124, IconButtonStyle::kOutlined, ButtonSize::kMedium,
            ButtonShape::kRound, IconButtonWidth::kNarrow);
  AddButton(app, 76, 124, IconButtonStyle::kOutlined, ButtonSize::kMedium,
            ButtonShape::kRound, IconButtonWidth::kUniform);
  AddButton(app, 148, 124, IconButtonStyle::kOutlined, ButtonSize::kMedium,
            ButtonShape::kRound, IconButtonWidth::kWide);

  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_icon_button/shapes_and_widths.ppm",
      "material3_icon_button_shapes_and_widths"));
}

// Verifies disabled color treatment and pressed-state shape/overlay feedback.
TEST_F(Material3IconButtonGoldenTest, DisabledAndPressedStates) {
  Application app(&env_, display_);
  AddBackdrop(app);
  AddButton(app, 12, 12, IconButtonStyle::kStandard, ButtonSize::kMedium,
            ButtonShape::kSquare, IconButtonWidth::kUniform, false);
  AddButton(app, 84, 12, IconButtonStyle::kFilled, ButtonSize::kMedium,
            ButtonShape::kSquare, IconButtonWidth::kUniform, false);
  IconButton* pressed = AddButton(app, 156, 12, IconButtonStyle::kFilledTonal,
                                  ButtonSize::kMedium, ButtonShape::kSquare);

  ASSERT_TRUE(app.refresh());
  pressed->onShowPress(pressed->width() / 2, pressed->height() / 2);
  ASSERT_TRUE(app.refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      Capture(), "test/goldens/material3_icon_button/disabled_and_pressed.ppm",
      "material3_icon_button_disabled_and_pressed"));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
