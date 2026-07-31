#include <Arduino.h>
#include <stddef.h>

#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "material3_navigation_rail_test_access.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/widgets/blank.h"

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

bool ContainsRect(const Rect& outer, const Rect& inner) {
  return !inner.empty() && outer.xMin() <= inner.xMin() &&
         outer.yMin() <= inner.yMin() && outer.xMax() >= inner.xMax() &&
         outer.yMax() >= inner.yMax();
}

bool TextBadgeStartsAtIconCenter(const Rect& icon_bounds,
                                 const Rect& badge_bounds) {
  return badge_bounds.xMin() == icon_bounds.xMin() + icon_bounds.width() / 2 &&
         badge_bounds.yMax() == icon_bounds.yMin() + icon_bounds.height() / 2;
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
  roo_windows::internal::ClipperState clipper_state;
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
      EXPECT_NE(quantized_untouched, pixel[0])
          << "at (" << x << ", " << y << ')';
    }
  }
}

class TestNavigationRail : public NavigationRail {
 public:
  explicit TestNavigationRail(ApplicationContext& context)
      : NavigationRail(context) {}

  using NavigationRail::onKeyEvent;

  std::vector<int> invoked;
  std::vector<int> selected_during_invocation;
  std::vector<std::pair<int, int>> selection_changes;
  std::vector<int> reselected;

 protected:
  void onDestinationInvoked(int index) override {
    invoked.push_back(index);
    selected_during_invocation.push_back(selectedIndex());
  }

  void onSelectedIndexChanged(int old_index, int new_index) override {
    selection_changes.emplace_back(old_index, new_index);
  }

  void onSelectedDestinationReselected(int index) override {
    reselected.push_back(index);
  }
};

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

  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, Scaled(80) - 1, Scaled(64) - 1));
  Rect collapsed_content =
      NavigationRailDestinationTestAccess::contentBounds(destination);
  EXPECT_FALSE(collapsed_content.empty());

  NavigationRailDestinationTestAccess::setLayout(
      destination, NavigationRailLayout::kExpanded);
  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, Scaled(320) - 1, Scaled(64) - 1));
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

TEST(Material3NavigationRail, BadgedDestinationCapsValuesAndStaysInTarget) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  BadgedNavigationRailDestination destination(
      context, "Inbox", &ic_outlined_24_action_bookmark());

  destination.setBadgeValue(1000);
  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, Scaled(80) - 1, Scaled(64) - 1));
  EXPECT_EQ(BadgeMode::kText, destination.badge().mode());
  EXPECT_EQ("999+", destination.badge().text());
  EXPECT_TRUE(ContainsRect(destination.bounds(), destination.badge().bounds()));
  EXPECT_TRUE(TextBadgeStartsAtIconCenter(
      NavigationRailDestinationTestAccess::iconBounds(destination),
      destination.badge().bounds()));

  NavigationRailDestinationTestAccess::setLayout(
      destination, NavigationRailLayout::kExpanded);
  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, Scaled(320) - 1, Scaled(64) - 1));
  destination.setBadgeDot();
  EXPECT_EQ(BadgeMode::kDot, destination.badge().mode());
  EXPECT_TRUE(ContainsRect(destination.bounds(), destination.badge().bounds()));
  EXPECT_TRUE(ContainsRect(
      NavigationRailDestinationTestAccess::contentBounds(destination),
      destination.badge().bounds()));
  destination.hideBadge();
  EXPECT_FALSE(destination.badge().visible());
  EXPECT_TRUE(destination.badge().bounds().empty());
}

TEST(Material3NavigationRail, CollapsedTextBadgesAnchorAtIconCenter) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  BadgedNavigationRailDestination destination(
      context, "Inbox", &ic_outlined_24_action_bookmark());
  destination.setBadgeValue(1000);
  ASSERT_TRUE(rail.add(WidgetRef(destination)));
  rail.measure(WidthSpec::Exactly(Scaled(80)), HeightSpec::Exactly(Scaled(64)));
  static_cast<Widget&>(rail).layout(Rect(0, 0, Scaled(80) - 1, Scaled(64) - 1));

  const Rect icon_bounds =
      NavigationRailDestinationTestAccess::iconBounds(destination);
  const Rect badge_bounds = destination.badge().bounds();
  EXPECT_EQ(icon_bounds.xMin() + icon_bounds.width() / 2, badge_bounds.xMin());
  EXPECT_EQ(icon_bounds.yMin() + icon_bounds.height() / 2, badge_bounds.yMax());
}

