#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/button/button.h"

namespace roo_windows {
namespace material3 {
namespace {

using roo_display::Color;

roo_display::Color QuantizeToArgb4444(roo_display::Color color) {
  roo_display::Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

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

class Material3ButtonClickAnimationTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 160;
  static constexpr int16_t kHeight = 80;

  Material3ButtonClickAnimationTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  ApplicationContext& context() { return app_.context(); }

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

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

// Verifies that a default button comes up with the intended Material 3 visual
// variant and geometry selectors, while preserving the supplied label.
TEST(Material3Button, DefaultVariantIsFilled) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Save");
  EXPECT_EQ(ButtonVariant::kFilled, b.variant());
  EXPECT_EQ(ButtonSize::kSmall, b.size());
  EXPECT_EQ(ButtonShape::kRound, b.shape());
  EXPECT_EQ(SmallButtonPadding::kReduced, b.smallButtonPadding());
  EXPECT_EQ(std::string("Save"), std::string(b.label()));
  EXPECT_FALSE(b.hasIcon());
}

// Verifies that Material 3 buttons always opt into click handling even before
// a callback is installed, so pressed-state affordances are available.
TEST(Material3Button, IsClickableEvenWithoutCallback) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Tap");
  EXPECT_TRUE(b.isClickable());
}

// Verifies that the default small button resolves to the expected 40 dp
// natural height once content size and default padding are combined.
TEST(Material3Button, ReportsMinimumHeightOfFortyDp) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Hi");
  EXPECT_EQ(Scaled(40), b.getNaturalDimensions().height());
}

// Verifies that the default small-button layout uses the reduced 16 dp side
// padding for both text-only and icon-bearing configurations.
TEST(Material3Button, DefaultHorizontalPaddingIsSixteenDpPerSide) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button label_only(context, "Hi");
  EXPECT_EQ(Scaled(16), label_only.getPadding().left());
  EXPECT_EQ(Scaled(16), label_only.getPadding().right());

  Button with_icon(context, "Hi");
  with_icon.setIcon(&ic_outlined_24_action_done());
  EXPECT_EQ(Scaled(16), with_icon.getPadding().left());
  EXPECT_EQ(Scaled(16), with_icon.getPadding().right());
}

// Verifies that changing the size preset feeds through to the measured natural
// height instead of leaving the button on a single hard-coded geometry bucket.
TEST(Material3Button, SizeControlsNaturalHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Hi");

  b.setSize(ButtonSize::kExtraSmall);
  EXPECT_EQ(Scaled(32), b.getNaturalDimensions().height());

  b.setSize(ButtonSize::kMedium);
  EXPECT_EQ(Scaled(56), b.getNaturalDimensions().height());

  b.setSize(ButtonSize::kLarge);
  EXPECT_EQ(Scaled(96), b.getNaturalDimensions().height());

  b.setSize(ButtonSize::kExtraLarge);
  EXPECT_EQ(Scaled(136), b.getNaturalDimensions().height());
}

// Verifies that the small-button padding selector only affects the small size;
// larger size presets must ignore it and use their own tokenized padding.
TEST(Material3Button, SmallPaddingModeOnlyAffectsSmallButtons) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Hi");

  EXPECT_EQ(Scaled(16), b.getPadding().left());
  b.setSmallButtonPadding(SmallButtonPadding::kDefault);
  EXPECT_EQ(Scaled(24), b.getPadding().left());

  b.setSize(ButtonSize::kMedium);
  EXPECT_EQ(Scaled(24), b.getPadding().left());
  b.setSmallButtonPadding(SmallButtonPadding::kReduced);
  EXPECT_EQ(Scaled(24), b.getPadding().left());
}

// Verifies that square buttons use the size-specific resting corner radii from
// the Material 3 table rather than one fixed radius for all sizes.
TEST(Material3Button, SquareShapeUsesSizeSpecificCornerRadius) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Hi");
  b.setShape(ButtonShape::kSquare);

  b.setSize(ButtonSize::kExtraSmall);
  EXPECT_EQ(Scaled(12), b.getBorderStyle().top_left_corner_radius());

  b.setSize(ButtonSize::kMedium);
  EXPECT_EQ(Scaled(16), b.getBorderStyle().top_left_corner_radius());

  b.setSize(ButtonSize::kLarge);
  EXPECT_EQ(Scaled(28), b.getBorderStyle().top_left_corner_radius());
}

// Verifies that each button variant resolves to the intended container role so
// inherited surface decoration follows the Material 3 semantics.
TEST(Material3Button, ContainerRoleMatchesVariant) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button filled(context, "F", ButtonVariant::kFilled);
  EXPECT_EQ(::roo_windows::material3::ColorToken::kPrimary, filled.containerRole());

  Button tonal(context, "FT", ButtonVariant::kFilledTonal);
  EXPECT_EQ(::roo_windows::material3::ColorToken::kSecondaryContainer, tonal.containerRole());

  Button elevated(context, "E", ButtonVariant::kElevated);
  EXPECT_EQ(::roo_windows::material3::ColorToken::kSurfaceContainerLow, elevated.containerRole());

  Button outlined(context, "O", ButtonVariant::kOutlined);
  EXPECT_EQ(::roo_windows::material3::ColorToken::kNone, outlined.containerRole());

  Button text(context, "T", ButtonVariant::kText);
  EXPECT_EQ(::roo_windows::material3::ColorToken::kNone, text.containerRole());
}

