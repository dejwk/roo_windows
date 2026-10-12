#include <algorithm>
#include <vector>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/material3/button/internal/split_chevrons.h"
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

// Verifies the subtracted rectangles leave an open V and retain a fixed
// half-pixel pivot for both odd and even bounds throughout the rotation.
TEST(Material3SplitButtonGeometry, ChevronRotatesAroundFixedCenter) {
  internal::SplitChevron chevron;
  for (int extent : {47, 48}) {
    roo_display::Box bounds(0, 0, extent - 1, extent - 1);
    std::vector<roo_display::Color> down(extent * extent);
    std::vector<roo_display::Color> up(extent * extent);
    chevron.setGeometry(bounds, 24, 0.0f, roo_display::color::White);
    ASSERT_EQ(2u, chevron.image().inputCount());
    chevron.image().readColorRect(0, 0, extent - 1, extent - 1, down.data());
    chevron.setGeometry(bounds, 24, 1.0f, roo_display::color::White);
    chevron.image().readColorRect(0, 0, extent - 1, extent - 1, up.data());
    int opaque = 0;
    for (int i = 0; i < extent * extent; ++i) {
      EXPECT_NEAR(down[i].a(), up[extent * extent - 1 - i].a(), 1);
      if (down[i].a() > 200) ++opaque;
    }
    EXPECT_GT(opaque, 8);
    EXPECT_EQ(0, down[(extent / 2 - 2) * extent + extent / 2].a());
    for (float fraction : {0.0f, 0.25f, 0.5f, 0.75f, 1.0f}) {
      chevron.setGeometry(bounds, 24, fraction, roo_display::color::White);
      EXPECT_EQ(bounds, chevron.image().anchorExtents());
      EXPECT_EQ(bounds, chevron.image().extents());
    }
  }
}

// Verifies both rectangles rotate together, preserving arm area between the
// cardinal samples where a square's symmetry could conceal a reversed angle.
TEST(Material3SplitButtonGeometry, ChevronKeepsItsShapeBetweenCardinalAngles) {
  internal::SplitChevron chevron;
  std::vector<roo_display::Color> pixels(120 * 120);
  int reference_alpha = 0;
  for (int frame = 0; frame <= 16; ++frame) {
    chevron.setGeometry(roo_display::Box(0, 0, 119, 119), 96, frame / 16.0f,
                        roo_display::color::White);
    chevron.image().readColorRect(0, 0, 119, 119, pixels.data());
    int total_alpha = 0;
    for (roo_display::Color color : pixels) total_alpha += color.a();
    if (frame == 0) reference_alpha = total_alpha;
    EXPECT_NEAR(reference_alpha, total_alpha, reference_alpha * 0.05);
  }
}

}  // namespace
}  // namespace roo_windows::material3