TEST(Material3NavigationRail, ExpandedBadgesMirrorBesideTheLabelInRtl) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  rail.setLayout(NavigationRailLayout::kExpanded);
  BadgedNavigationRailDestination destination(
      context, "Inbox", &ic_outlined_24_action_bookmark());
  ASSERT_TRUE(rail.add(WidgetRef(destination)));
  rail.measure(WidthSpec::Exactly(Scaled(320)),
               HeightSpec::Exactly(Scaled(96)));
  static_cast<Widget&>(rail).layout(
      Rect(0, 0, Scaled(320) - 1, Scaled(96) - 1));
  destination.setBadgeText("12");
  const Rect ltr_label =
      NavigationRailDestinationTestAccess::labelBounds(destination);
  const Rect ltr_badge = destination.badge().bounds();
  EXPECT_GT(ltr_badge.xMin(), ltr_label.xMax());

  rail.setLayoutDirection(LayoutDirection::kRightToLeft);
  static_cast<Widget&>(rail).layout(
      Rect(0, 0, Scaled(320) - 1, Scaled(96) - 1));
  const Rect rtl_label =
      NavigationRailDestinationTestAccess::labelBounds(destination);
  const Rect rtl_badge = destination.badge().bounds();
  EXPECT_LT(rtl_badge.xMax(), rtl_label.xMin());
  EXPECT_TRUE(ContainsRect(destination.bounds(), rtl_badge));
}

TEST(Material3NavigationRail, RailOwnsSelectionAndReselection) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestNavigationRail rail(context);
  NavigationRailDestination home(context, "Home",
                                 &ic_outlined_24_action_done());
  NavigationRailDestination inbox(context, "Inbox",
                                  &ic_outlined_24_action_bookmark());

  EXPECT_TRUE(rail.add(WidgetRef(home)));
  EXPECT_TRUE(rail.add(WidgetRef(inbox)));
  EXPECT_EQ(0, rail.selectedIndex());
  EXPECT_TRUE(home.selected());
  EXPECT_FALSE(inbox.selected());

  NavigationRailDestinationTestAccess::click(inbox);
  EXPECT_EQ(1, rail.selectedIndex());
  EXPECT_FALSE(home.selected());
  EXPECT_TRUE(inbox.selected());
  EXPECT_EQ(std::vector<int>({1}), rail.invoked);
  EXPECT_EQ(std::vector<int>({0}), rail.selected_during_invocation);
  EXPECT_EQ((std::vector<std::pair<int, int>>{{0, 1}}), rail.selection_changes);
  EXPECT_TRUE(rail.reselected.empty());

  NavigationRailDestinationTestAccess::click(inbox);
  EXPECT_EQ(std::vector<int>({1, 1}), rail.invoked);
  EXPECT_EQ(std::vector<int>({0, 1}), rail.selected_during_invocation);
  EXPECT_EQ(std::vector<int>({1}), rail.reselected);
}