// Verifies that only the outlined variant advertises a non-zero outline width
// to the surface pipeline.
TEST(Material3Button, OutlinedVariantHasOutline) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button outlined(context, "O", ButtonVariant::kOutlined);
  EXPECT_GT((int)outlined.getBorderStyle().outline_width().floor(), 0);

  Button filled(context, "F", ButtonVariant::kFilled);
  EXPECT_EQ(0, (int)filled.getBorderStyle().outline_width().floor());
}

// Verifies that elevated buttons surface a non-zero resting elevation while
// other standard button variants remain flat at rest.
TEST(Material3Button, ElevatedVariantHasNonzeroRestingElevation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button elevated(context, "E", ButtonVariant::kElevated);
  EXPECT_EQ(3, elevated.getElevation());

  Button filled(context, "F", ButtonVariant::kFilled);
  EXPECT_EQ(0, filled.getElevation());
}

// Verifies that disabling an elevated button drops its advertised elevation,
// matching the Material 3 disabled-state treatment.
TEST(Material3Button, DisabledElevatedVariantDropsElevation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button elevated(context, "E", ButtonVariant::kElevated);
  elevated.setEnabled(false);

  EXPECT_EQ(0, elevated.getElevation());
}

// Verifies that adding a leading icon expands the measured width, proving that
// icon slot and gap geometry participate in sizing.
TEST(Material3Button, IconChangesNaturalWidth) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);

  Button b(context, "Send");
  Dimensions text_only =
      b.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  b.setIcon(&ic_outlined_24_action_done());
  Dimensions with_icon =
      b.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_GT(with_icon.width(), text_only.width());
}

// Verifies that square buttons morph their corner radius during the first half
// of the click animation, then settle at the pressed radius.
TEST_F(Material3ButtonClickAnimationTest,
       SquareButtonCornerRadiusSettlesMidwayThroughClickAnimation) {
  auto button = std::make_unique<Button>(context(), "Save");
  Button* button_ptr = button.get();
  button_ptr->setShape(ButtonShape::kSquare);

  app_.add(std::move(button), roo_display::Box(20, 20, 139, 59));
  ASSERT_TRUE(refresh());

  uint8_t resting_radius =
      button_ptr->getBorderStyle().top_left_corner_radius();
  uint8_t pressed_radius = (uint8_t)Scaled(8);
  EXPECT_EQ((uint8_t)Scaled(12), resting_radius);

  button_ptr->onShowPress(button_ptr->width() / 2, button_ptr->height() / 2);

  delay(kPressAnimationMillis / 8);
  uint8_t animated_radius =
      button_ptr->getBorderStyle().top_left_corner_radius();
  EXPECT_LT(animated_radius, resting_radius);
  EXPECT_GT(animated_radius, pressed_radius);

  delay(kPressAnimationMillis / 4 + 20);
  EXPECT_EQ(pressed_radius,
            button_ptr->getBorderStyle().top_left_corner_radius());
}

// Verifies that round buttons keep their pill radius at rest, animate away
// from it early, and settle at the pressed radius by mid-click.
TEST_F(Material3ButtonClickAnimationTest,
       RoundButtonCornerRadiusSettlesMidwayThroughClickAnimation) {
  auto button = std::make_unique<Button>(context(), "Save");
  Button* button_ptr = button.get();

  app_.add(std::move(button), roo_display::Box(20, 20, 139, 59));
  ASSERT_TRUE(refresh());

  uint8_t pressed_radius = (uint8_t)Scaled(8);
  EXPECT_EQ(0xFF, button_ptr->getBorderStyle().top_left_corner_radius());

  button_ptr->onShowPress(button_ptr->width() / 2, button_ptr->height() / 2);

  delay(kPressAnimationMillis / 8);
  uint8_t animated_radius =
      button_ptr->getBorderStyle().top_left_corner_radius();
  EXPECT_LT(animated_radius, (uint8_t)0xFF);
  EXPECT_GT(animated_radius, pressed_radius);

  delay(kPressAnimationMillis / 4 + 20);
  EXPECT_EQ(pressed_radius,
            button_ptr->getBorderStyle().top_left_corner_radius());
}

// Verifies that rounded button contents stay clipped to the rectangular
// surface interior, leaving the decoration-owned corner cutout on the
// backdrop both at rest and during the press shape morph.
TEST_F(Material3ButtonClickAnimationTest,
       RoundButtonContentsDoNotPaintCornerCutouts) {
  constexpr Color kBackdropColor(0xFFE7F6F2);

  auto backdrop = std::make_unique<SolidBackdrop>(context(), kBackdropColor,
                                                  Dimensions(kWidth, kHeight));
  app_.add(std::move(backdrop),
           roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

  auto button = std::make_unique<Button>(context(), "Save");
  Button* button_ptr = button.get();
  app_.add(std::move(button), roo_display::Box(20, 20, 139, 59));

  ASSERT_TRUE(refresh());

  Color expected_backdrop = QuantizeToArgb4444(kBackdropColor);
  EXPECT_EQ(expected_backdrop, pixelAt(21, 21));

  button_ptr->onShowPress(button_ptr->width() / 2, button_ptr->height() / 2);
  delay(kPressAnimationMillis / 4);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(expected_backdrop, pixelAt(21, 21));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
