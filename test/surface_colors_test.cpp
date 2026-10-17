#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using material3::ColorToken;
using roo_display::Box;
using roo_display::Color;

class OpenPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

class RolePanel : public OpenPanel {
 public:
  /// Publishes this surface's resolved Material 3 content colors.
  SurfaceColors surfaceColors() const override {
    const auto& colors = theme().material3Theme().color;
    const auto role = containerRole();
    return role == material3::ColorToken::kNone
               ? OpenPanel::surfaceColors()
               : SurfaceColors{colors.contentColorFor(role),
                               colors.accentColorFor(role)};
  }

  using OpenPanel::OpenPanel;
  ColorToken containerRole() const { return role; }
  Color background() const override { return fill; }
  ColorToken role = ColorToken::kSurface;
  Color fill = roo_display::color::Transparent;
};

class PairPanel : public OpenPanel {
 public:
  using OpenPanel::OpenPanel;
  SurfaceColors surfaceColors() const override { return colors; }
  Color background() const override { return Color(0xFF000000); }
  SurfaceColors colors{Color(0xFF00FF00), Color(0xFFFF0000)};
};

class SurfaceColorsTest : public testing::Test {
 protected:
  roo::byte pixels_[64 * 48 * 2]{};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_{
      64, 48, pixels_, roo_display::Argb4444()};
  roo_display::Display display_{device_};
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_;
  Environment environment_{scheduler_, theme_};
  Application app_{&environment_, display_};
  ApplicationContext& context() { return app_.context(); }

  bool containsColor(Color expected, Box bounds) {
    expected = test_support::QuantizeToArgb4444(expected);
    for (int16_t y = bounds.yMin(); y <= bounds.yMax(); ++y) {
      for (int16_t x = bounds.xMin(); x <= bounds.xMax(); ++x) {
        Color actual;
        device_.raster().readColors(&x, &y, 1, &actual);
        if (actual == expected) return true;
      }
    }
    return false;
  }
};

// Verifies detached widgets use live framework colors without retaining state.
TEST_F(SurfaceColorsTest, DetachedRootsUseFrameworkPair) {
  static_assert(sizeof(SurfaceColors) == 8);
  static_assert(sizeof(material2::ThemeMixin<Widget>) == sizeof(Widget));
  static_assert(sizeof(SurfaceWidget) == sizeof(Widget));
  Icon icon(context());
  theme_.framework.content = Color(0xFF123456);
  theme_.framework.emphasis = Color(0xFFABCDEF);
  EXPECT_EQ(theme_.framework.content, icon.defaultColor());
  EXPECT_EQ(theme_.framework.emphasis, icon.surfaceColors().accent);
  theme_.framework.content = Color(0xFF654321);
  EXPECT_EQ(theme_.framework.content, icon.defaultColor());
}

// Verifies generic layouts forward a surface pair across reparenting and do
// not infer colors from a transparent or arbitrary background RGB value.
TEST_F(SurfaceColorsTest, LayoutsForwardLivePairsAcrossReparenting) {
  Icon leaf(context());
  OpenPanel layout(context());
  RolePanel first(context());
  PairPanel second(context());
  theme_.material3.color.onSurface = Color(0xFF123456);
  theme_.material3.color.primary = Color(0xFF789ABC);
  layout.add(leaf);
  first.add(layout);
  EXPECT_EQ(theme_.material3.color.onSurface, leaf.defaultColor());
  EXPECT_EQ(theme_.material3.color.primary, leaf.surfaceColors().accent);
  first.fill = Color(0xFF000000);
  EXPECT_EQ(theme_.material3.color.onSurface, leaf.defaultColor());
  theme_.material3.color.onPrimary = Color(0xFFFEDCBA);
  first.role = ColorToken::kPrimary;
  EXPECT_EQ(theme_.material3.color.onPrimary, leaf.defaultColor());
  EXPECT_EQ(leaf.defaultColor(), leaf.surfaceColors().accent);
  first.removeAll();
  second.add(layout);
  EXPECT_EQ(second.colors.foreground, leaf.defaultColor());
  EXPECT_EQ(second.colors.accent, leaf.surfaceColors().accent);
  second.colors.foreground = Color(0xFF112233);
  EXPECT_EQ(second.colors.foreground, leaf.defaultColor());
  second.removeAll();
  EXPECT_EQ(theme_.framework.content, leaf.defaultColor());
  layout.removeAll();
}