TEST(Material3NavigationRail,
     ReselectionSettlesDirectlyIntoSelectedAppearance) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 80;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto rail = std::make_unique<TestNavigationRail>(app.context());
  auto home = std::make_unique<NavigationRailDestination>(
      app.context(), "Home", &ic_outlined_24_action_done());
  NavigationRailDestination* home_raw = home.get();
  ASSERT_TRUE(rail->add(WidgetRef(std::move(home))));
  app.add(std::move(rail),
          roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(app.refresh());
  ASSERT_TRUE(home_raw->selected());
  ASSERT_FALSE(home_raw->isDirty());

  home_raw->onShowPress(home_raw->width() / 2, home_raw->height() / 2);
  NavigationRailDestinationTestAccess::tapUp(
      *home_raw, home_raw->width() / 2, home_raw->height() / 2);
  ASSERT_TRUE(app.refresh());

  delay(kPressAnimationMillis + 20);
  app.root().refreshClickAnimation();
  ASSERT_TRUE(app.refresh());
  ASSERT_FALSE(home_raw->isClicking());
  ASSERT_TRUE(home_raw->isDirty());

  // The completed final refresh invokes and invalidates after drawing returns.
  // Reselection therefore has one selected settlement frame even though the
  // owner does not change selected state.
  EXPECT_TRUE(home_raw->selected());
  EXPECT_EQ(std::vector<int>({0}),
            static_cast<TestNavigationRail*>(home_raw->parent())->reselected);
  ASSERT_TRUE(app.refresh());
  EXPECT_FALSE(home_raw->isDirty());
}

TEST(Material3NavigationRail, TouchReleaseDefersSelectionUntilClickCompletes) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 160;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto rail = std::make_unique<TestNavigationRail>(app.context());
  TestNavigationRail* rail_raw = rail.get();
  auto home = std::make_unique<NavigationRailDestination>(
      app.context(), "Home", &ic_outlined_24_action_done());
  auto inbox = std::make_unique<NavigationRailDestination>(
      app.context(), "Inbox", &ic_outlined_24_action_bookmark());
  NavigationRailDestination* inbox_raw = inbox.get();
  ASSERT_TRUE(rail->add(WidgetRef(std::move(home))));
  ASSERT_TRUE(rail->add(WidgetRef(std::move(inbox))));
  app.add(std::move(rail), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(app.refresh());

  inbox_raw->onShowPress(inbox_raw->width() / 2, inbox_raw->height() / 2);
  NavigationRailDestinationTestAccess::tapUp(*inbox_raw, inbox_raw->width() / 2,
                                             inbox_raw->height() / 2);
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_EQ(0, rail_raw->selectedIndex());
  EXPECT_FALSE(inbox_raw->selected());
  EXPECT_TRUE(rail_raw->invoked.empty());

  delay(kPressAnimationMillis + 20);
  app.root().refreshClickAnimation();
  const ClickAnimation* animation = inbox_raw->getClickAnimation();
  ASSERT_NE(nullptr, animation);
  // A confirmed target retains its settled overlay until the final frame is
  // actually painted. Clearing it here would leave the destination blank
  // while the deferred selection waits for a clean widget.
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_GE(animation->progress(), 1.0f);
  ASSERT_TRUE(app.refresh());
  EXPECT_FALSE(inbox_raw->isClicking());

  // The completed final refresh invalidates and invokes after drawing returns.
  // The next paint therefore sees the selected state directly; there is no
  // intermediate unselected frame between the final overlay and indicator.
  EXPECT_EQ(1, rail_raw->selectedIndex());
  EXPECT_TRUE(inbox_raw->selected());
  EXPECT_TRUE(inbox_raw->isDirty());
  EXPECT_EQ(std::vector<int>({1}), rail_raw->invoked);
  EXPECT_TRUE(rail_raw->reselected.empty());
  ASSERT_TRUE(app.refresh());
  EXPECT_FALSE(inbox_raw->isDirty());
}

