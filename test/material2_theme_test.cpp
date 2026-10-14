#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material2/icon_with_caption.h"
#include "roo_windows/material2/theme.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows/material3/button/button.h"

namespace roo_windows::material2 {
namespace {

// Verifies copies own both systems, including interaction tables,
// independently.
TEST(Material2Theme, OwnsPaletteAndInteractionsIndependently) {
  Theme original = DefaultTheme();
  Theme theme = original;
  Material2Theme& material = theme.material2Theme();
  const Theme& const_theme = theme;
  EXPECT_EQ(&material, &const_theme.material2Theme());
  EXPECT_NE(static_cast<const void*>(&material),
            static_cast<const void*>(&theme.material3Theme()));
  material.color.primary = Color(0xFF123456);
  material.state.neutral[static_cast<unsigned>(InteractionState::kPressed)] =
      93;
  EXPECT_NE(original.material2Theme().color.primary, material.color.primary);
  EXPECT_EQ(31, original.material2Theme().state.neutral[4]);
  EXPECT_EQ(original.material3Theme().color.primary,
            theme.material3Theme().color.primary);
  theme.material3Theme().color.primary = Color(0xFFABCDEF);
  theme.material3Theme().state.disabledContentOpacity = 7;
  EXPECT_EQ(Color(0xFF123456), material.color.primary);
  EXPECT_EQ(49, material.state.disabledContentOpacity);
}

// Verifies defaults match the previous palette and state opacity policy on
// background, surface, and colored Material 2 container roles.
TEST(Material2Theme, PreservesLegacyDefaults) {
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
  EXPECT_EQ(previous.state.disabledContentOpacity,
            material.state.disabledContentOpacity);
  for (unsigned i = 0; i < 6; ++i) {
    const auto state = static_cast<InteractionState>(i);
    EXPECT_EQ(
        previous.stateLayer(material3::ColorToken::kBackground, state).a(),
        material.state.opacity(ColorRole::kBackground, state));
    EXPECT_EQ(previous.stateLayer(material3::ColorToken::kPrimary, state).a(),
              material.state.opacity(ColorRole::kPrimary, state));
  }
  for (unsigned token_index = 0; token_index < 33; ++token_index) {
    const auto token = static_cast<material3::ColorToken>(token_index);
    const ColorRole role = Material2ColorRole(token);
    EXPECT_EQ(previous.color.contentColorFor(token),
              material.color.contentColorFor(role));
    EXPECT_EQ(previous.color.accentColorFor(token),
              material.color.accentColorFor(role));
    for (unsigned i = 0; i < 6; ++i) {
      const auto state = static_cast<InteractionState>(i);
      EXPECT_EQ(previous.stateLayer(token, state).a(),
                material.state.opacity(role, state));
    }
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

// Verifies shared hover/press/disabled effects dispatch to the widget's system,
// without adding theme pointers or policy fields to every Material 2 control.
TEST(Material2Theme, InteractionsUseTheOwningSystem) {
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
  theme.material2Theme().state.colored[0] = 37;
  theme.material2Theme().state.colored[4] = 82;
  theme.material2Theme().state.disabledContentOpacity = 64;
  EXPECT_EQ(Color(0xFF123456), material2_button.interactionColor());
  EXPECT_EQ(37, material2_button.interactionOpacity(InteractionState::kHover));
  EXPECT_EQ(82,
            material2_button.interactionOpacity(InteractionState::kPressed));
  EXPECT_EQ(64, material2_button.disabledContentOpacity());
  EXPECT_EQ(49, material3_button.disabledContentOpacity());
  EXPECT_NE(material2_button.interactionColor(),
            material3_button.interactionColor());
}

}  // namespace
}  // namespace roo_windows::material2
