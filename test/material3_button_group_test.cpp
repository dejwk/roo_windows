#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_windows/material3/button/internal/button_group_appearance.h"
#include "roo_windows/material3/button/internal/icon_geometry.h"
namespace roo_windows::material3 {
namespace {
using namespace internal;
// Verifies selected, unselected and disabled roles for every contained style.
TEST(ButtonGroupHelpers, ConnectedPalette) {
  const Theme& theme = DefaultTheme();
  const ColorScheme& colors = theme.material3Theme().color;
  EXPECT_EQ(colors.surfaceContainer,
            ResolveConnectedAppearance(theme, GroupStyle::kFilled, false, true)
                .container);
  EXPECT_EQ(colors.primary,
            ResolveConnectedAppearance(theme, GroupStyle::kFilled, true, true)
                .container);
  EXPECT_EQ(colors.secondary, ResolveConnectedAppearance(
                                  theme, GroupStyle::kFilledTonal, true, true)
                                  .container);
  EXPECT_EQ(colors.inverseSurface,
            ResolveConnectedAppearance(theme, GroupStyle::kOutlined, true, true)
                .container);
  for (GroupStyle style : {GroupStyle::kFilled, GroupStyle::kFilledTonal,
                           GroupStyle::kOutlined, GroupStyle::kElevated}) {
    EXPECT_EQ(ResolveConnectedAppearance(theme, style, false, false).container,
              ResolveConnectedAppearance(theme, style, true, false).container);
    EXPECT_EQ(roo_display::color::Transparent,
              ResolveConnectedAppearance(theme, style, true, false).outline);
  }
}
// Verifies exposed corners survive press and RTL mirrors only their placement.
TEST(ButtonGroupHelpers, ConnectedCorners) {
  Dimensions frame(80, 40);
  BorderStyle first = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
      true, false, false, 1, frame, 0);
  EXPECT_EQ(20, first.top_left_corner_radius());
  EXPECT_EQ(Scaled(4), first.top_right_corner_radius());
  BorderStyle rtl = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kRightToLeft,
      true, false, false, 1, frame, 0);
  EXPECT_EQ(first.top_left_corner_radius(), rtl.top_right_corner_radius());
  BorderStyle selected = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kSquare, LayoutDirection::kLeftToRight,
      false, false, true, 0, frame, 0);
  EXPECT_EQ(20, selected.top_left_corner_radius());
  BorderStyle single = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
      true, true, true, 1, frame, 0);
  EXPECT_EQ(20, single.top_right_corner_radius());
}
// Verifies extracted icon slots and widths preserve standalone geometry.
TEST(ButtonGroupHelpers, IconMetrics) {
  MonoIcon icon = ic_outlined_24_action_home();
  for (ButtonSize size :
       {ButtonSize::kExtraSmall, ButtonSize::kSmall, ButtonSize::kMedium,
        ButtonSize::kLarge, ButtonSize::kExtraLarge}) {
    EXPECT_EQ(Scaled(IconGeometryFor(size).height_dp),
              IconContainerWidth(size, IconButtonWidth::kUniform));
    EXPECT_GE(ResolveIconSlot(size, icon).width(),
              icon.anchorExtents().width());
    EXPECT_EQ(Scaled(2), ResolveGroupGap(size, true));
  }
}
}  // namespace
}  // namespace roo_windows::material3