TEST(Material3NavigationRail,
     LateReleaseCoalescesSelectionIntoHeldPressSettlement) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 160;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto rail = std::make_unique<TestNavigationRail>(app.context());
  TestNavigationRail* rail_raw = rail.get();
  auto home = std::make_unique<NavigationRailDestination>(
      app.context(), "Home", &ic_outlined_24_action_done());
  auto inbox = std::make_unique<NavigationRailDestination>(
      app.context(), "Inbox", &ic_outlined_24_action_bookmark());
  NavigationRailDestination* inbox_raw = inbox.get();
  ASSERT_TRUE(rail->add(WidgetRef(std::move(home))));
  ASSERT_TRUE(rail->add(WidgetRef(std::move(inbox))));
  app.add(std::move(rail),
          roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(app.refresh());

  inbox_raw->onShowPress(inbox_raw->width() / 2, inbox_raw->height() / 2);
  delay(kPressAnimationMillis + 20);
  ASSERT_TRUE(app.refresh());
  ASSERT_FALSE(inbox_raw->isClicking());
  ASSERT_TRUE(inbox_raw->isDirty());
  ASSERT_EQ(0, rail_raw->selectedIndex());

  // The completed final refresh schedules held-state settlement. Releasing
  // before that frame is painted must merge selection into it rather than
  // paint one intermediate unselected frame.
  NavigationRailDestinationTestAccess::tapUp(
      *inbox_raw, inbox_raw->width() / 2, inbox_raw->height() / 2);

  EXPECT_EQ(1, rail_raw->selectedIndex());
  EXPECT_TRUE(inbox_raw->selected());
  EXPECT_EQ(std::vector<int>({1}), rail_raw->invoked);
  EXPECT_EQ(nullptr, inbox_raw->getClickAnimation());
  EXPECT_TRUE(inbox_raw->isDirty());

  ASSERT_TRUE(app.refresh());
  EXPECT_FALSE(inbox_raw->isDirty());
}

TEST(Material3NavigationRail,
     QuickReleaseCannotReplaceAnotherDestinationsActiveAnimation) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 160;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto rail = std::make_unique<TestNavigationRail>(app.context());
  auto home = std::make_unique<NavigationRailDestination>(
      app.context(), "Home", &ic_outlined_24_action_done());
  auto inbox = std::make_unique<NavigationRailDestination>(
      app.context(), "Inbox", &ic_outlined_24_action_bookmark());
  NavigationRailDestination* home_raw = home.get();
  NavigationRailDestination* inbox_raw = inbox.get();
  ASSERT_TRUE(rail->add(WidgetRef(std::move(home))));
  ASSERT_TRUE(rail->add(WidgetRef(std::move(inbox))));
  app.add(std::move(rail),
          roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  ASSERT_TRUE(app.refresh());

  inbox_raw->onShowPress(inbox_raw->width() / 2, inbox_raw->height() / 2);
  NavigationRailDestinationTestAccess::tapUp(
      *inbox_raw, inbox_raw->width() / 2, inbox_raw->height() / 2);
  ASSERT_EQ(inbox_raw, app.root().click_animation().target());
  ASSERT_TRUE(inbox_raw->isClicking());

  // A quick release has no preceding onShowPress(). It must still honor the
  // active-animation guard instead of replacing the controller target and
  // stranding the previous destination in kWidgetClicking.
  NavigationRailDestinationTestAccess::tapUp(
      *home_raw, home_raw->width() / 2, home_raw->height() / 2);

  EXPECT_EQ(inbox_raw, app.root().click_animation().target());
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_FALSE(home_raw->isClicking());
  EXPECT_EQ(0, static_cast<TestNavigationRail*>(inbox_raw->parent())
                   ->selectedIndex());
}

TEST(Material3NavigationRail, RailCapsDestinationsAndRetainsHeaderOnClear) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  Blank header(context, Dimensions(32, 24));
  NavigationRailDestination first(context);
  NavigationRailDestination second(context);
  NavigationRailDestination third(context);
  NavigationRailDestination fourth(context);
  NavigationRailDestination fifth(context);
  NavigationRailDestination sixth(context);
  auto seventh = std::make_unique<NavigationRailDestination>(context);
  NavigationRailDestination eighth(context);

  rail.setHeader(WidgetRef(header));
  EXPECT_EQ(&rail, header.parent());
  EXPECT_TRUE(rail.add(WidgetRef(first)));
  EXPECT_TRUE(rail.add(WidgetRef(second)));
  EXPECT_TRUE(rail.add(WidgetRef(third)));
  EXPECT_TRUE(rail.add(WidgetRef(fourth)));
  EXPECT_TRUE(rail.add(WidgetRef(fifth)));
  EXPECT_TRUE(rail.add(WidgetRef(sixth)));
  EXPECT_TRUE(rail.add(WidgetRef(std::move(seventh))));
  EXPECT_EQ(nullptr, seventh);
  EXPECT_FALSE(rail.add(WidgetRef(eighth)));
  EXPECT_EQ(NavigationRail::kMaxDestinations, rail.destinationCount());

  rail.clear();
  EXPECT_EQ(0, rail.destinationCount());
  EXPECT_EQ(-1, rail.selectedIndex());
  EXPECT_EQ(&rail, header.parent());
  EXPECT_EQ(nullptr, first.parent());
  rail.clearHeader();
  EXPECT_EQ(nullptr, header.parent());
}

