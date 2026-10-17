#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material2/radio_button.h"
#include "roo_windows/material2/slider.h"
#include "roo_windows/material2/switch.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/checkbox/checkbox.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/radio_button/radio_button.h"
#include "roo_windows/material3/switch/switch.h"

namespace roo_windows {
namespace {

class InkBoundary : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
  SurfaceColors surfaceColors() const override { return colors; }
  SurfaceColors colors{Color(0xFF102030), Color(0xFF405060)};
};

class ComponentInkTest : public testing::Test {
 protected:
  ComponentInkTest() {
    auto& c = theme.material3.color;
    c.primary = Color(0xFF112233);
    c.onPrimary = Color(0xFF223344);
    c.onSurface = Color(0xFF334455);
    c.onSurfaceVariant = Color(0xFF445566);
    c.onSecondaryContainer = Color(0xFF556677);
    c.onSecondary = Color(0xFF667788);
    c.inverseOnSurface = Color(0xFF778899);
  }

  void expectInk(const Widget& widget, Color expected) {
    for (InteractionState state :
         {InteractionState::kHover, InteractionState::kFocus,
          InteractionState::kPressed, InteractionState::kDragged}) {
      EXPECT_EQ(expected, widget.interactionColor(state));
    }
  }

  roo_scheduler::SchedulingService scheduler;
  Theme theme;
  Environment environment{scheduler, theme};
  ApplicationContext context{scheduler, theme,
                             environment.keyboardColorTheme()};
};

// Verifies all button variants use their own content ink across arbitrary
// ancestors, including transparent variants and a live palette change.
TEST_F(ComponentInkTest, ButtonVariantsDoNotLeakAncestorInk) {
  using material3::ButtonVariant;
  const auto& c = theme.material3.color;
  const Color expected[] = {c.primary, c.onPrimary, c.onSecondaryContainer,
                            c.onSurfaceVariant, c.primary};
  const ButtonVariant variants[] = {
      ButtonVariant::kText, ButtonVariant::kFilled, ButtonVariant::kFilledTonal,
      ButtonVariant::kOutlined, ButtonVariant::kElevated};
  InkBoundary parent(context);
  for (int i = 0; i < 5; ++i) {
    material3::Button button(context, "Action", variants[i]);
    parent.add(button);
    expectInk(button, expected[i]);
    parent.colors = {Color(0xFFABCDEF), Color(0xFFFEDCBA)};
    expectInk(button, expected[i]);
    parent.removeAll();
  }
  material3::Button text(context, "Action", ButtonVariant::kText);
  theme.material3.color.primary = Color(0xFFAA3344);
  expectInk(text, theme.material3.color.primary);
}

// Verifies toggle icon ink tracks selection in all styles, rather than the
// role of the parent surface or the unselected IconButton implementation.
TEST_F(ComponentInkTest, ToggleIconStylesFollowSelection) {
  using material3::IconButtonStyle;
  const auto& c = theme.material3.color;
  const IconButtonStyle styles[] = {
      IconButtonStyle::kStandard, IconButtonStyle::kFilled,
      IconButtonStyle::kFilledTonal, IconButtonStyle::kOutlined};
  const Color off[] = {c.onSurfaceVariant, c.onSurfaceVariant,
                       c.onSecondaryContainer, c.onSurfaceVariant};
  const Color on[] = {c.primary, c.onPrimary, c.onSecondary,
                      c.inverseOnSurface};
  InkBoundary parent(context);
  for (int i = 0; i < 4; ++i) {
    material3::ToggleIconButton button(context, ic_outlined_24_action_done(),
                                       nullptr, styles[i], false);
    parent.add(button);
    expectInk(button, off[i]);
    button.setSelected(true);
    expectInk(button, on[i]);
    button.setEnabled(false);
    EXPECT_EQ(0, button.getOverlayOpacity());
    parent.removeAll();
  }
}

// Verifies point halos use component selection ink even when their parent
// supplies unrelated foreground and accent colors.
TEST_F(ComponentInkTest, Material3PointHalosUseLocalSelectionInk) {
  material3::Checkbox checkbox(context);
  material3::RadioButton radio(context);
  material3::Switch toggle(context);
  InkBoundary parent(context);
  parent.add(checkbox);
  parent.add(radio);
  parent.add(toggle);
  for (const Widget* widget :
       {static_cast<Widget*>(&checkbox), static_cast<Widget*>(&radio),
        static_cast<Widget*>(&toggle)}) {
    expectInk(*widget, theme.material3.color.onSurface);
  }
  checkbox.setOn();
  radio.setOn();
  toggle.setOn();
  expectInk(checkbox, theme.material3.color.primary);
  expectInk(radio, theme.material3.color.primary);
  expectInk(toggle, theme.material3.color.primary);
  checkbox.setOnOffState(material3::Checkbox::OnOffState::kIndeterminate);
  expectInk(checkbox, theme.material3.color.primary);
  parent.removeAll();
}

// Verifies borrowed M2 controls follow live surrounding colors rather than
// translating the ancestor's identity through a separate M2 palette.
TEST_F(ComponentInkTest, BorrowedMaterial2ControlsFollowLiveBoundary) {
  material2::Checkbox checkbox(context);
  material2::RadioButton radio(context);
  material2::Switch toggle(context);
  material2::Slider slider(context);
  InkBoundary parent(context);
  parent.add(checkbox);
  parent.add(radio);
  parent.add(toggle);
  parent.add(slider);
  expectInk(checkbox, parent.colors.foreground);
  expectInk(radio, parent.colors.foreground);
  expectInk(toggle, parent.colors.foreground);
  expectInk(slider, parent.colors.accent);
  checkbox.setOn();
  radio.setOn();
  toggle.setOn();
  parent.colors.accent = Color(0xFFCD1256);
  theme.material2.color.primary = Color(0xFFFEABCD);
  expectInk(checkbox, parent.colors.accent);
  expectInk(radio, parent.colors.accent);
  expectInk(toggle, parent.colors.accent);
  expectInk(slider, parent.colors.accent);
  parent.removeAll();
  expectInk(checkbox, theme.framework.emphasis);
}

// Verifies row interaction ink follows visual selection and local appearance,
// even when generic descendants use a different supporting-text foreground.
TEST_F(ComponentInkTest, ListAndMenuRowsUseTheirOwnSelectionInk) {
  material3::ListEntry row(context);
  material3::MenuEntry menu(context);
  InkBoundary parent(context);
  parent.add(row);
  parent.add(menu);
  expectInk(row, theme.material3.color.onSurface);
  expectInk(menu, theme.material3.color.onSurface);
  material3::ListEntryVisualContext visual;
  visual.variant = material3::ListVariant::kExpressive;
  visual.selected = true;
  row.setVisualContext(visual);
  expectInk(row, theme.material3.color.onSecondaryContainer);
  visual.variant = material3::ListVariant::kBaseline;
  menu.setVisualContext(visual);
  expectInk(menu, theme.material3.color.onSecondaryContainer);
  parent.removeAll();
}

// Verifies explicit state-aware overrides still control the shared pipeline.
TEST_F(ComponentInkTest, ExplicitInteractionOverrideRetainsPrecedence) {
  class CustomButton : public material3::Button {
   public:
    using Button::Button;
    Color interactionColor(InteractionState state) const override {
      return state == InteractionState::kPressed ? Color(0xFF123456)
                                                 : Color(0xFF654321);
    }
  } button(context, "Custom");
  EXPECT_EQ(Color(0xFF123456),
            button.interactionColor(InteractionState::kPressed));
  EXPECT_EQ(Color(0xFF654321),
            button.interactionColor(InteractionState::kHover));
}

}  // namespace
}  // namespace roo_windows
