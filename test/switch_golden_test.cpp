#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/widgets/switch.h"

namespace roo_windows {
namespace {

class SwitchBackdrop : public Panel {
 public:
  SwitchBackdrop(ApplicationContext& context, Color color,
                 Dimensions dimensions)
      : Panel(context), color_(color), dimensions_(dimensions) {}

  using Panel::add;

  Color background() const override { return color_; }
  void paint(PaintContext& context) const override { context.clear(); }
  Dimensions getSuggestedMinimumDimensions() const override {
    return dimensions_;
  }

 private:
  Color color_;
  Dimensions dimensions_;
};

class SwitchGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 220;
  static constexpr int16_t kHeight = 52;

  SwitchGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> renderRow(bool animate_on) {
    Application app(&env_, display_);
    auto backdrop = std::make_unique<SwitchBackdrop>(
        app.context(), Color(0xFFF3EFE7), Dimensions(kWidth, kHeight));

    auto material_off = std::make_unique<material3::Switch>(app.context());
    auto material_on = std::make_unique<material3::Switch>(app.context());
    material3::Switch* material_on_ptr = material_on.get();
    auto legacy_off = std::make_unique<Switch>(app.context());
    auto legacy_on = std::make_unique<Switch>(app.context());
    Switch* legacy_on_ptr = legacy_on.get();

    if (!animate_on) {
      material_on->setOn();
      legacy_on->setOn();
    }
    backdrop->add(std::move(material_off),
                  Rect(8, 10, 8 + Scaled(52) - 1, 10 + Scaled(32) - 1));
    backdrop->add(std::move(material_on),
                  Rect(68, 10, 68 + Scaled(52) - 1, 10 + Scaled(32) - 1));
    backdrop->add(std::move(legacy_off),
                  Rect(128, 14, 128 + Scaled(42) - 1, 14 + Scaled(24) - 1));
    backdrop->add(std::move(legacy_on),
                  Rect(178, 14, 178 + Scaled(42) - 1, 14 + Scaled(24) - 1));
    app.add(std::move(backdrop),
            roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
    EXPECT_TRUE(app.refresh());

    if (animate_on) {
      material_on_ptr->onClicked();
      legacy_on_ptr->onClicked();
      EXPECT_TRUE(app.refresh());
      delay(130);
      EXPECT_TRUE(app.refresh());
    }
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, kHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Locks down Material and legacy off/on resting geometry, tokens, and shadow.
TEST_F(SwitchGoldenTest, RestingStates) {
  auto image = renderRow(false);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/switch/resting_states.ppm",
      "switch_resting_states"));
}

// Verifies animated thumb travel restores the same final pixels, including
// the legacy shadow region vacated by the moving thumb.
TEST_F(SwitchGoldenTest, AnimatedFinalMatchesRestingStates) {
  auto image = renderRow(true);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/switch/resting_states.ppm",
      "switch_animated_final"));
}

}  // namespace
}  // namespace roo_windows
