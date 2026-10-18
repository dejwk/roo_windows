#include <Arduino.h>
#include <stddef.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "material3_navigation_rail_test_access.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/blank.h"

namespace roo_windows {
namespace material3 {
namespace {

class WriteCountingDevice final
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  WriteCountingDevice(int width, int height, roo::byte* raster)
      : OffscreenDevice(width, height, raster, roo_display::Argb4444()),
        width_(width),
        writes_(width * height, 0) {}

  void reset() { std::fill(writes_.begin(), writes_.end(), 0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(roo_display::Color* colors, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(roo_display::Color color, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::fill(color, count);
  }

  void writePixels(roo_display::BlendingMode mode, roo_display::Color* values,
                   int16_t* x, int16_t* y, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(roo_display::BlendingMode mode, roo_display::Color color,
                  int16_t* x, int16_t* y, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(roo_display::BlendingMode mode, roo_display::Color* values,
                  int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(roo_display::BlendingMode mode, roo_display::Color color,
                 int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                 uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  uint16_t writesAt(int x, int y) const { return writes_[y * width_ + x]; }

 private:
  void countPixels(uint32_t count) {
    while (count-- != 0) {
      if (x_ < width_ && y_ < static_cast<int>(writes_.size() / width_)) {
        ++writes_[y_ * width_ + x_];
      }
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  int width_;
  std::vector<uint16_t> writes_;
  int x_ = 0;
  int y_ = 0;
  int left_ = 0;
  int right_ = 0;
};

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

bool TextBadgeUsesMaterialIconPlacement(const Rect& icon_bounds,
                                        const Rect& badge_bounds) {
  return badge_bounds.xMin() == icon_bounds.xMax() - Scaled(12) &&
         badge_bounds.yMax() == icon_bounds.yMin() + Scaled(14);
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

  roo_scheduler::SchedulingService scheduler;
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
  Clipper clipper(clipper_state, canvas.out());
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

class QueuedKeySource : public KeySource {
 public:
  int drain(KeyEvent* out, int max_events) override {
    int count = 0;
    while (count < max_events && !events_.empty()) {
      out[count++] = events_.front();
      events_.erase(events_.begin());
    }
    return count;
  }

  void push(KeyEvent event) { events_.push_back(event); }

 private:
  bool hasPendingEvents() const override { return !events_.empty(); }

  std::vector<KeyEvent> events_;
};

// Verifies the Phase 1 pointer-size-aware budgets that keep badge state off
// base destinations and keep rail state limited to its header, destination
// storage, selection, and layout bits.
TEST(Material3NavigationRail, PublicTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kDestinationBudget =
      sizeof(Widget) + sizeof(roo::string_view) + 2 * sizeof(void*) + 8;
  constexpr size_t kBadgedDestinationBudget =
      sizeof(NavigationRailDestination) + sizeof(Badge) + 4;
  constexpr size_t kNavigationRailBudget =
      sizeof(Container) + sizeof(Widget*) + sizeof(std::vector<void*>) + 8;

  EXPECT_LE(sizeof(NavigationRailDestination), kDestinationBudget);
  EXPECT_LE(sizeof(BadgedNavigationRailDestination), kBadgedDestinationBudget);
  EXPECT_LE(sizeof(NavigationRail), kNavigationRailBudget);
}

// Verifies collapsed and expanded persistent rails choose independent slots.
TEST(Material3NavigationRail, UsesThemedContainerRolesForBothLayouts) {
  roo_scheduler::SchedulingService scheduler;
  Theme theme = DefaultTheme();
  material3::Material3Theme& material = theme.material3Theme();
  material.components.navigationRail.collapsedContainer =
      ColorToken::kSurfaceContainerLow;
  material.components.navigationRail.expandedContainer =
      ColorToken::kSurfaceContainerHigh;

  Environment env(scheduler, theme);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);

  EXPECT_EQ(ColorToken::kSurfaceContainerLow, rail.containerRole());
  EXPECT_EQ(material.color.surfaceContainerLow, rail.background());
  rail.setLayout(NavigationRailLayout::kExpanded);
  EXPECT_EQ(ColorToken::kSurfaceContainerHigh, rail.containerRole());
  EXPECT_EQ(material.color.surfaceContainerHigh, rail.background());
}

// Verifies baseline geometry propagates to existing and newly added items,
// and an explicit surface role persists across layout switches and can reset.
TEST(Material3NavigationRail, BaselineAndSurfaceOverrides) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  NavigationRailDestination first(context, "Home");
  NavigationRailDestination second(context, "Inbox");
  ASSERT_TRUE(rail.add(first));
  rail.setLayout(NavigationRailLayout::kBaseline);
  ASSERT_TRUE(rail.add(second));
  EXPECT_EQ(NavigationRailLayout::kBaseline, first.layout());
  EXPECT_EQ(NavigationRailLayout::kBaseline, second.layout());
  const Dimensions size =
      rail.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(80), size.width());
  EXPECT_EQ(Scaled(128), size.height());
  static_cast<Widget&>(rail).layout(
      Rect(0, 0, size.width() - 1, size.height() - 1));
  EXPECT_EQ(Scaled(8), first.parent_bounds().yMin());
  EXPECT_EQ(Scaled(60), first.height());
  rail.setContainerRole(ColorToken::kSurfaceContainer);
  rail.setLayout(NavigationRailLayout::kExpanded);
  EXPECT_EQ(ColorToken::kSurfaceContainer, rail.containerRole());
  EXPECT_EQ(env.theme().material3Theme().color.surfaceContainer,
            rail.background());
  rail.setContainerRole(ColorToken::kNone);
  EXPECT_EQ(ColorToken::kNone, rail.containerRole());
  rail.clearContainerRoleOverride();
  EXPECT_EQ(
      env.theme().material3Theme().components.navigationRail.expandedContainer,
      rail.containerRole());
}

// Verifies a changed surface reaches descendants without overdraw during
// initial, dirty, invalidated, and surface-role-change refreshes.
TEST(Material3NavigationRail, SurfaceOverrideRepaintSettlesPixelsOnce) {
  constexpr int kWidth = 96;
  constexpr int kHeight = 180;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  WriteCountingDevice device(kWidth, kHeight, raster.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  app.refresh();
  auto rail = std::make_unique<NavigationRail>(app.context());
  NavigationRail* raw = rail.get();
  raw->setContainerRole(ColorToken::kSurfaceContainer);
  auto destination = std::make_unique<BadgedNavigationRailDestination>(
      app.context(), "Inbox", &ic_outlined_24_action_done());
  destination->setBadgeValue(3);
  raw->add(std::move(destination));
  app.add(std::move(rail), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  for (int pass = 0; pass < 4; ++pass) {
    device.reset();
    if (pass == 1) raw->setDirty();
    if (pass == 2) raw->invalidateInterior();
    if (pass == 3) raw->setContainerRole(ColorToken::kSurfaceContainerHigh);
    app.refresh();
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        EXPECT_LE(device.writesAt(x, y), 1) << pass << ": " << x << "," << y;
        if (pass != 1) {
          EXPECT_EQ(device.writesAt(x, y), 1);
        }
      }
    }
    int16_t x = 0;
    int16_t y = 0;
    Color pixel;
    device.raster().readColors(&x, &y, 1, &pixel);
    EXPECT_EQ(QuantizeToArgb4444(raw->background()), pixel);
  }
}

TEST(Material3NavigationRail, DestinationDefaultsAndSetters) {
  roo_scheduler::SchedulingService scheduler;
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
  EXPECT_EQ(ClickActivationPolicy::kImmediateContinueAnimation,
            destination.getClickActivationPolicy());
  EXPECT_EQ(Widget::OVERLAY_CUSTOM, destination.getOverlayType());
  EXPECT_EQ(Widget::ClickOverlayAnimation::kFade,
            destination.getClickOverlayAnimation());
  EXPECT_EQ(0, destination.interactionOpacity(InteractionState::kSelected));
  EXPECT_EQ(env.theme().material3Theme().color.onSurfaceVariant,
            destination.interactionColor(InteractionState::kHover));

  NavigationRailDestinationTestAccess::setSelected(destination, true);
  EXPECT_EQ(env.theme().material3Theme().color.onSecondaryContainer,
            destination.interactionColor(InteractionState::kHover));
  destination.setLabel("Saved");
  destination.setIcon(nullptr);
  destination.setSelectedIcon(nullptr);
  EXPECT_EQ("Saved", destination.label());
  EXPECT_EQ(nullptr, destination.icon());
  EXPECT_EQ(nullptr, destination.selectedIcon());
}

TEST(Material3NavigationRail, DestinationMeasuresForBothLayouts) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRailDestination destination(context, "Inbox",
                                        &ic_outlined_24_action_done());

  Dimensions collapsed = destination.measure(WidthSpec::Unspecified(0),
                                             HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(64), collapsed.height());
  EXPECT_GE(collapsed.width(), Scaled(96));

  NavigationRailDestinationTestAccess::setLayout(
      destination, NavigationRailLayout::kExpanded);
  Dimensions expanded = destination.measure(WidthSpec::Unspecified(0),
                                            HeightSpec::Unspecified(0));
  EXPECT_EQ(collapsed.height(), expanded.height());
  EXPECT_EQ(Scaled(220), expanded.width());
}

// Verifies the collapsed 64 dp target and taller targets preserve the reference
// icon/label positions and the full label line box, independent of selection.
TEST(Material3NavigationRail, CollapsedContentMatchesMaterialPlacement) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRailDestination destination(context, "Opcje",
                                        &ic_outlined_24_action_done());
  for (int height : {64, 72}) {
    for (bool selected : {false, true}) {
      NavigationRailDestinationTestAccess::setSelected(destination, selected);
      static_cast<Widget&>(destination)
          .layout(Rect(0, 0, Scaled(96) - 1, Scaled(height) - 1));
      const Rect icon =
          NavigationRailDestinationTestAccess::iconBounds(destination);
      const Rect label =
          NavigationRailDestinationTestAccess::labelBounds(destination);
      EXPECT_EQ(Scaled(10 + (height - 64) / 2), icon.yMin());
      EXPECT_EQ(Scaled(24), icon.height());
      EXPECT_EQ(Scaled(8), label.yMin() - icon.yMax() - 1);
      EXPECT_EQ(Scaled(16), label.height());
      EXPECT_EQ(Scaled(6), destination.height() - label.yMax() - 1 -
                               Scaled((height - 64) / 2));
    }
  }
}

// Verifies actual label pixels center the ascent band and use separate rail
// label ink, and initial, dirty and invalidated frames settle pixels once.
TEST(Material3NavigationRail, LabelMidAscentColorsAndRepaintWrites) {
  for (NavigationRailLayout layout :
       {NavigationRailLayout::kCollapsed, NavigationRailLayout::kExpanded,
        NavigationRailLayout::kBaseline}) {
    for (bool selected : {false, true}) {
      for (bool enabled : {false, true}) {
        const int width =
            Scaled(layout == NavigationRailLayout::kExpanded   ? 220
                   : layout == NavigationRailLayout::kBaseline ? 80
                                                               : 96);
        const int height =
            Scaled(layout == NavigationRailLayout::kBaseline ? 60 : 64);
        std::vector<roo::byte> raster(width * height * 2);
        WriteCountingDevice device(width, height, raster.data());
        roo_display::Display display(device);
        roo_scheduler::SchedulingService scheduler;
        Environment env(scheduler);
        Application app(&env, display);
        // Complete the window startup prefill before the first destination
        // paint.
        app.refresh();
        auto destination = std::make_unique<NavigationRailDestination>(
            app.context(), "Opcje", &ic_outlined_24_action_done());
        NavigationRailDestination* raw = destination.get();
        NavigationRailDestinationTestAccess::setLayout(*raw, layout);
        NavigationRailDestinationTestAccess::setSelected(*raw, selected);
        raw->setEnabled(enabled);
        app.add(std::move(destination),
                roo_display::Box(0, 0, width - 1, height - 1));
        for (int pass = 0; pass < 3; ++pass) {
          SCOPED_TRACE(testing::Message()
                       << "layout=" << static_cast<int>(layout)
                       << " selected=" << selected << " enabled=" << enabled
                       << " pass=" << pass);
          device.reset();
          if (pass == 1) raw->setDirty();
          if (pass == 2) raw->invalidateInterior();
          app.refresh();
          for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
              ASSERT_EQ(1, device.writesAt(x, y)) << x << "," << y;
            }
          }
        }
        // Disabled content receives an additional framework opacity effect.
        // Check its write coverage above; compare unmodified enabled ink below.
        if (!enabled) continue;
        const Rect label =
            NavigationRailDestinationTestAccess::labelBounds(*raw);
        const ColorScheme& colors = env.theme().material3Theme().color;
        const roo_display::Color ink =
            !selected ? colors.onSurfaceVariant
            : layout != NavigationRailLayout::kExpanded
                ? colors.secondary
                : colors.onSecondaryContainer;
        const roo_display::Color background =
            selected && layout == NavigationRailLayout::kExpanded
                ? colors.secondaryContainer
                : colors.surface;
        std::vector<roo::byte> expected_raster(width * height * 2);
        roo_display::OffscreenDevice<roo_display::Argb4444> expected(
            width, height, expected_raster.data(), roo_display::Argb4444());
        roo_display::Display expected_display(expected);
        roo_display::Surface surface(
            expected_display.output(), 0, 0,
            roo_display::Box(0, 0, width - 1, height - 1), true, background,
            roo_display::FillMode::kExtents,
            roo_display::BlendingMode::kSourceOver);
        Canvas canvas(&surface);
        canvas.clear();
        const TextStyle& style = layout == NavigationRailLayout::kExpanded
                                     ? text_style_label_large()
                                     : text_style_label_medium();
        roo_display::ClippedStringViewLabel text("Opcje", style.font(), ink,
                                                 style.fontOptions());
        canvas.drawObject(
            text, label.xMin() + (label.width() - text.metrics().advance()) / 2,
            label.yMin() + (label.height() + style.ascent()) / 2);
        for (int16_t y = label.yMin(); y <= label.yMax(); ++y) {
          for (int16_t x = label.xMin(); x <= label.xMax(); ++x) {
            roo_display::Color actual_color;
            roo_display::Color expected_color;
            device.raster().readColors(&x, &y, 1, &actual_color);
            expected.raster().readColors(&x, &y, 1, &expected_color);
            ASSERT_EQ(expected_color, actual_color) << x << "," << y;
          }
        }
      }
    }
  }
}

TEST(Material3NavigationRail, ExpandedContentHugsIconAndLabel) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRailDestination destination(context, "Inbox",
                                        &ic_outlined_24_action_done());

  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, Scaled(96) - 1, Scaled(64) - 1));
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
  const Rect icon =
      NavigationRailDestinationTestAccess::iconBounds(destination);
  const Rect label =
      NavigationRailDestinationTestAccess::labelBounds(destination);
  EXPECT_EQ(Scaled(8), label.xMin() - icon.xMax() - 1);
  EXPECT_EQ(text_style_label_large().lineHeight(), label.height());
  EXPECT_EQ(Scaled(56),
            NavigationRailDestinationTestAccess::indicatorBounds(destination)
                .height());
}

TEST(Material3NavigationRail, DestinationPaintSettlesEveryPixelExactlyOnce) {
  ExpectDestinationPaintsEveryPixel(NavigationRailLayout::kCollapsed, true);
  ExpectDestinationPaintsEveryPixel(NavigationRailLayout::kExpanded, true);
}

TEST(Material3NavigationRail, BadgedDestinationCapsValuesAndStaysInTarget) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  BadgedNavigationRailDestination destination(
      context, "Inbox", &ic_outlined_24_action_bookmark());