TEST(Material3NavigationRail, RailLayoutsHeaderAndDestinationGroupByMode) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  Blank header(context, Dimensions(32, 24));
  NavigationRailDestination first(context, "Home",
                                  &ic_outlined_24_action_done());
  NavigationRailDestination second(context, "Inbox",
                                   &ic_outlined_24_action_bookmark());
  NavigationRailDestination third(context, "Saved",
                                  &ic_outlined_24_action_done());
  rail.setHeader(WidgetRef(header));
  ASSERT_TRUE(rail.add(WidgetRef(first)));
  ASSERT_TRUE(rail.add(WidgetRef(second)));
  ASSERT_TRUE(rail.add(WidgetRef(third)));

  rail.measure(WidthSpec::Exactly(80), HeightSpec::Exactly(400));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 79, 399));
  EXPECT_EQ(64, first.parent_bounds().width());
  EXPECT_LT(header.parent_bounds().yMax(), first.parent_bounds().yMin());
  EXPECT_LT(first.parent_bounds().yMax(), second.parent_bounds().yMin());
  EXPECT_LT(second.parent_bounds().yMax(), third.parent_bounds().yMin());

  rail.measure(WidthSpec::Exactly(80), HeightSpec::Exactly(160));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 79, 159));
  EXPECT_EQ(first.parent_bounds().yMax() + 1, second.parent_bounds().yMin());
  EXPECT_EQ(second.parent_bounds().yMax() + 1, third.parent_bounds().yMin());

  rail.setGroupAlignment(NavigationRailGroupAlignment::kCenter);
  rail.measure(WidthSpec::Exactly(80), HeightSpec::Exactly(400));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 79, 399));
  EXPECT_GT(first.parent_bounds().yMin(), header.parent_bounds().yMax() + 16);

  rail.setLayout(NavigationRailLayout::kExpanded);
  rail.measure(WidthSpec::Exactly(320), HeightSpec::Exactly(400));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 319, 399));
  EXPECT_EQ(NavigationRailLayout::kExpanded, first.layout());
  EXPECT_EQ(304, first.parent_bounds().width());
}

TEST(Material3NavigationRail, ArrowKeysMoveFocusWithoutChangingSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  TestNavigationRail rail(context);
  NavigationRailDestination first(context, "Home",
                                  &ic_outlined_24_action_done());
  NavigationRailDestination second(context, "Inbox",
                                   &ic_outlined_24_action_bookmark());
  NavigationRailDestination third(context, "Saved",
                                  &ic_outlined_24_action_done());
  ASSERT_TRUE(rail.add(WidgetRef(first)));
  ASSERT_TRUE(rail.add(WidgetRef(second)));
  ASSERT_TRUE(rail.add(WidgetRef(third)));
  rail.measure(WidthSpec::Exactly(80), HeightSpec::Exactly(256));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 79, 255));
  ASSERT_TRUE(context.focus().requestFocus(second));

  EXPECT_TRUE(rail.onKeyEvent(KeyEvent{KeyPhase::kDown, KeyCode::kDown, 0, 0}));
  EXPECT_EQ(&third, context.focus().focused());
  EXPECT_EQ(0, rail.selectedIndex());
  EXPECT_FALSE(third.selected());
  EXPECT_TRUE(rail.onKeyEvent(KeyEvent{KeyPhase::kRepeat, KeyCode::kUp, 0, 0}));
  EXPECT_EQ(&second, context.focus().focused());
  EXPECT_FALSE(
      rail.onKeyEvent(KeyEvent{KeyPhase::kDown, KeyCode::kRight, 0, 0}));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
