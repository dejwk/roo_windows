#include <vector>

#include "button_group_test_support.h"
#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/button/internal/button_group_appearance.h"
namespace roo_windows::material3 {
namespace {
class GoldenPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

class PaletteSurface : public SurfaceWidget {
 public:
  PaletteSurface(ApplicationContext& context, internal::GroupStyle style,
                 bool selected, bool enabled)
      : SurfaceWidget(context),
        style_(style),
        selected_(selected),
        enabled_(enabled) {}
  Color background() const override {
    return internal::ResolveConnectedAppearance(theme(), style_, selected_,
                                                enabled_)
        .container;
  }
  Color getOutlineColor() const override {
    return internal::ResolveConnectedAppearance(theme(), style_, selected_,
                                                enabled_)
        .outline;
  }
  BorderStyle getBorderStyle() const override {
    return internal::ResolveConnectedBorder(
        ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
        true, false, selected_, 0, {width(), height()},
        style_ == internal::GroupStyle::kOutlined && !selected_ ? Scaled(1)
                                                                : 0);
  }
  bool useAutomaticDisabledStyle() const override { return false; }
  Dimensions getSuggestedMinimumDimensions() const override { return {60, 40}; }

 private:
  internal::GroupStyle style_;
  bool selected_;
  bool enabled_;
};
// Verifies real surface rendering for each palette and selection combination.
TEST(ButtonGroupGolden, ConnectedHelperSurfaces) {
  for (bool dark : {false, true}) {
    std::vector<roo::byte> raster(320 * 160 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 160, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Material3Theme material = test_support::MakeAcceptanceTheme(dark);
    Theme theme{MakeFrameworkTheme(material), &material};
    Environment env(scheduler, theme);
    Application app(&env, display);
    GoldenPanel panel(app.context());
    std::vector<std::unique_ptr<PaletteSurface>> entries;
    app.addTaskFullScreen(panel);
    for (int i = 0; i < 12; ++i) {
      entries.push_back(std::make_unique<PaletteSurface>(
          app.context(), static_cast<internal::GroupStyle>(i % 4), i >= 4,
          i < 8));
      panel.add(WidgetRef(*entries.back()),
                Rect(8 + (i % 4) * 76, 8 + (i / 4) * 48, 67 + (i % 4) * 76,
                     47 + (i / 4) * 48));
    }
    app.refresh();
    std::string name = dark ? "helper_dark" : "helper_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 160),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    panel.removeAll();
  }
}

class StripedPanel : public GoldenPanel {
 public:
  using GoldenPanel::GoldenPanel;
  void paint(PaintContext& context) const override {
    for (int x = 0; x < width(); x += 8) {
      context.canvas().fillRect(x, 0, std::min<int>(width() - 1, x + 7),
                                height() - 1,
                                x % 16 == 0 ? roo_display::Color(0xFF335577)
                                            : roo_display::Color(0xFF667788));
    }
  }
};
// Verifies stable mixed entries preserve patterned gaps and default clipping.
TEST(ButtonGroupGolden, InternalLayoutAndRestoration) {
  for (bool unclipped : {false, true}) {
    std::vector<roo::byte> raster(320 * 160 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 160, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    StripedPanel panel(app.context());
    MonoIcon icon = ic_outlined_24_action_home();
    internal::ButtonGroupLabelEntry day(app.context(), "Day",
                                        internal::GroupStyle::kElevated);
    internal::ButtonGroupIconEntry home(app.context(), icon,
                                        internal::GroupStyle::kOutlined);
    internal::ButtonGroupLabelEntry week(app.context(), "Week");
    test_support::InternalGroup group(app.context());
    group.add(day);
    group.add(home);
    group.add(week);
    if (unclipped) day.setParentClipMode(ParentClipMode::kUnclipped);
    app.addTaskFullScreen(panel);
    panel.add(WidgetRef(group), Rect(24, 24, 183, 73));
    app.refresh();
    std::string name = unclipped ? "internal_unclipped" : "internal_clipped";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 160),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    for (LayoutDirection direction :
         {LayoutDirection::kRightToLeft, LayoutDirection::kLeftToRight}) {
      home.setVisibility(direction == LayoutDirection::kRightToLeft
                             ? Visibility::kGone
                             : Visibility::kVisible);
      group.setLayoutDirection(direction);
      app.refresh();
      std::vector<roo::byte> incremental = raster;
      panel.invalidateDescending();
      app.refresh();
      EXPECT_EQ(incremental, raster);
    }
    panel.removeAll();
  }
}

// Verifies selected/pressed/disabled contours and stable anchors across each
// contained style, including mixed artwork, RTL, compact density and dark mode.
TEST(ButtonGroupGolden, ConnectedSelectionAndRestoration) {
  for (bool dark : {false, true}) {
    std::vector<roo::byte> raster(320 * 240 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> device(
        320, 240, raster.data(), {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Material3Theme material = test_support::MakeAcceptanceTheme(dark);
    Theme theme{MakeFrameworkTheme(material), &material};
    Environment env(scheduler, theme);
    Application app(&env, display);
    StripedPanel panel(app.context());
    app.addTaskFullScreen(panel);
    MonoIcon icon = ic_outlined_24_action_home();
    std::vector<std::unique_ptr<ConnectedButtonGroup>> groups;
    for (int i = 0; i < 4; ++i) {
      auto group = std::make_unique<ConnectedButtonGroup>(app.context());
      group->add(std::make_unique<ButtonGroupButton>(
          app.context(), "Day", static_cast<ButtonGroupButtonStyle>(i)));
      auto middle = std::make_unique<ButtonGroupButton>(
          app.context(), "Week", static_cast<ButtonGroupButtonStyle>(i));
      middle->setIcon(&icon);
      group->add(std::move(middle));
      group->add(std::make_unique<ButtonGroupButton>(
          app.context(), "Month", static_cast<ButtonGroupButtonStyle>(i)));
      group->setSelectionMask(2);
      panel.add(WidgetRef(*group), Rect(12, 12 + i * 52, 307, 55 + i * 52));
      groups.push_back(std::move(group));
    }
    app.refresh();
    std::string name = dark ? "connected_dark" : "connected_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 240),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    for (uint32_t mask : {1u, 4u, 2u}) {
      for (auto& group : groups) {
        Widget* child = static_cast<Widget&>(*group).focusChildAt(1);
        Rect previous = child->parent_bounds();
        group->setSelectionMask(mask);
        EXPECT_EQ(previous, child->parent_bounds());
        child->setPressed(mask == 4);
      }
      app.refresh();
      std::vector<roo::byte> incremental = raster;
      panel.invalidateDescending();
      app.refresh();
      EXPECT_EQ(incremental, raster);
    }
    material.density = Density::kMinus3;
    for (auto& group : groups) {
      group->setLayoutDirection(LayoutDirection::kRightToLeft);
      group->setShape(ButtonShape::kSquare);
      group->setEnabled(false);
    }
    panel.requestLayoutDescending();
    panel.invalidateDescending();
    app.refresh();
    name =
        dark ? "connected_disabled_rtl_dark" : "connected_disabled_rtl_light";
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(device.raster(), 0, 0, 320, 240),
        "test/goldens/material3_button_group/" + name + ".ppm", name));
    panel.removeAll();
  }
}
}  // namespace
}  // namespace roo_windows::material3