  destination.setBadgeValue(1000);
  static_cast<Widget&>(destination)
      .layout(Rect(0, 0, Scaled(96) - 1, Scaled(64) - 1));
  EXPECT_EQ(BadgeMode::kText, destination.badge().mode());
  EXPECT_EQ("999+", destination.badge().text());
  EXPECT_TRUE(ContainsRect(destination.bounds(), destination.badge().bounds()));
  EXPECT_TRUE(TextBadgeUsesMaterialIconPlacement(
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

TEST(Material3NavigationRail, CollapsedTextBadgesUseMaterialIconPlacement) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  BadgedNavigationRailDestination destination(
      context, "Inbox", &ic_outlined_24_action_bookmark());
  destination.setBadgeValue(1000);
  ASSERT_TRUE(rail.add(WidgetRef(destination)));
  rail.measure(WidthSpec::Exactly(Scaled(96)),
               HeightSpec::Exactly(Scaled(108)));
  static_cast<Widget&>(rail).layout(
      Rect(0, 0, Scaled(96) - 1, Scaled(108) - 1));

  const Rect icon_bounds =
      NavigationRailDestinationTestAccess::iconBounds(destination);
  const Rect badge_bounds = destination.badge().bounds();
  EXPECT_EQ(icon_bounds.xMax() - Scaled(12), badge_bounds.xMin());
  EXPECT_EQ(icon_bounds.yMin() + Scaled(14), badge_bounds.yMax());
}

TEST(Material3NavigationRail, ExpandedBadgesMirrorBesideTheLabelInRtl) {
  roo_scheduler::SchedulingService scheduler;
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
  const Rect rtl_icon =
      NavigationRailDestinationTestAccess::iconBounds(destination);
  EXPECT_EQ(Scaled(8), rtl_icon.xMin() - rtl_label.xMax() - 1);
  EXPECT_TRUE(ContainsRect(destination.bounds(), rtl_badge));
}

TEST(Material3NavigationRail, RailOwnsSelectionAndReselection) {
  roo_scheduler::SchedulingService scheduler;
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

// Verifies reselection is delivered before repaint and only once.
TEST(Material3NavigationRail, ReselectionContinuesAnimationWithoutRedelivery) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 80;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto rail = std::make_unique<TestNavigationRail>(app.context());
  auto home = std::make_unique<NavigationRailDestination>(
      app.context(), "Home", &ic_outlined_24_action_done());
  NavigationRailDestination* home_raw = home.get();
  ASSERT_TRUE(rail->add(WidgetRef(std::move(home))));
  app.add(std::move(rail), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();
  ASSERT_TRUE(home_raw->selected());
  ASSERT_FALSE(home_raw->isDirty());

  home_raw->onShowPress(home_raw->width() / 2, home_raw->height() / 2);
  NavigationRailDestinationTestAccess::tapUp(*home_raw, home_raw->width() / 2,
                                             home_raw->height() / 2);
  EXPECT_TRUE(home_raw->isClicking());
  EXPECT_TRUE(home_raw->selected());
  EXPECT_EQ(std::vector<int>({0}),
            static_cast<TestNavigationRail*>(home_raw->parent())->reselected);

  delay(kPressAnimationMillis + 20);
  app.root().refreshClickAnimation();
  app.refresh();
  ASSERT_FALSE(home_raw->isClicking());
  // Clearing the terminal click state schedules the final settled repaint.
  app.refresh();
  EXPECT_FALSE(home_raw->isDirty());
  EXPECT_EQ(std::vector<int>({0}),
            static_cast<TestNavigationRail*>(home_raw->parent())->reselected);
}

// Verifies release selects immediately and animation completion cannot invoke
// again.
TEST(Material3NavigationRail,
     TouchReleaseSelectsImmediatelyAndContinuesAnimation) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 160;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::SchedulingService scheduler;
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
  app.refresh();

  inbox_raw->onShowPress(inbox_raw->width() / 2, inbox_raw->height() / 2);
  NavigationRailDestinationTestAccess::tapUp(*inbox_raw, inbox_raw->width() / 2,
                                             inbox_raw->height() / 2);
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_EQ(1, rail_raw->selectedIndex());
  EXPECT_TRUE(inbox_raw->selected());
  EXPECT_EQ(std::vector<int>({1}), rail_raw->invoked);
  ASSERT_NE(nullptr, inbox_raw->getClickAnimation());
  EXPECT_LT(inbox_raw->getClickAnimation()->progress(), 1.0f);
  app.refresh();
  EXPECT_TRUE(inbox_raw->isClicking());

  delay(kPressAnimationMillis + 20);
  app.root().refreshClickAnimation();
  const ClickAnimation* animation = inbox_raw->getClickAnimation();
  ASSERT_NE(nullptr, animation);
  // Animation remains attached until its terminal frame has painted.
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_GE(animation->progress(), 1.0f);
  app.refresh();
  EXPECT_FALSE(inbox_raw->isClicking());

  // Completing feedback must preserve selection without a second invocation.
  EXPECT_EQ(1, rail_raw->selectedIndex());
  EXPECT_TRUE(inbox_raw->selected());
  EXPECT_EQ(std::vector<int>({1}), rail_raw->invoked);
  EXPECT_TRUE(rail_raw->reselected.empty());
  app.refresh();
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
  roo_scheduler::SchedulingService scheduler;
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
  app.refresh();

  inbox_raw->onShowPress(inbox_raw->width() / 2, inbox_raw->height() / 2);
  delay(kPressAnimationMillis + 20);
  app.refresh();
  ASSERT_FALSE(inbox_raw->isClicking());
  ASSERT_TRUE(inbox_raw->isDirty());
  ASSERT_EQ(0, rail_raw->selectedIndex());

  // The completed final refresh schedules held-state settlement. Releasing
  // before that frame is painted must merge selection into it rather than
  // paint one intermediate unselected frame.
  NavigationRailDestinationTestAccess::tapUp(*inbox_raw, inbox_raw->width() / 2,
                                             inbox_raw->height() / 2);

  EXPECT_EQ(1, rail_raw->selectedIndex());
  EXPECT_TRUE(inbox_raw->selected());
  EXPECT_EQ(std::vector<int>({1}), rail_raw->invoked);
  EXPECT_EQ(nullptr, inbox_raw->getClickAnimation());
  EXPECT_TRUE(inbox_raw->isDirty());

  app.refresh();
  EXPECT_FALSE(inbox_raw->isDirty());
}

// Verifies that keyboard activation enters the shared click-animation path so
// key-up merges selection into the pending held-press settlement repaint.
TEST(Material3NavigationRail,
     KeyReleaseCoalescesSelectionIntoHeldPressSettlement) {
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 160;
  roo::byte raster[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen(
      kWidth, kHeight, raster, roo_display::Argb4444());
  roo_display::Display display(offscreen);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  QueuedKeySource keys;
  Application app(&env, display, keys, false);

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
  app.refresh();
  ASSERT_TRUE(inbox_raw->requestFocus());
  app.refresh();

  keys.push(KeyEvent{KeyPhase::kDown, KeyCode::kEnter, 0, 0});
  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  EXPECT_TRUE(inbox_raw->isPressed());
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_EQ(inbox_raw, app.root().click_animation().target());

  delay(kPressAnimationMillis + 20);
  app.root().refreshClickAnimation();
  app.refresh();
  ASSERT_TRUE(inbox_raw->isPressed());
  ASSERT_FALSE(inbox_raw->isClicking());
  ASSERT_TRUE(inbox_raw->isDirty());
  ASSERT_EQ(0, rail_raw->selectedIndex());

  keys.push(KeyEvent{KeyPhase::kUp, KeyCode::kEnter, 0, 0});
  delay(30);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);

  EXPECT_EQ(1, rail_raw->selectedIndex());
  EXPECT_TRUE(inbox_raw->selected());
  EXPECT_EQ(std::vector<int>({1}), rail_raw->invoked);
  EXPECT_FALSE(inbox_raw->isClicking());
  EXPECT_EQ(nullptr, inbox_raw->getClickAnimation());
  // Input dispatch may leave painting for a later frame. Complete that frame
  // explicitly so this assertion does not depend on scheduler timing.
  app.refresh();
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
  roo_scheduler::SchedulingService scheduler;
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
  app.add(std::move(rail), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();

  inbox_raw->onShowPress(inbox_raw->width() / 2, inbox_raw->height() / 2);
  NavigationRailDestinationTestAccess::tapUp(*inbox_raw, inbox_raw->width() / 2,
                                             inbox_raw->height() / 2);
  ASSERT_EQ(inbox_raw, app.root().click_animation().target());
  ASSERT_TRUE(inbox_raw->isClicking());

  // A quick release has no preceding onShowPress(). It must still honor the
  // active-animation guard instead of replacing the controller target and
  // stranding the previous destination in kWidgetClicking.
  NavigationRailDestinationTestAccess::tapUp(*home_raw, home_raw->width() / 2,
                                             home_raw->height() / 2);

  EXPECT_EQ(inbox_raw, app.root().click_animation().target());
  EXPECT_TRUE(inbox_raw->isClicking());
  EXPECT_FALSE(home_raw->isClicking());
  EXPECT_EQ(
      1,
      static_cast<TestNavigationRail*>(inbox_raw->parent())->selectedIndex());
}

TEST(Material3NavigationRail, RailCapsDestinationsAndRetainsHeaderOnClear) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  Blank header(context, Dimensions(40, 40));
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
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  Blank header(context, Dimensions(40, 40));
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
  EXPECT_EQ(80, first.parent_bounds().width());
  EXPECT_EQ(44, header.parent_bounds().yMin());
  EXPECT_EQ(124, first.parent_bounds().yMin());
  EXPECT_EQ(64, first.parent_bounds().height());
  EXPECT_EQ(4,
            second.parent_bounds().yMin() - first.parent_bounds().yMax() - 1);
  EXPECT_EQ(4,
            third.parent_bounds().yMin() - second.parent_bounds().yMax() - 1);

  // Top space, header, header gap, three 64 dp items and two 4 dp gaps
  // consume exactly 324 dp in the collapsed variant.
  rail.measure(WidthSpec::Exactly(80), HeightSpec::Exactly(324));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 79, 323));
  EXPECT_EQ(124, first.parent_bounds().yMin());
  EXPECT_EQ(323, third.parent_bounds().yMax());

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
  EXPECT_EQ(320, first.parent_bounds().width());
}

// Verifies a header consuming all remaining space clears stale hit targets.
TEST(Material3NavigationRail, HeaderExhaustionClearsDestinations) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  Blank header(context, Dimensions(40, 40));
  NavigationRailDestination destination(context, "Home");
  rail.setHeader(header);
  rail.add(destination);
  rail.measure(WidthSpec::Exactly(96), HeightSpec::Exactly(240));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 95, 239));
  EXPECT_FALSE(destination.bounds().empty());
  rail.measure(WidthSpec::Exactly(96), HeightSpec::Exactly(100));
  static_cast<Widget&>(rail).layout(Rect(0, 0, 95, 99));
  EXPECT_TRUE(destination.bounds().empty());
}

