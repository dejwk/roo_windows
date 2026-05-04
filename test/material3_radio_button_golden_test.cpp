#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/radio_button/radio_button.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;

class SolidBackdrop : public BasicSurfaceWidget {
 public:
  SolidBackdrop(const Environment& env, Color color, Dimensions dims)
      : BasicSurfaceWidget(env), color_(color), dims_(dims) {}

  Color background() const override { return color_; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class Material3RadioButtonGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 88;
  static constexpr int16_t kHeight = 44;
  static constexpr int16_t kButtonSize = Scaled(20);
  static constexpr int16_t kRowY = 12;
  static constexpr int16_t kX0 = 16;
  static constexpr int16_t kGap = 16;
  static constexpr int16_t kStride = kButtonSize + kGap;

  Material3RadioButtonGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderRow(bool enabled) {
    Application app(&env_, display_);

    auto backdrop = std::make_unique<SolidBackdrop>(
        env_, Color(0xFFF3EFE7), Dimensions(kWidth, kHeight));
    app.add(std::move(backdrop),
            roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

    AddRadioButton(app, kX0 + 0 * kStride, false, enabled);
    AddRadioButton(app, kX0 + 1 * kStride, true, enabled);

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, kHeight);
  }

  void AddRadioButton(Application& app, int16_t x, bool on, bool enabled) {
    auto radio = std::make_unique<RadioButton>(env_, on);
    radio->setEnabled(enabled);
    app.add(std::move(radio), roo_display::Box(x, kRowY, x + kButtonSize - 1,
                                               kRowY + kButtonSize - 1));
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

TEST_F(Material3RadioButtonGoldenTest, EnabledStatesRow) {
  auto image = RenderRow(true);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_radio_button/enabled_states_row.ppm",
      "material3_radio_button_enabled_states_row"));
}

TEST_F(Material3RadioButtonGoldenTest, DisabledStatesRow) {
  auto image = RenderRow(false);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/material3_radio_button/disabled_states_row.ppm",
      "material3_radio_button_disabled_states_row"));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows