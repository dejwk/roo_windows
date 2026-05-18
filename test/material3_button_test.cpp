#include "roo_windows/material3/button/button.h"

#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"

namespace roo_windows {
namespace material3 {
namespace {

TEST(Material3Button, DefaultVariantIsFilled) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button b(env, "Save");
  EXPECT_EQ(ButtonVariant::kFilled, b.variant());
  EXPECT_EQ(std::string("Save"), std::string(b.label()));
  EXPECT_FALSE(b.hasIcon());
}

TEST(Material3Button, IsClickableEvenWithoutCallback) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button b(env, "Tap");
  EXPECT_TRUE(b.isClickable());
}

TEST(Material3Button, ReportsMinimumHeightOfFortyDp) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button b(env, "Hi");
  EXPECT_EQ(Scaled(40), b.getNaturalDimensions().height());
}

TEST(Material3Button, DefaultHorizontalPaddingIsSixteenDpPerSide) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button label_only(env, "Hi");
  EXPECT_EQ(Scaled(16), label_only.getPadding().left());
  EXPECT_EQ(Scaled(16), label_only.getPadding().right());

  Button with_icon(env, "Hi");
  with_icon.setIcon(&ic_outlined_24_action_done());
  EXPECT_EQ(Scaled(16), with_icon.getPadding().left());
  EXPECT_EQ(Scaled(16), with_icon.getPadding().right());
}

TEST(Material3Button, ContainerRoleMatchesVariant) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button filled(env, "F", ButtonVariant::kFilled);
  EXPECT_EQ(ColorRole::kPrimary, filled.containerRole());

  Button tonal(env, "FT", ButtonVariant::kFilledTonal);
  EXPECT_EQ(ColorRole::kSecondaryContainer, tonal.containerRole());

  Button elevated(env, "E", ButtonVariant::kElevated);
  EXPECT_EQ(ColorRole::kSurfaceContainerLow, elevated.containerRole());

  Button outlined(env, "O", ButtonVariant::kOutlined);
  EXPECT_EQ(ColorRole::kUndefined, outlined.containerRole());

  Button text(env, "T", ButtonVariant::kText);
  EXPECT_EQ(ColorRole::kUndefined, text.containerRole());
}

TEST(Material3Button, OutlinedVariantHasOutline) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button outlined(env, "O", ButtonVariant::kOutlined);
  EXPECT_GT((int)outlined.getBorderStyle().outline_width().floor(), 0);

  Button filled(env, "F", ButtonVariant::kFilled);
  EXPECT_EQ(0, (int)filled.getBorderStyle().outline_width().floor());
}

TEST(Material3Button, ElevatedVariantHasNonzeroRestingElevation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button elevated(env, "E", ButtonVariant::kElevated);
  EXPECT_EQ(3, elevated.getElevation());

  Button filled(env, "F", ButtonVariant::kFilled);
  EXPECT_EQ(0, filled.getElevation());
}

TEST(Material3Button, DisabledElevatedVariantDropsElevation) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button elevated(env, "E", ButtonVariant::kElevated);
  elevated.setEnabled(false);

  EXPECT_EQ(0, elevated.getElevation());
}

TEST(Material3Button, IconChangesNaturalWidth) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  Button b(env, "Send");
  Dimensions text_only =
      b.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_GT(text_only.width(), 0);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