// Verifies Material 2 owned surfaces resolve their own palette and generic
// children forward that pair even inside a differently colored M3 surface.
TEST_F(SurfaceColorsTest, Material2SurfaceKeepsItsPalette) {
  Icon child(context());
  OpenPanel layout(context());
  material2::ThemeMixin<OpenPanel> material2_surface(context());
  class FilledPanel : public material2::ThemeMixin<OpenPanel> {
   public:
    using ThemeMixin::ThemeMixin;
    material2::ColorRole containerRole() const override {
      return material2::ColorRole::kPrimary;
    }
  } filled(context());
  RolePanel material3_surface(context());
  theme_.material2.color.onPrimary = Color(0xFF112233);
  theme_.material3.color.onPrimary = Color(0xFFABCDEF);
  material3_surface.role = ColorToken::kPrimary;
  layout.add(child);
  filled.add(layout);
  material3_surface.add(filled);
  EXPECT_EQ(theme_.material2.color.onPrimary, child.defaultColor());
  EXPECT_EQ(child.defaultColor(), child.surfaceColors().accent);
  material3_surface.removeAll();
  filled.removeAll();
  material2_surface.add(layout);
  EXPECT_EQ(theme_.framework.content, child.defaultColor());
  material2_surface.removeAll();
  layout.removeAll();
}

// Verifies generic icon/label ink, explicit overrides, scrollbar ink, and a
// borrowed checkbox use the pair supplied through a generic layout.
TEST_F(SurfaceColorsTest, GenericContentAndBorrowedCheckboxRenderInheritedInk) {
  auto boundary = std::make_unique<PairPanel>(context());
  PairPanel* owner = boundary.get();
  auto layout = std::make_unique<OpenPanel>(context());
  auto checkbox = std::make_unique<material2::Checkbox>(
      context(), material2::Checkbox::OnOffState::kOn);
  material2::Checkbox* borrowed = checkbox.get();
  layout->add(std::move(checkbox), Rect(0, 0, 23, 23));
  layout->add(std::make_unique<Icon>(context(), ic_outlined_24_action_done()),
              Rect(24, 0, 47, 23));
  layout->add(std::make_unique<Icon>(context(), ic_outlined_24_action_done(),
                                     Color(0xFF0000FF)),
              Rect(0, 24, 23, 47));
  layout->add(std::make_unique<TextLabel>(context(), "A",
                                          material2::text_style_body1()),
              Rect(24, 24, 47, 47));
  auto bar = std::make_unique<VerticalScrollBar>(context());
  bar->setRange(0, 47);
  layout->add(std::move(bar), Rect(58, 0, 63, 47));
  owner->add(std::move(layout), Rect(0, 0, 63, 47));
  app_.add(std::move(boundary), Box(0, 0, 63, 47));
  app_.refresh();
  EXPECT_TRUE(containsColor(owner->colors.accent, Box(0, 0, 23, 23)));
  EXPECT_TRUE(containsColor(owner->colors.foreground, Box(24, 0, 47, 23)));
  EXPECT_TRUE(containsColor(Color(0xFF0000FF), Box(0, 24, 23, 47)));
  EXPECT_TRUE(containsColor(owner->colors.foreground, Box(24, 24, 47, 47)));
  EXPECT_TRUE(containsColor(
      roo_display::AlphaBlend(owner->background(),
                              owner->colors.foreground.withA(0xC0)),
      Box(58, 0, 63, 47)));
  borrowed->setOff();
  owner->colors = {Color(0xFFFFFF00), Color(0xFF00FFFF)};
  owner->invalidateDescending();
  owner->invalidateInterior();
  app_.refresh();
  EXPECT_TRUE(containsColor(
      roo_display::AlphaBlend(owner->background(),
                              owner->colors.foreground.withA(0x90)),
      Box(0, 0, 23, 23)));
  EXPECT_TRUE(containsColor(owner->colors.foreground, Box(24, 0, 47, 23)));
  EXPECT_TRUE(containsColor(Color(0xFF0000FF), Box(0, 24, 23, 47)));
}

}  // namespace
}  // namespace roo_windows
