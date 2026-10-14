#include <algorithm>
#include <array>
#include <memory>
#include <tuple>
#include <utility>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/dialogs/alert_dialog.h"
#include "roo_windows/material2/navigation_panel.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/card/flex_card.h"
#include "roo_windows/material3/fab/fab.h"
#include "roo_windows/material3/menu/menu_surface.h"
#include "roo_windows/material3/utilities/dropdown_button.h"

namespace roo_windows {
namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Color;
constexpr int kWidth = 160;
constexpr int kHeight = 140;
const Rect kTargetBounds(48, 40, 103, 95);

// Counts actual device writes across every output entry point, after filters.
class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* data)
      : OffscreenDevice(kWidth, kHeight, data, Argb8888()) {}

  void reset() { writes.fill(0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(Color value, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::fill(value, count);
  }

  void writePixels(BlendingMode mode, Color* values, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(BlendingMode mode, Color value, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(value, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* values, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(BlendingMode mode, Color value, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(value, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  std::array<uint16_t, kWidth * kHeight> writes{};

 private:
  void countPixels(uint32_t count) {
    while (count-- != 0) {
      EXPECT_GE(x_, 0);
      EXPECT_LT(x_, kWidth);
      EXPECT_GE(y_, 0);
      EXPECT_LT(y_, kHeight);
      if (x_ >= 0 && x_ < kWidth && y_ >= 0 && y_ < kHeight) {
        ++writes[y_ * kWidth + x_];
      }

      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  int16_t left_ = 0;
  int16_t right_ = 0;
  int16_t x_ = 0;
  int16_t y_ = 0;
};

class MutableSurface : public SurfaceWidget {
 public:
  using SurfaceWidget::moveTo;
  using SurfaceWidget::SurfaceWidget;
  Color background() const override { return color_; }
  BorderStyle getBorderStyle() const override {
    return BorderStyle(radius_, 0);
  }

  uint8_t getElevation() const override { return elevation_; }
  Dimensions getSuggestedMinimumDimensions() const override { return {56, 56}; }
  void paint(PaintContext& ctx) const override { ctx.clear(); }

  void setColor(Color color) {
    color_ = color;
    invalidateInterior();
  }

  void setRadius(uint8_t radius) {
    BorderStyle previous = getBorderStyle();
    radius_ = radius;
    borderStyleChanged(previous);
  }

  void setElevation(uint8_t elevation) {
    uint8_t previous = elevation_;
    elevation_ = elevation;
    elevationChanged(std::max(previous, elevation));
  }

 private:
  Color color_{0xFF7289AB};
  uint8_t radius_ = 16;
  uint8_t elevation_ = 6;
};

class LegacyButton : public material2::Button {
 public:
  explicit LegacyButton(ApplicationContext& context)
      : material2::Button(context) {}

  Dimensions getSuggestedMinimumDimensions() const override { return {56, 56}; }

  void paint(PaintContext& ctx) const override { ctx.clear(); }
};

class ElevatedPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  Color background() const override { return Color(0xFFECE4DB); }
  BorderStyle getBorderStyle() const override { return BorderStyle(12, 0); }
  uint8_t getElevation() const override { return 6; }
};

class SurfaceInvalidationTest : public testing::Test {
 protected:
  template <typename T, typename... Args>
  T& addTarget(Args&&... args) {
    auto target =
        std::make_unique<T>(app_.context(), std::forward<Args>(args)...);
    T& result = *target;
    app_.add(std::move(target), kTargetBounds.asBox());
    app_.refresh();
    device_.reset();
    return result;
  }

  void frame(int millis = 0) {
    device_.reset();
    system_time_delay_micros(millis * 1000);
    app_.refresh();
  }

  int writesOutside(const Rect& bounds) const {
    int count = 0;
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        if (x < bounds.xMin() || x > bounds.xMax() || y < bounds.yMin() ||
            y > bounds.yMax()) {
          count += device_.writes[y * kWidth + x];
        }
      }
    }

    return count;
  }

  void expectInteriorOnly() {
    EXPECT_EQ(0, writesOutside(kTargetBounds));
    EXPECT_GT(writesOutside(Rect(0, 0, -1, -1)), 0);
    for (int count : device_.writes) EXPECT_LE(count, 1);
  }

  // Compares incremental output to repainting the same retained scene sample.
  void expectMatchesFullRepaint(const char* stage = "") {
    SCOPED_TRACE(stage);
    const auto previous = raster_;
    app_.root().invalidateInterior();
    app_.refresh();
    for (size_t i = 0; i < raster_.size(); ++i) {
      ASSERT_EQ(previous[i], raster_[i])
          << "pixel " << (i / 4) % kWidth << "," << (i / 4) / kWidth;
    }
  }

  std::array<roo::byte, kWidth * kHeight * 4> raster_{};
  RecordingDevice device_{raster_.data()};
  roo_display::Display display_{device_};
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_;
  Environment env_{scheduler_, theme_};
  Application app_{&env_, display_};
};

class FabInvalidationTest
    : public SurfaceInvalidationTest,
      public testing::WithParamInterface<std::tuple<bool, bool>> {};

// Verifies ordinary and quick taps, including final settlement, write no shadow
// pixels for either FAB family while the rendered shape and elevation are
// fixed.
TEST_P(FabInvalidationTest, ClickFramesStayInsideBounds) {
  SurfaceWidget* target;
  if (std::get<0>(GetParam())) {
    target = &addTarget<material3::ExtendedFloatingActionButton>(
        "", ic_outlined_24_action_done());
  } else {
    target = &addTarget<material3::FloatingActionButton>(
        ic_outlined_24_action_done());
  }

  if (!std::get<1>(GetParam())) {
    target->onShowPress(28, 28);
    frame();
    expectInteriorOnly();
    expectMatchesFullRepaint();
  }

  target->onSingleTapUp(28, 28);
  frame();
  expectInteriorOnly();
  for (int i = 0; i < 4; ++i) {
    frame(40);
    expectInteriorOnly();
    expectMatchesFullRepaint();
  }

  frame(400);
  expectInteriorOnly();
  frame();
  expectInteriorOnly();
  expectMatchesFullRepaint();
}

INSTANTIATE_TEST_SUITE_P(FabFamilies, FabInvalidationTest,
                         testing::Combine(testing::Bool(), testing::Bool()));

// Verifies elevation growth, shrinkage, and removal restore the entire old
// shadow, including sharp-cornered surfaces that need no corner restoration.
TEST_F(SurfaceInvalidationTest, ElevationChangesRestoreOldAndNewCoverage) {
  auto& target = addTarget<MutableSurface>();
  for (uint8_t radius : {16, 0}) {
    target.setRadius(radius);
    frame();
    for (uint8_t elevation : {12, 2, 0, 6}) {
      target.setElevation(elevation);
      frame();
      EXPECT_GT(writesOutside(kTargetBounds), 0);
      expectMatchesFullRepaint();
    }
  }
}

// Verifies actual rendered radii control shadow damage, including clamped
// radii which differ numerically but produce the same pixels.
TEST_F(SurfaceInvalidationTest, BorderChangesRepaintOnlyWhenGeometryChanges) {
  auto& target = addTarget<MutableSurface>();
  for (uint8_t radius : {0, 8, 28}) {
    target.setRadius(radius);
    frame();
    EXPECT_GT(writesOutside(kTargetBounds), 0);
    expectMatchesFullRepaint();
  }

  target.setRadius(255);
  target.invalidateInterior();
  frame();
  expectInteriorOnly();
  expectMatchesFullRepaint();
}

// Verifies color-only surface updates preserve shadows and still restore
// antialiased corners against the backdrop inside the widget rectangle.
TEST_F(SurfaceInvalidationTest, ColorChangesStayInsideBounds) {
  auto& target = addTarget<MutableSurface>();
  target.setColor(Color(0xFFA6D478));
  frame();
  expectInteriorOnly();
  expectMatchesFullRepaint();
}

// Verifies a size-token change damages the FAB shadow even when fixed parent
// constraints retain the same outer dimensions.
TEST_F(SurfaceInvalidationTest, FabSizeChangeRepaintsShadow) {
  auto& target =
      addTarget<material3::FloatingActionButton>(ic_outlined_24_action_done());
  target.setSize(material3::FabSize::kSmall);
  frame();
  EXPECT_EQ(kTargetBounds, target.parent_bounds());
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies a legacy button's radius setter restores its elevated boundary.
TEST_F(SurfaceInvalidationTest, LegacyButtonRadiusChangeRepaintsShadow) {
  auto& target = addTarget<LegacyButton>();
  target.setElevation(6, 6);
  frame();
  target.setCornerRadius(20);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies card radius overrides compare rendered geometry before damaging
// shadows, and clearing an override restores the authored shape.
TEST_F(SurfaceInvalidationTest, CardRadiusOverridesRespectClamping) {
  auto& target =
      addTarget<material3::FlexCard>(material3::FlexCard::Style::kElevated);
  target.setCornerRadius(28);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  target.setCornerRadius(255);
  target.invalidateInterior();
  frame();
  expectInteriorOnly();
  target.clearCornerRadiusOverride();
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies changing a menu's variant repaints its changed shadow corners.
TEST_F(SurfaceInvalidationTest, MenuPolicyChangeRepaintsShadow) {
  auto& target = addTarget<material3::internal::MenuPanel>();
  material3::MenuPolicy policy;
  policy.variant = material3::ListVariant::kBaseline;
  target.setPolicy(policy);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies FAB hover/elevation changes still damage shadows after ordinary
// interaction invalidation is narrowed.
TEST_F(SurfaceInvalidationTest, FabElevationTransitionsRepaintShadow) {
  auto& target =
      addTarget<material3::FloatingActionButton>(ic_outlined_24_action_done());
  target.setHover(true);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  target.setPressed(true);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  target.setEnabled(false);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies button morph frames repaint shadows while radii change, then stop
// writing shadow pixels once the shape settles even though the ripple
// continues.
TEST_F(SurfaceInvalidationTest, ButtonMorphStopsDamagingSettledShadow) {
  auto& target =
      addTarget<material3::Button>("", material3::ButtonVariant::kElevated);
  target.onShowPress(28, 28);
  frame();
  expectInteriorOnly();
  frame(20);
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  frame(80);
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  frame(20);
  expectInteriorOnly();
  expectMatchesFullRepaint();
  target.setShapeMorph(material3::ButtonShapeMorph::kDisabled);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  frame(20);
  expectInteriorOnly();
}

// Verifies forced completion and controller removal restore the correct shadow
// both when a click is confirmed and when partially morphed feedback is
// canceled.
TEST_F(SurfaceInvalidationTest,
       ButtonFinalizationAndCancellationRestoreShadow) {
  auto& target =
      addTarget<material3::Button>("", material3::ButtonVariant::kElevated);
  for (bool cancel : {true, false}) {
    target.onShowPress(28, 28);
    frame(20);
    target.onSingleTapUp(28, 28);
    if (cancel) {
      app_.root().click_animation().cancel(target);
    } else {
      ASSERT_TRUE(app_.root().click_animation().forceFinalFrame(target));
    }

    frame();
    EXPECT_GT(writesOutside(kTargetBounds), 0);
    if (!cancel) {
      frame();
      EXPECT_GT(writesOutside(kTargetBounds), 0);
    }

    expectMatchesFullRepaint();
  }
}

// Verifies the dropdown's shared button morph follows the same damage contract.
TEST_F(SurfaceInvalidationTest, DropdownMorphRepaintsShadow) {
  static const char* const kItems[] = {"One"};
  auto& target = addTarget<material3::DropdownButton>(kItems);
  target.setVariant(material3::ButtonVariant::kElevated);
  frame();
  target.onShowPress(28, 28);
  frame(20);
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
  frame(80);
  frame(20);
  expectInteriorOnly();
  target.setPressed(false);
  app_.root().click_animation().cancel(target);
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies an unchanged shadow is still composited when damage comes from a
// changed backdrop, and structural move/hide/show damage retains shadow
// extents.
TEST_F(SurfaceInvalidationTest, BackdropAndStructuralDamageKeepShadows) {
  auto backdrop = std::make_unique<MutableSurface>(app_.context());
  auto* behind = backdrop.get();
  behind->setRadius(0);
  behind->setElevation(0);
  app_.add(std::move(backdrop), Rect(0, 0, kWidth - 1, kHeight - 1).asBox());
  auto& target = addTarget<MutableSurface>();
  behind->setColor(Color(0xFFDFC47A));
  frame();
  expectMatchesFullRepaint("backdrop");
  target.moveTo(kTargetBounds.translate(12, 8));
  frame();
  expectMatchesFullRepaint("move");
  target.setVisibility(Visibility::kInvisible);
  frame();
  expectMatchesFullRepaint("hide");
  target.setVisibility(Visibility::kVisible);
  frame();
  expectMatchesFullRepaint("show");
}

// Verifies invalidating a container preserves the complete visual footprint of
// its unclipped children while excluding its own unchanged decoration overflow.
TEST_F(SurfaceInvalidationTest, ContainerInteriorIncludesChildOverflow) {
  auto& panel = addTarget<ElevatedPanel>();
  panel.invalidateInterior();
  frame();
  expectInteriorOnly();
  auto child = std::make_unique<MutableSurface>(app_.context());
  child->setParentClipMode(ParentClipMode::kUnclipped);
  panel.add(std::move(child), Rect(42, 12, 69, 39));
  frame();
  panel.invalidateInterior();
  frame();
  EXPECT_GT(writesOutside(kTargetBounds), 0);
  expectMatchesFullRepaint();
}

// Verifies the M2 rail/page composition settles pixels once on initial, dirty,
// selection-change, and invalidated frames, with incremental output matching
// a complete repaint.
TEST_F(SurfaceInvalidationTest, Material2NavigationPanelPaintsOnce) {
  // Finish display initialization before counting the composition's first
  // frame.
  app_.refresh();
  auto panel = std::make_unique<material2::NavigationPanel>(app_.context());
  material2::NavigationPanel& target = *panel;
  panel->addPage(ic_outlined_24_action_done(), "Home",
                 std::make_unique<MutableSurface>(app_.context()));
  auto second = std::make_unique<MutableSurface>(app_.context());
  second->setColor(Color(0xFFABCDEF));
  panel->addPage(ic_outlined_24_action_bookmark(), "Inbox", std::move(second));
  app_.add(std::move(panel), Rect(0, 0, kWidth - 1, kHeight - 1).asBox());
  frame();
  for (int count : device_.writes) EXPECT_EQ(count, 1);
  expectMatchesFullRepaint("initial");

  const auto& rail =
      static_cast<const material2::NavigationRail&>(*target.children()[0]);
  rail.children()[1]->setDirty();
  frame();
  EXPECT_GT(writesOutside(Rect(0, 0, -1, -1)), 0);
  for (int count : device_.writes) EXPECT_LE(count, 1);
  expectMatchesFullRepaint("dirty");

  target.setActive(1);
  frame();
  for (int count : device_.writes) EXPECT_LE(count, 1);
  expectMatchesFullRepaint("selection");

  target.invalidateInterior();
  frame();
  for (int count : device_.writes) EXPECT_EQ(count, 1);
  expectMatchesFullRepaint("invalidated");
}

// Verifies a constructed M2 button follows its live palette after invalidation,
// is unaffected by M3 palette edits, and preserves explicit fill/ink overrides.
// Both theme refresh and dirty interaction frames settle each pixel once.
TEST_F(SurfaceInvalidationTest, Material2ThemeChangesRepaintLiveDefaults) {
  auto& button = addTarget<material2::SimpleButton>("A");
  const auto initial = raster_;
  theme_.material3Theme().color.primary = Color(0xFF123456);
  app_.invalidateTheme();
  frame();
  EXPECT_EQ(initial, raster_);
  theme_.material2Theme().color.primary = Color(0xFF246824);
  theme_.material2Theme().color.onPrimary = Color(0xFFFFFF00);
  app_.invalidateTheme();
  frame();
  EXPECT_NE(initial, raster_);
  for (int count : device_.writes) EXPECT_LE(count, 1);
  expectMatchesFullRepaint("palette");

  theme_.material2Theme().state.colored[4] = 91;
  button.setPressed(true);
  frame();
  expectInteriorOnly();
  expectMatchesFullRepaint("pressed");
  button.setPressed(false);
  button.setEnabled(false);
  theme_.material2Theme().state.disabledContentOpacity = 64;
  app_.invalidateTheme();
  frame();
  for (int count : device_.writes) EXPECT_LE(count, 1);
  expectMatchesFullRepaint("disabled");
  button.setEnabled(true);
  button.setInteriorColor(button.background());
  button.setContentColor(button.contentColor());
  frame();
  const auto fixed = raster_;
  theme_.material2Theme().color.primary = Color(0xFFBB2288);
  theme_.material2Theme().color.onPrimary = Color(0xFF00FFFF);
  app_.invalidateTheme();
  frame();
  EXPECT_EQ(fixed, raster_);
}

// Verifies muted alert text resolves its theme color after construction,
// including detached presentation roots invalidated by Application.
TEST_F(SurfaceInvalidationTest, Material2AlertSupportingTextStaysLive) {
  Task& owner = app_.addTaskFullScreen();
  material2::AlertDialog dialog(app_.context(), "", "Supporting text", {});
  app_.refresh();
  ASSERT_EQ(PresentationStartResult::kStarted, dialog.show(owner, [](int) {}));
  frame();
  const auto initial = raster_;
  theme_.material2Theme().color.onSurface = Color(0xFFEE0022);
  app_.invalidateTheme();
  frame();
  EXPECT_NE(initial, raster_);
  for (int count : device_.writes) EXPECT_LE(count, 1);
  expectMatchesFullRepaint("alert theme");
}

}  // namespace
}  // namespace roo_windows
