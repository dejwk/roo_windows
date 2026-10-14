#include <cstddef>

#include "gtest/gtest.h"
#include "roo_windows/core/framework_theme.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/default_theme.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows {
namespace {

TEST(FrameworkColorSchemeTest, ResolvesEveryRole) {
  const FrameworkColorScheme scheme = {
      roo_display::Color(1), roo_display::Color(2), roo_display::Color(3),
      roo_display::Color(4), roo_display::Color(5), roo_display::Color(6),
      roo_display::Color(7), roo_display::Color(8)};
  for (uint8_t i = 0; i < 8; ++i) {
    EXPECT_EQ(roo_display::Color(i + 1),
              scheme.resolve(static_cast<FrameworkColorRole>(i)));
  }
}

TEST(FrameworkInteractionThemeTest, ResolvesEveryRoleAndState) {
  FrameworkInteractionTheme interaction = {};
  for (uint8_t role = 0; role < 8; ++role) {
    for (uint8_t state = 0; state < 6; ++state) {
      interaction.layer[role][state] = roo_display::Color(role * 6 + state + 1);
      EXPECT_EQ(roo_display::Color(role * 6 + state + 1),
                interaction.resolve(static_cast<FrameworkColorRole>(role),
                                    static_cast<InteractionState>(state)));
    }
  }
}

TEST(Material3ColorSchemeTest, ResolvesEveryToken) {
  const material3::ColorScheme scheme = {
      roo_display::Color(1),  roo_display::Color(2),  roo_display::Color(3),
      roo_display::Color(4),  roo_display::Color(5),  roo_display::Color(6),
      roo_display::Color(7),  roo_display::Color(8),  roo_display::Color(9),
      roo_display::Color(10), roo_display::Color(11), roo_display::Color(12),
      roo_display::Color(13), roo_display::Color(14), roo_display::Color(15),
      roo_display::Color(16), roo_display::Color(17), roo_display::Color(18),
      roo_display::Color(19), roo_display::Color(20), roo_display::Color(21),
      roo_display::Color(22), roo_display::Color(23), roo_display::Color(24),
      roo_display::Color(25), roo_display::Color(26), roo_display::Color(27),
      roo_display::Color(28), roo_display::Color(29), roo_display::Color(30),
      roo_display::Color(31), roo_display::Color(32), roo_display::Color(33)};
  for (uint8_t token = 0; token < 33; ++token) {
    EXPECT_EQ(roo_display::Color(token + 1),
              scheme.resolve(static_cast<material3::ColorToken>(token)));
  }
}

TEST(Material3StateLayerThemeTest, ResolvesEveryTokenAndState) {
  material3::StateLayerTheme state = {};
  for (uint8_t token = 0; token < 33; ++token) {
    for (uint8_t interaction = 0; interaction < 6; ++interaction) {
      const roo_display::Color color(token * 6 + interaction + 1);
      state.layer[token][interaction] = color;
      EXPECT_EQ(color,
                state.resolve(static_cast<material3::ColorToken>(token),
                              static_cast<InteractionState>(interaction)));
    }
  }
}

// Verifies aggregate Material themes retain their existing color/state
// initialization while adding all compact component-surface defaults.
TEST(Material3ThemeTest, InitializesCompactComponentSurfaceDefaults) {
  const material3::Material3Theme material = {};

  EXPECT_EQ(static_cast<size_t>(3), sizeof(material3::CardTheme));
  EXPECT_EQ(static_cast<size_t>(1), sizeof(material3::ToolbarTheme));
  EXPECT_EQ(static_cast<size_t>(29), sizeof(material3::ComponentTheme));
  // The 29-byte component payload and one-byte density field add 32 aligned
  // bytes to the 929-byte color/state payload (Color has 32-bit alignment).
  EXPECT_EQ(static_cast<size_t>(960), sizeof(material3::Material3Theme));
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerLow,
            material.components.card.elevatedContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHighest,
            material.components.card.filledContainer);
  EXPECT_EQ(material3::ColorToken::kSurface,
            material.components.card.outlinedContainer);
  EXPECT_EQ(material3::ColorToken::kSurface,
            material.components.layoutScaffold.container);
  EXPECT_EQ(material3::ColorToken::kSurface,
            material.components.appBar.flatContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.appBar.scrolledContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHigh,
            material.components.searchBar.container);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.searchAppBar.flatSearchContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHighest,
            material.components.searchAppBar.scrolledSearchContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHigh,
            material.components.dialog.basicContainer);
  EXPECT_EQ(material3::ColorToken::kSurface,
            material.components.dialog.fullScreenContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHigh,
            material.components.datePicker.container);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.navigationBar.container);
  EXPECT_EQ(material3::ColorToken::kSurface,
            material.components.navigationRail.collapsedContainer);
  EXPECT_EQ(material3::ColorToken::kSurface,
            material.components.tabs.primaryContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.list.segmentedContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.menu.baselineContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerLow,
            material.components.menu.expressiveContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHighest,
            material.components.textField.filledContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerLow,
            material.components.button.elevatedContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.toggleIconButton.filledUnselectedContainer);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainerHighest,
            material.components.switchControl.unselectedTrack);
  EXPECT_EQ(material3::ColorToken::kSurfaceContainer,
            material.components.toolbar.standardContainer);
}

