#include <algorithm>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/button/split_button.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {

// Verifies all expressive tiers reserve the authored trailing width and a
// physical 2 dp separation before extra width goes to the primary segment.
TEST(Material3SplitButtonGeometry, NaturalSizesAndExactSegments) {
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  ApplicationContext context(environment.scheduler(), environment.theme(),
                             environment.keyboardColorTheme());
  Menu menu(context);
  SplitButton button(context, menu, "Run");
  const int trailing_sides[] = {13, 13, 15, 29, 43};
  const int trailing_icons[] = {22, 22, 26, 38, 50};
  const int heights[] = {32, 40, 56, 96, 136};
  for (int i = 0; i < 5; ++i) {
    button.setSize(static_cast<ButtonSize>(i));
    Dimensions natural =
        button.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
    EXPECT_EQ(Scaled(heights[i]), natural.height());
    button.layout(Rect(0, 0, natural.width() - 1, natural.height() - 1));
    const Widget* primary = static_cast<Widget&>(button).focusChildAt(0);
    const Widget* trailing = static_cast<Widget&>(button).focusChildAt(1);
    ASSERT_NE(nullptr, primary);
    ASSERT_NE(nullptr, trailing);
    const Button* trailing_button = static_cast<const Button*>(trailing);
    ASSERT_NE(nullptr, trailing_button->icon());
    EXPECT_EQ(Scaled(trailing_icons[i]),
              trailing_button->icon()->anchorExtents().width());
    EXPECT_EQ(Scaled(trailing_icons[i]),
              trailing_button->icon()->anchorExtents().height());
    EXPECT_EQ(Scaled(trailing_sides[i]) + Scaled(trailing_icons[i]) +
                  Scaled(trailing_sides[i]),
              trailing->width());
    EXPECT_EQ(Scaled(2), trailing->offsetLeft() -
                             (primary->offsetLeft() + primary->width()));
    EXPECT_EQ(natural.height(), primary->height());
    EXPECT_EQ(natural.height(), trailing->height());

    button.setLayoutDirection(LayoutDirection::kRightToLeft);
    button.measure(WidthSpec::Exactly(natural.width()),
                   HeightSpec::Exactly(natural.height()));
    button.layout(Rect(0, 0, natural.width() - 1, natural.height() - 1));
    EXPECT_EQ(0, trailing->offsetLeft());
    EXPECT_EQ(Scaled(2), primary->offsetLeft() - trailing->width());
    button.setLayoutDirection(LayoutDirection::kLeftToRight);
  }
}

// Verifies tight width gives space to the trailing action first and never
// creates a phantom primary hit rectangle.
TEST(Material3SplitButtonGeometry, ConstrainedWidthReservesTrailingFirst) {
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  ApplicationContext context(environment.scheduler(), environment.theme(),
                             environment.keyboardColorTheme());
  Menu menu(context);
  SplitButton button(context, menu, "Run");
  button.measure(WidthSpec::Exactly(30), HeightSpec::Exactly(20));
  button.layout(Rect(0, 0, 29, 19));
  const Widget* primary = static_cast<Widget&>(button).focusChildAt(0);
  const Widget* trailing = static_cast<Widget&>(button).focusChildAt(1);
  ASSERT_NE(nullptr, primary);
  ASSERT_NE(nullptr, trailing);
  EXPECT_EQ(0, primary->width());
  EXPECT_EQ(30, trailing->width());
  EXPECT_EQ(0, trailing->offsetLeft());
}

// Verifies all compact density levels change only height and keep the
// trailing width and natural horizontal spacing stable.
TEST(Material3SplitButtonGeometry, DensityKeepsHorizontalGeometry) {
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = DefaultTheme().material3Theme();
  Theme theme{MakeFrameworkTheme(material), &material};
  Environment environment(scheduler, theme);
  ApplicationContext context(environment.scheduler(), environment.theme(),
                             environment.keyboardColorTheme());
  Menu menu(context);
  SplitButton button(context, menu, "Run");
  int natural_width = 0;
  for (int level = 0; level >= -5; --level) {
    material.density = static_cast<Density>(level);
    Dimensions measured =
        button.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
    if (level == 0) natural_width = measured.width();
    EXPECT_EQ(natural_width, measured.width());
    EXPECT_GE(measured.height(), Scaled(22) + 2 * Scaled(4));
    EXPECT_EQ(std::max(Scaled(std::max(24, 40 + 4 * level)),
                       Scaled(22) + 2 * Scaled(4)),
              measured.height());
  }
}

}  // namespace
}  // namespace roo_windows::material3
