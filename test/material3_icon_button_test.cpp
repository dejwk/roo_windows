#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/button/icon_button.h"

namespace roo_windows {
namespace material3 {
namespace {

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

roo_display::Color QuantizeToArgb4444(roo_display::Color color) {
  roo_display::Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

class SolidBackdrop : public BasicSurfaceWidget {
 public:
  SolidBackdrop(ApplicationContext& context, roo_display::Color color,
                Dimensions dimensions)
      : BasicSurfaceWidget(context), color_(color), dimensions_(dimensions) {}

  roo_display::Color background() const override { return color_; }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
  Dimensions getSuggestedMinimumDimensions() const override {
    return dimensions_;
  }

 private:
  roo_display::Color color_;
  Dimensions dimensions_;
};

class Material3IconButtonRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 160;
  static constexpr int16_t kHeight = 80;

  Material3IconButtonRenderTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  ApplicationContext& context() { return app_.context(); }

  bool refresh() { return app_.refresh(roo_time::Uptime::Max()); }

  roo_display::Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    roo_display::Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

// Verifies that the constructor selects the documented Material 3 defaults.
TEST(Material3IconButton, DefaultsAreFilledSmallRoundAndUniform) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  IconButton button(context, ic_outlined_24_action_done());

  EXPECT_EQ(IconButtonStyle::kFilled, button.style());
  EXPECT_EQ(ButtonSize::kSmall, button.size());
  EXPECT_EQ(ButtonShape::kRound, button.shape());
  EXPECT_EQ(IconButtonWidth::kUniform, button.widthMode());
  EXPECT_EQ(&ic_outlined_24_action_done(), &button.icon());
  EXPECT_EQ(Margins(0), button.getMargins());
}

// Verifies that size and width tokens determine the visible container bounds.
TEST(Material3IconButton, SizeAndWidthResolveContainerDimensions) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  IconButton button(context, ic_outlined_24_action_done());

  EXPECT_EQ(Scaled(40), button.getNaturalDimensions().width());
  EXPECT_EQ(Scaled(40), button.getNaturalDimensions().height());

  button.setWidthMode(IconButtonWidth::kNarrow);
  EXPECT_EQ(Scaled(32), button.getNaturalDimensions().width());
  button.setWidthMode(IconButtonWidth::kWide);
  EXPECT_EQ(Scaled(48), button.getNaturalDimensions().width());

  button.setSize(ButtonSize::kExtraLarge);
  EXPECT_EQ(Scaled(144), button.getNaturalDimensions().width());
  EXPECT_EQ(Scaled(136), button.getNaturalDimensions().height());
}

// Verifies that each style exposes its intended surface and outline semantics.
TEST(Material3IconButton, StylesExposeExpectedSurfaceSemantics) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  IconButton standard(context, ic_outlined_24_action_done(),
                      IconButtonStyle::kStandard);
  IconButton filled(context, ic_outlined_24_action_done(),
                    IconButtonStyle::kFilled);
  IconButton tonal(context, ic_outlined_24_action_done(),
                   IconButtonStyle::kFilledTonal);
  IconButton outlined(context, ic_outlined_24_action_done(),
                      IconButtonStyle::kOutlined);

  EXPECT_EQ(ColorToken::kSurfaceVariant, standard.containerRole());
  EXPECT_EQ(ColorToken::kPrimary, filled.containerRole());
  EXPECT_EQ(ColorToken::kSecondaryContainer, tonal.containerRole());
  EXPECT_EQ(ColorToken::kSurfaceVariant, outlined.containerRole());
  EXPECT_GT((int)outlined.getBorderStyle().outline_width().floor(), 0);
  EXPECT_EQ(0, (int)filled.getBorderStyle().outline_width().floor());
}

// Verifies that square buttons use the configured size's corner token.
TEST(Material3IconButton, SquareShapeUsesSizeSpecificCornerRadius) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  IconButton button(context, ic_outlined_24_action_done());
  button.setShape(ButtonShape::kSquare);

  EXPECT_EQ(Scaled(12), button.getBorderStyle().top_left_corner_radius());
  button.setSize(ButtonSize::kMedium);
  EXPECT_EQ(Scaled(16), button.getBorderStyle().top_left_corner_radius());
  button.setSize(ButtonSize::kLarge);
  EXPECT_EQ(Scaled(28), button.getBorderStyle().top_left_corner_radius());
}

// Verifies that badge hosts can reuse centered icon-slot geometry directly.
TEST(Material3IconButton, IconBoundsStayCenteredInTheVisualContainer) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  IconButton button(context, ic_outlined_24_action_done());
  button.layout(Rect(0, 0, Scaled(48) - 1, Scaled(40) - 1));

  Rect icon_bounds = button.getIconBounds();
  EXPECT_EQ((button.width() - icon_bounds.width()) / 2, icon_bounds.xMin());
  EXPECT_EQ((button.height() - icon_bounds.height()) / 2, icon_bounds.yMin());
  EXPECT_EQ(Scaled(24), icon_bounds.width());
  EXPECT_EQ(Scaled(24), icon_bounds.height());
}

// Verifies that a filled button settles padding pixels before excluding them.
TEST_F(Material3IconButtonRenderTest, FilledButtonPaintsTheEntireContainer) {
  constexpr roo_display::Color kBackdropColor(0xFFE7F6F2);
  app_.add(std::make_unique<SolidBackdrop>(context(), kBackdropColor,
                                           Dimensions(kWidth, kHeight)),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

  auto button = std::make_unique<IconButton>(
      context(), ic_outlined_24_action_done(), IconButtonStyle::kFilled);
  IconButton* button_ptr = button.get();
  button_ptr->setShape(ButtonShape::kSquare);
  app_.add(std::move(button), roo_display::Box(20, 20, 59, 59));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(button_ptr->background()), pixelAt(21, 40));
}

// Verifies that callback-free buttons remain activatable and storage stays
// compact.
TEST(Material3IconButton, IsClickableAndFitsItsStorageBudget) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  IconButton button(context, ic_outlined_24_action_done());

  EXPECT_TRUE(button.isClickable());
  constexpr size_t kRawBudget = sizeof(BasicSurfaceWidget) + sizeof(void*) + 4;
  constexpr size_t kAlignmentSlack = alignof(IconButton) - 1;
  EXPECT_LE(sizeof(IconButton), kRawBudget + kAlignmentSlack);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
