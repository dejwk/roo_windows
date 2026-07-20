#include <stddef.h>

#include "gtest/gtest.h"
#include "material3_navigation_bar_test_access.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"

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

void ExpectDestinationPaintsEveryPixel(NavigationBarLayout layout,
                                       bool selected) {
  constexpr int16_t kWidth = 96;
  constexpr int16_t kHeight = 80;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  const roo_display::Box paint_bounds(0, 0, kWidth - 1, kHeight - 1);
  const roo_display::Color untouched = roo_display::color::Green;
  display.output().fillRect(roo_display::BlendingMode::kSource, paint_bounds,
                            untouched);

  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationBarDestination destination(context, "Inbox",
                                       &ic_outlined_24_action_done());
  NavigationBarDestinationTestAccess::setLayout(destination, layout);
  NavigationBarDestinationTestAccess::setSelected(destination, selected);
  const int16_t destination_height =
      layout == NavigationBarLayout::kVertical ? Scaled(80) : Scaled(64);
  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, kWidth - 1, destination_height - 1));

  roo_display::Surface surface(
      display.output(), 0, 0,
      roo_display::Box(0, 0, kWidth - 1, destination_height - 1),
      /*is_write_once=*/true, roo_display::color::Blue,
      roo_display::FillMode::kExtents, roo_display::BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  canvas.set_out(clipper.out());
  PaintContext paint_context(canvas, clipper);
  NavigationBarDestinationTestAccess::paint(destination, paint_context);

  ASSERT_EQ(1u, clipper.exclusions().size());
  EXPECT_EQ(roo_display::Box(0, 0, kWidth - 1, destination_height - 1),
            clipper.exclusions().front());
  const roo_display::Color quantized_untouched = QuantizeToArgb4444(untouched);
  for (int16_t y = 0; y < destination_height; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      int16_t px[] = {x};
      int16_t py[] = {y};
      roo_display::Color pixel[1];
      offscreen.raster().readColors(px, py, 1, pixel);
      EXPECT_NE(quantized_untouched, pixel[0])
          << "at (" << x << ", " << y << ')';
    }
  }
}

// Verifies the Phase 1 pointer-size-aware budgets that keep badge state off
// base destinations and keep container state limited to destination storage,
// selection, and layout mode.
TEST(Material3NavigationBar, PublicTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kDestinationBudget =
      sizeof(BasicWidget) + sizeof(roo::string_view) + 2 * sizeof(void*) + 8;
  constexpr size_t kBadgedDestinationBudget =
      sizeof(NavigationBarDestination) + sizeof(Badge) + 4;
  constexpr size_t kNavigationBarBudget =
      sizeof(Container) + 3 * sizeof(void*) + 12;

  EXPECT_LE(sizeof(NavigationBarDestination), kDestinationBudget);
  EXPECT_LE(sizeof(BadgedNavigationBarDestination), kBadgedDestinationBudget);
  EXPECT_LE(sizeof(NavigationBar), kNavigationBarBudget);
}

// Verifies that base destinations preserve their non-owning text and icon
// references while defaulting to the compact vertical layout and no selection.
TEST(Material3NavigationBar, DestinationDefaultsAndSetters) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  const MonoIcon& icon = ic_outlined_24_action_done();
  const MonoIcon& selected_icon = ic_outlined_24_action_bookmark();
  NavigationBarDestination destination(context, "Inbox", &icon, &selected_icon);

  EXPECT_EQ("Inbox", destination.label());
  EXPECT_EQ(&icon, destination.icon());
  EXPECT_EQ(&selected_icon, destination.selectedIcon());
  EXPECT_EQ(NavigationBarLayout::kVertical, destination.layout());
  EXPECT_FALSE(destination.selected());
  EXPECT_TRUE(destination.isClickable());
  EXPECT_EQ(Widget::OVERLAY_NONE, destination.getOverlayType());

  destination.setLabel("Saved");
  destination.setIcon(nullptr);
  destination.setSelectedIcon(nullptr);

  EXPECT_EQ("Saved", destination.label());
  EXPECT_EQ(nullptr, destination.icon());
  EXPECT_EQ(nullptr, destination.selectedIcon());
}

// Verifies that destination measurement follows the compact stacked and
// medium inline geometry contracts without storing separate layout metrics.
TEST(Material3NavigationBar, DestinationMeasuresForBothLayouts) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationBarDestination destination(context, "Inbox",
                                       &ic_outlined_24_action_done());

  Dimensions vertical = destination.measure(WidthSpec::Unspecified(0),
                                            HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(80), vertical.height());
  EXPECT_GE(vertical.width(), Scaled(64));

  NavigationBarDestinationTestAccess::setLayout(
      destination, NavigationBarLayout::kHorizontal);
  Dimensions horizontal = destination.measure(WidthSpec::Unspecified(0),
                                              HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(64), horizontal.height());
  EXPECT_GT(horizontal.width(), 0);
  EXPECT_LT(horizontal.width(), vertical.width() + Scaled(64));
}

// Verifies that vertical and horizontal selected destinations retain an icon
// anchor for the later badge phase and derive selection through the bar-only
// state hook rather than a public per-destination selection API.
TEST(Material3NavigationBar, SelectionStateAndIconAnchorFollowLayout) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationBarDestination destination(context, "Inbox",
                                       &ic_outlined_24_action_done());
  static_cast<Widget&>(destination).layout(Rect(0, 0, Scaled(95), Scaled(79)));

  NavigationBarDestinationTestAccess::setSelected(destination, true);
  Rect vertical_icon =
      NavigationBarDestinationTestAccess::iconBounds(destination);
  EXPECT_TRUE(destination.selected());
  EXPECT_FALSE(vertical_icon.empty());

  NavigationBarDestinationTestAccess::setLayout(
      destination, NavigationBarLayout::kHorizontal);
  static_cast<Widget&>(destination).layout(Rect(0, 0, Scaled(143), Scaled(63)));
  Rect horizontal_icon =
      NavigationBarDestinationTestAccess::iconBounds(destination);
  EXPECT_FALSE(horizontal_icon.empty());
  EXPECT_LT(horizontal_icon.xMin(), Scaled(72));
  EXPECT_EQ(Scaled(24), horizontal_icon.width());
}

// Verifies that destination paint settles every local pixel in both layouts,
// including pill corners, foreground-slot gaps, and surface margins.
TEST(Material3NavigationBar, DestinationPaintSettlesEveryPixelExactlyOnce) {
  ExpectDestinationPaintsEveryPixel(NavigationBarLayout::kVertical, true);
  ExpectDestinationPaintsEveryPixel(NavigationBarLayout::kHorizontal, true);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
