#include <stddef.h>

#include "gtest/gtest.h"
#include "material3_navigation_rail_test_access.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"

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

void ExpectDestinationPaintsEveryPixel(NavigationRailLayout layout,
                                       bool selected) {
  constexpr int16_t kWidth = 256;
  constexpr int16_t kHeight = 64;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  const roo_display::Color untouched = roo_display::color::Green;
  display.output().fillRect(roo_display::BlendingMode::kSource,
                            roo_display::Box(0, 0, kWidth - 1, kHeight - 1),
                            untouched);

  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRailDestination destination(context, "Inbox",
                                        &ic_outlined_24_action_done());
  NavigationRailDestinationTestAccess::setLayout(destination, layout);
  NavigationRailDestinationTestAccess::setSelected(destination, selected);
  static_cast<Widget&>(destination).layout(Rect(0, 0, kWidth - 1, kHeight - 1));

  roo_display::Surface surface(
      display.output(), 0, 0, roo_display::Box(0, 0, kWidth - 1, kHeight - 1),
      /*is_write_once=*/true, roo_display::color::Blue,
      roo_display::FillMode::kExtents, roo_display::BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState clipper_state;
  Clipper clipper(clipper_state, canvas.out(), roo_time::Uptime::Max());
  canvas.set_out(clipper.out());
  PaintContext paint_context(canvas, clipper);
  NavigationRailDestinationTestAccess::paint(destination, paint_context);

  ASSERT_EQ(1u, clipper.exclusions().size());
  EXPECT_EQ(roo_display::Box(0, 0, kWidth - 1, kHeight - 1),
            clipper.exclusions().front());
  const roo_display::Color quantized_untouched = QuantizeToArgb4444(untouched);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      int16_t px[] = {x};
      int16_t py[] = {y};
      roo_display::Color pixel[1];
      offscreen.raster().readColors(px, py, 1, pixel);
      EXPECT_NE(quantized_untouched, pixel[0]) << "at (" << x << ", " << y
                                                << ')';
    }
  }
}

// Verifies the Phase 1 pointer-size-aware budgets that keep badge state off
// base destinations and keep rail state limited to its header, destination
// storage, selection, and layout bits.
TEST(Material3NavigationRail, PublicTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kDestinationBudget =
      sizeof(BasicWidget) + sizeof(roo::string_view) + 2 * sizeof(void*) + 8;
  constexpr size_t kBadgedDestinationBudget =
      sizeof(NavigationRailDestination) + sizeof(Badge) + 4;
  constexpr size_t kNavigationRailBudget =
      sizeof(Container) + sizeof(Widget*) + sizeof(std::vector<void*>) + 8;

  EXPECT_LE(sizeof(NavigationRailDestination), kDestinationBudget);
  EXPECT_LE(sizeof(BadgedNavigationRailDestination), kBadgedDestinationBudget);
  EXPECT_LE(sizeof(NavigationRail), kNavigationRailBudget);
}

TEST(Material3NavigationRail, DestinationDefaultsAndSetters) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  const MonoIcon& icon = ic_outlined_24_action_done();
  const MonoIcon& selected_icon = ic_outlined_24_action_bookmark();
  NavigationRailDestination destination(context, "Inbox", &icon,
                                        &selected_icon);

  EXPECT_EQ("Inbox", destination.label());
  EXPECT_EQ(&icon, destination.icon());
  EXPECT_EQ(&selected_icon, destination.selectedIcon());
  EXPECT_EQ(NavigationRailLayout::kCollapsed, destination.layout());
  EXPECT_FALSE(destination.selected());
  EXPECT_TRUE(destination.isClickable());
  EXPECT_EQ(Widget::OVERLAY_CUSTOM, destination.getOverlayType());
  EXPECT_EQ(Widget::ClickOverlayAnimation::kFade,
            destination.getClickOverlayAnimation());
  EXPECT_FALSE(destination.useOverlayOnSelection());
  EXPECT_EQ(ColorToken::kSurface, destination.effectiveOverlayColorRole());

  NavigationRailDestinationTestAccess::setSelected(destination, true);
  EXPECT_EQ(ColorToken::kSecondaryContainer,
            destination.effectiveOverlayColorRole());
  destination.setLabel("Saved");
  destination.setIcon(nullptr);
  destination.setSelectedIcon(nullptr);
  EXPECT_EQ("Saved", destination.label());
  EXPECT_EQ(nullptr, destination.icon());
  EXPECT_EQ(nullptr, destination.selectedIcon());
}

TEST(Material3NavigationRail, DestinationMeasuresForBothLayouts) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRailDestination destination(context, "Inbox",
                                        &ic_outlined_24_action_done());

  Dimensions collapsed = destination.measure(WidthSpec::Unspecified(0),
                                             HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(64), collapsed.height());
  EXPECT_GE(collapsed.width(), Scaled(80));

  NavigationRailDestinationTestAccess::setLayout(
      destination, NavigationRailLayout::kExpanded);
  Dimensions expanded = destination.measure(WidthSpec::Unspecified(0),
                                            HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(64), expanded.height());
  EXPECT_GE(expanded.width(), Scaled(256));
}

TEST(Material3NavigationRail, ExpandedContentHugsIconAndLabel) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRailDestination destination(context, "Inbox",
                                        &ic_outlined_24_action_done());

  static_cast<Widget&>(destination).layout(Rect(0, 0, Scaled(80) - 1,
                                                 Scaled(64) - 1));
  Rect collapsed_content =
      NavigationRailDestinationTestAccess::contentBounds(destination);
  EXPECT_FALSE(collapsed_content.empty());

  NavigationRailDestinationTestAccess::setLayout(
      destination, NavigationRailLayout::kExpanded);
  static_cast<Widget&>(destination).layout(Rect(0, 0, Scaled(320) - 1,
                                                 Scaled(64) - 1));
  Rect expanded_content =
      NavigationRailDestinationTestAccess::contentBounds(destination);
  EXPECT_FALSE(expanded_content.empty());
  EXPECT_LT(expanded_content.width(), Scaled(320));
  EXPECT_LT(expanded_content.xMin(), Scaled(160));
}

TEST(Material3NavigationRail, DestinationPaintSettlesEveryPixelExactlyOnce) {
  ExpectDestinationPaintsEveryPixel(NavigationRailLayout::kCollapsed, true);
  ExpectDestinationPaintsEveryPixel(NavigationRailLayout::kExpanded, true);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