TEST(Material3ThemeTest, MakesTheDocumentedFrameworkMapping) {
  material3::Material3Theme material = {};
  material.color.background = roo_display::Color(1);
  material.color.surface = roo_display::Color(2);
  material.color.onSurface = roo_display::Color(3);
  material.color.onSurfaceVariant = roo_display::Color(4);
  material.color.primary = roo_display::Color(5);
  material.color.outlineVariant = roo_display::Color(6);
  material.color.error = roo_display::Color(7);
  material.color.onError = roo_display::Color(8);
  material.state.disabledContentOpacity = 49;
  material.state.layer[static_cast<uint8_t>(material3::ColorToken::kPrimary)]
                      [static_cast<uint8_t>(InteractionState::kPressed)] =
      roo_display::Color(9);

  const FrameworkTheme framework = material3::MakeFrameworkTheme(material);
  EXPECT_EQ(roo_display::Color(1),
            framework.color.resolve(FrameworkColorRole::kCanvas));
  EXPECT_EQ(roo_display::Color(2),
            framework.color.resolve(FrameworkColorRole::kSurface));
  EXPECT_EQ(roo_display::Color(3),
            framework.color.resolve(FrameworkColorRole::kContent));
  EXPECT_EQ(roo_display::Color(4),
            framework.color.resolve(FrameworkColorRole::kMutedContent));
  EXPECT_EQ(roo_display::Color(5),
            framework.color.resolve(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(roo_display::Color(6),
            framework.color.resolve(FrameworkColorRole::kOutline));
  EXPECT_EQ(roo_display::Color(7),
            framework.color.resolve(FrameworkColorRole::kCritical));
  EXPECT_EQ(roo_display::Color(8),
            framework.color.resolve(FrameworkColorRole::kOnCritical));
  EXPECT_EQ(49, framework.interaction.disabledContentOpacity);
  EXPECT_EQ(roo_display::Color(9),
            framework.interaction.resolve(FrameworkColorRole::kEmphasis,
                                          InteractionState::kPressed));
}

TEST(ThemeTest, DefaultThemeComposesMaterial3AndFrameworkThemes) {
  const Theme& theme = DefaultTheme();

  ASSERT_TRUE(theme.hasMaterial3Theme());
  const material3::Material3Theme& material = theme.material3Theme();
  EXPECT_EQ(material.color.background, theme.framework.color.canvas);
  EXPECT_EQ(material.color.surface, theme.framework.color.surface);
  EXPECT_EQ(material.color.onSurface, theme.framework.color.content);
  EXPECT_EQ(material.color.onSurfaceVariant,
            theme.framework.color.mutedContent);
  EXPECT_EQ(material.color.primary, theme.framework.color.emphasis);
  EXPECT_EQ(material.color.outlineVariant, theme.framework.color.outline);
  EXPECT_EQ(material.color.error, theme.framework.color.critical);
  EXPECT_EQ(material.color.onError, theme.framework.color.onCritical);

  EXPECT_EQ(material.color.onPrimary.withA(20),
            material.state.resolve(material3::ColorToken::kPrimary,
                                   InteractionState::kHover));
  EXPECT_EQ(material.color.onSurface.withA(31),
            material.state.resolve(material3::ColorToken::kSurface,
                                   InteractionState::kPressed));
}

// Verifies the compatibility API borrows the exact same static defaults.
TEST(ThemeTest, CompatibilityDefaultUsesExplicitMaterial3Default) {
  EXPECT_EQ(&DefaultTheme(), &material3::DefaultTheme());
  EXPECT_EQ(&DefaultTheme().material3Theme(),
            &material3::DefaultMaterial3Theme());
}

// Verifies construction with a custom palette derives interaction colors
// without modifying shared defaults, and projects into the framework theme.
TEST(Material3ThemeTest, BuildsStateLayersFromCustomPalette) {
  material3::ColorScheme colors = material3::DefaultColorScheme();
  colors.primary = roo_display::Color(0xFF123456);
  colors.onPrimary = roo_display::Color(0xFFABCDEF);
  colors.onSurface = roo_display::Color(0xFF654321);
  const material3::Material3Theme theme = material3::MakeTheme(colors);
  EXPECT_EQ(colors.primary, theme.color.primary);
  EXPECT_EQ(colors.onPrimary.withA(20),
            theme.state.resolve(material3::ColorToken::kPrimary,
                                InteractionState::kHover));
  EXPECT_EQ(colors.onSurface.withA(31),
            theme.state.resolve(material3::ColorToken::kSurface,
                                InteractionState::kPressed));
  EXPECT_EQ(colors.primary,
            material3::MakeFrameworkTheme(theme).color.emphasis);
  EXPECT_EQ(roo_display::Color(0xFF6750A4),
            material3::DefaultMaterial3Theme().color.primary);
}

// Verifies the public factory preserves every default color and state entry.
TEST(Material3ThemeTest, FactoryMatchesStaticDefaults) {
  const material3::Material3Theme theme =
      material3::MakeTheme(material3::DefaultColorScheme());
  const material3::Material3Theme& defaults =
      material3::DefaultMaterial3Theme();
  EXPECT_EQ(defaults.state.disabledContentOpacity,
            theme.state.disabledContentOpacity);
  EXPECT_EQ(defaults.density, theme.density);
  for (uint8_t token = 0; token < 33; ++token) {
    const auto color_token = static_cast<material3::ColorToken>(token);
    EXPECT_EQ(defaults.color.resolve(color_token),
              theme.color.resolve(color_token));
    for (uint8_t state = 0; state < 6; ++state) {
      const auto interaction = static_cast<InteractionState>(state);
      EXPECT_EQ(defaults.state.resolve(color_token, interaction),
                theme.state.resolve(color_token, interaction));
    }
  }
}

TEST(ThemeTest, CanBeUsedWithoutAMaterial3Slot) {
  Theme theme = {};
  EXPECT_FALSE(theme.hasMaterial3Theme());
}

}  // namespace
}  // namespace roo_windows
