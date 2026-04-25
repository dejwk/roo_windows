#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "test/golden_image.h"

namespace roo_windows {
namespace {

using roo_display::Color;

class SolidWidget : public BasicWidget {
 public:
  SolidWidget(const Environment& env, Color color, Dimensions dims)
      : BasicWidget(env), color_(color), dims_(dims) {}

  Color background() const override { return color_; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class DecoratedWidget : public BasicWidget {
 public:
  DecoratedWidget(const Environment& env, Color fill_color, Color outline_color,
                  BorderStyle border_style, uint8_t elevation, Dimensions dims)
      : BasicWidget(env),
        fill_color_(fill_color),
        outline_color_(outline_color),
        border_style_(border_style),
        elevation_(elevation),
        dims_(dims) {}

  Color background() const override { return fill_color_; }

  Color getOutlineColor() const override { return outline_color_; }

  BorderStyle getBorderStyle() const override { return border_style_; }

  uint8_t getElevation() const override { return elevation_; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color fill_color_;
  Color outline_color_;
  BorderStyle border_style_;
  uint8_t elevation_;
  Dimensions dims_;
};

class DecorationGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 180;
  static constexpr int16_t kHeight = 128;

  DecorationGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> Render(BorderStyle border_style,
                                                     uint8_t elevation) {
    Application app(&env_, display_);

    auto backdrop = std::make_unique<SolidWidget>(env_, Color(0xFFE8EAEE),
                                                  Dimensions(kWidth, kHeight));
    app.add(WidgetRef(std::move(backdrop)),
            roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

    auto card = std::make_unique<DecoratedWidget>(
        env_, Color(0xFF0F7FBF), Color(0xFFE9A334), border_style, elevation,
        Dimensions(92, 58));
    app.add(WidgetRef(std::move(card)), roo_display::Box(44, 30, 135, 87));

    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 20, 8, 140, 112);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

TEST_F(DecorationGoldenTest, SharpNoOutlineElevation0) {
  auto image = Render(BorderStyle(0, 0), 0);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "lib/roo_windows/test/goldens/decoration/sharp_no_outline_e0.ppm",
      "decoration_sharp_no_outline_e0"));
}

TEST_F(DecorationGoldenTest, RoundUniformNoOutlineElevation0) {
  auto image = Render(BorderStyle(14, 0), 0);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image,
      "lib/roo_windows/test/goldens/decoration/round_uniform_no_outline_e0.ppm",
      "decoration_round_uniform_no_outline_e0"));
}

TEST_F(DecorationGoldenTest, RoundVariableNoOutlineElevation0) {
  auto image = Render(BorderStyle(4, 12, 18, 8, 0), 0);
  EXPECT_TRUE(
      test::CompareOrUpdateGolden(image,
                                  "lib/roo_windows/test/goldens/decoration/"
                                  "round_variable_no_outline_e0.ppm",
                                  "decoration_round_variable_no_outline_e0"));
}

TEST_F(DecorationGoldenTest, SharpOutlineElevation0) {
  auto image = Render(BorderStyle(0, SmallNumber::Of16ths(36)), 0);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "lib/roo_windows/test/goldens/decoration/sharp_outline_e0.ppm",
      "decoration_sharp_outline_e0"));
}

TEST_F(DecorationGoldenTest, RoundUniformOutlineElevation0) {
  auto image = Render(BorderStyle(14, SmallNumber::Of16ths(36)), 0);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image,
      "lib/roo_windows/test/goldens/decoration/round_uniform_outline_e0.ppm",
      "decoration_round_uniform_outline_e0"));
}

TEST_F(DecorationGoldenTest, RoundVariableOutlineElevation0) {
  auto image = Render(BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)), 0);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image,
      "lib/roo_windows/test/goldens/decoration/round_variable_outline_e0.ppm",
      "decoration_round_variable_outline_e0"));
}

TEST_F(DecorationGoldenTest, RoundVariableOutlineElevation4) {
  auto image = Render(BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)), 4);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image,
      "lib/roo_windows/test/goldens/decoration/round_variable_outline_e4.ppm",
      "decoration_round_variable_outline_e4"));
}

TEST_F(DecorationGoldenTest, RoundVariableOutlineElevation12) {
  auto image = Render(BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)), 12);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image,
      "lib/roo_windows/test/goldens/decoration/round_variable_outline_e12.ppm",
      "decoration_round_variable_outline_e12"));
}

}  // namespace
}  // namespace roo_windows
