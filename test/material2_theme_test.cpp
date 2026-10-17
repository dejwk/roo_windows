#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material2/icon_with_caption.h"
#include "roo_windows/material2/theme.h"
#include "roo_windows/material2/toggle_buttons.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows/material3/button/button.h"

namespace roo_windows::material2 {
namespace {

// Verifies palette copies remain independent of framework feedback settings.
TEST(Material2Theme, OwnsPaletteIndependently) {
  Theme original = DefaultTheme();
  Theme theme = original;
  Material2Theme& material = theme.material2Theme();
  const Theme& const_theme = theme;
  EXPECT_EQ(&material, &const_theme.material2Theme());
  EXPECT_NE(static_cast<const void*>(&material),
            static_cast<const void*>(&theme.material3Theme()));
  material.color.primary = Color(0xFF123456);
  theme.framework.state.pressed = 93;
  EXPECT_NE(original.material2Theme().color.primary, material.color.primary);
  EXPECT_EQ(26, original.framework.state.pressed);
  EXPECT_EQ(original.material3Theme().color.primary,
            theme.material3Theme().color.primary);
  theme.material3Theme().color.primary = Color(0xFFABCDEF);
  theme.framework.state.disabledContentOpacity = 7;
  EXPECT_EQ(Color(0xFF123456), material.color.primary);
  EXPECT_EQ(7, theme.disabledContentOpacity());
}

// Verifies matching default color fields and shared interaction policy.
TEST(Material2Theme, PreservesPaletteDefaults) {
  Theme theme = DefaultTheme();
  const auto& material = theme.material2Theme();
  const auto& previous = theme.material3Theme();
  EXPECT_EQ(previous.color.primary, material.color.primary);
  EXPECT_EQ(previous.color.onPrimary, material.color.onPrimary);
  EXPECT_EQ(previous.color.background, material.color.background);
  EXPECT_EQ(previous.color.onBackground, material.color.onBackground);
  EXPECT_EQ(previous.color.surface, material.color.surface);
  EXPECT_EQ(previous.color.onSurface, material.color.onSurface);
  EXPECT_EQ(previous.color.outline, material.color.outline);
  for (unsigned token_index = 0; token_index < 33; ++token_index) {
    const auto token = static_cast<material3::ColorToken>(token_index);
    const ColorRole role = Material2ColorRole(token);
    EXPECT_EQ(previous.color.contentColorFor(token),
              material.color.contentColorFor(role));
    EXPECT_EQ(previous.color.accentColorFor(token),
              material.color.accentColorFor(role));
  }
}

// Verifies defaults follow palette changes, while explicit overrides (even the
// previous default and transparent colors) remain literal values.
TEST(Material2Theme, ButtonDefaultsStayLiveAndOverridesStayExplicit) {
  Theme theme;
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler, theme);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  SimpleButton live(context, "Live");
  SimpleButton outlined(context, "Outline", Button::OUTLINED);
  SimpleButton fixed(context, "Fixed");
  const Color old_fill = fixed.background();
  fixed.setInteriorColor(old_fill);
  fixed.setOutlineColor(roo_display::color::Transparent);
  fixed.setContentColor(roo_display::color::Transparent);
  fixed.setFont(font_h6());
  theme.material2Theme().color.primary = Color(0xFF123456);
  theme.material2Theme().color.onPrimary = Color(0xFFABCDEF);
  theme.material2Theme().color.outline = Color(0xFF345678);
  theme.material2Theme().color.background = Color(0xFF222222);
  EXPECT_EQ(Color(0xFF123456), live.background());
  EXPECT_EQ(Color(0xFFABCDEF), live.contentColor());
  EXPECT_EQ(Color(0x80345678), outlined.getOutlineColor());
  EXPECT_EQ(Color(0xFF222222), outlined.background());
  EXPECT_EQ(old_fill, fixed.background());
  EXPECT_EQ(roo_display::color::Transparent, fixed.getOutlineColor());
  EXPECT_EQ(roo_display::color::Transparent, fixed.contentColor());
  EXPECT_EQ(&font_h6(), &fixed.getFont());
  EXPECT_EQ(&text_style_button().font(), &live.getFont());
  theme.material3Theme().color.primary = Color(0xFF987654);
  EXPECT_EQ(Color(0xFF123456), live.background());
}

// Verifies shared hover/press/disabled effects use the framework policy,
// without adding theme pointers or policy fields to every Material 2 control.
TEST(Material2Theme, InteractionsUseSharedPolicy) {
  static_assert(sizeof(ThemeMixin<Widget>) == sizeof(Widget),
                "No per-widget policy state");
  Theme theme;
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler, theme);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  SimpleButton material2_button(context, "M2");
  material3::Button material3_button(context, "M3");
  theme.material2Theme().color.onPrimary = Color(0xFF123456);
  theme.framework.state.hover = 37;
  theme.framework.state.pressed = 82;
  theme.framework.state.disabledContentOpacity = 64;
  EXPECT_EQ(Color(0xFF123456), material2_button.interactionColor());
  EXPECT_EQ(37, material2_button.interactionOpacity(InteractionState::kHover));
  EXPECT_EQ(82,
            material2_button.interactionOpacity(InteractionState::kPressed));
  EXPECT_EQ(64, material2_button.disabledContentOpacity());
  EXPECT_EQ(37, material3_button.interactionOpacity(InteractionState::kHover));
  EXPECT_EQ(82,
            material3_button.interactionOpacity(InteractionState::kPressed));
  EXPECT_EQ(64, material3_button.disabledContentOpacity());
  EXPECT_NE(material2_button.interactionColor(),
            material3_button.interactionColor());
}

// Verifies activation has no generic layer and keeps the container transparent
// while transient feedback remains governed by the shared policy.
TEST(Material2Theme, ToggleSelectionOwnsOverlayAboveTransparentContainer) {
  Theme theme;
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler, theme);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  ToggleButtons group(context);
  Widget& button = group.addButton(ic_outlined_24_action_done());
  auto& surface = static_cast<SurfaceWidget&>(button);
  EXPECT_EQ(roo_display::color::Transparent, surface.background());
  EXPECT_EQ(roo_display::color::Transparent, button.componentOverlayColor());
  EXPECT_TRUE(group.setActive(0));
  EXPECT_EQ(roo_display::color::Transparent, surface.background());
  EXPECT_EQ(theme.material2Theme().color.onSurface.withA(31),
            button.componentOverlayColor());
  EXPECT_EQ(0, button.getOverlayOpacity());
  button.setHover(true);
  EXPECT_EQ(20, button.getOverlayOpacity());
  button.setHover(false);
  button.setFocused(true);
  EXPECT_EQ(26, button.getOverlayOpacity());
  button.setFocused(false);
  button.setPressed(true);
  EXPECT_EQ(26, button.getOverlayOpacity());
  button.setPressed(false);
  button.setEnabled(false);
  theme.framework.state.hover = 0;
  theme.framework.state.pressed = 0;
  EXPECT_TRUE(button.isActivated());
  EXPECT_EQ(roo_display::color::Transparent, surface.background());
  EXPECT_EQ(theme.material2Theme().color.onSurface.withA(31),
            button.componentOverlayColor());
}

}  // namespace
}  // namespace roo_windows::material2