TEST(Material3NavigationRail, CollapsedWidthDoesNotDependOnDestinationState) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  NavigationRail rail(context);
  NavigationRailDestination status(context, "Status",
                                   &ic_outlined_24_action_done());
  BadgedNavigationRailDestination alerts(context, "Alerts",
                                         &ic_outlined_24_action_bookmark());
  alerts.setBadgeValue(2);
  ASSERT_TRUE(rail.add(WidgetRef(status)));
  ASSERT_TRUE(rail.add(WidgetRef(alerts)));

  EXPECT_EQ(Scaled(96),
            rail.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
                .width());
  static_cast<Widget&>(rail).layout(
      Rect(0, 0, Scaled(96) - 1, Scaled(176) - 1));
  EXPECT_EQ(Scaled(44), status.parent_bounds().yMin());
  EXPECT_EQ(Scaled(64), status.parent_bounds().height());
  EXPECT_EQ(Scaled(112), alerts.parent_bounds().yMin());
  EXPECT_EQ(Scaled(96), status.parent_bounds().width());
  EXPECT_EQ(Scaled(96), alerts.parent_bounds().width());

  rail.setSelectedIndex(1);
  EXPECT_EQ(Scaled(96),
            rail.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
                .width());
}

TEST(Material3NavigationRail, ArrowKeysMoveFocusWithoutChangingSelection) {
  roo_scheduler::SchedulingService scheduler;
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
