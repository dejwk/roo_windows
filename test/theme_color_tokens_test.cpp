#include <cstddef>

#include "gtest/gtest.h"
#include "roo_windows/core/framework_theme.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/default_theme.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows {
namespace {

static_assert(sizeof(Theme) >= sizeof(material3::Material3Theme) +
                                   sizeof(material2::Material2Theme),
              "Application themes own independent system settings.");

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

// Verifies the official state opacities are independent of every container
// token and selection/activation do not introduce generic state layers.
TEST(Material3StateLayerThemeTest, UsesSystemStateOpacities) {
  material3::Material3Theme theme;
  const uint8_t expected[] = {20, 26, 0, 0, 26, 41};
  static_assert(sizeof(material3::StateLayerTheme) == 5);
  for (uint8_t token = 0; token < 33; ++token) {
    const auto container = static_cast<material3::ColorToken>(token);
    for (uint8_t i = 0; i < 6; ++i) {
      const auto state = static_cast<InteractionState>(i);
      EXPECT_EQ(expected[i], theme.stateLayer(container, state).a());
    }
  }
  EXPECT_EQ(49, theme.state.disabledContentOpacity);
}

// Verifies palette and opacity edits are live, copies own their settings,
// and zero opacity disables a layer without borrowed override storage.
TEST(Material3StateLayerThemeTest, ResolvesLivePaletteAndOwnedOpacities) {
  material3::Material3Theme original;
  material3::Material3Theme theme = original;
  theme.color.onPrimary = roo_display::Color(0xFF654321);
  theme.state.hover = 73;
  theme.state.pressed = 0;
  EXPECT_EQ(theme.color.onPrimary.withA(73),
            theme.stateLayer(material3::ColorToken::kPrimary,
                             InteractionState::kHover));
  EXPECT_EQ(theme.color.onSurface.withA(73),
            theme.stateLayer(material3::ColorToken::kSurface,
                             InteractionState::kHover));
  EXPECT_EQ(roo_display::color::Transparent,
            theme.stateLayer(material3::ColorToken::kPrimary,
                             InteractionState::kPressed));
  EXPECT_EQ(
      roo_display::color::Transparent,
      theme.stateLayer(material3::ColorToken::kNone, InteractionState::kHover));
  EXPECT_EQ(20, original.state.hover);
  EXPECT_EQ(26, original.state.pressed);
}

// Verifies aggregate Material themes retain their existing color/state
// initialization while adding all compact component-surface defaults.
TEST(Material3ThemeTest, InitializesCompactComponentSurfaceDefaults) {
  const material3::Material3Theme material = {};

  EXPECT_EQ(static_cast<size_t>(3), sizeof(material3::CardTheme));
  EXPECT_EQ(static_cast<size_t>(1), sizeof(material3::ToolbarTheme));
  EXPECT_EQ(static_cast<size_t>(29), sizeof(material3::ComponentTheme));
  // Each theme owns its compact palette, state, and component settings.
  EXPECT_LE(sizeof(material3::Material3Theme), static_cast<size_t>(200));
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
  material.state.pressed = 87;

  const Theme framework{.material3 = material};
  EXPECT_EQ(roo_display::Color(1),
            framework.frameworkColor(FrameworkColorRole::kCanvas));
  EXPECT_EQ(roo_display::Color(2),
            framework.frameworkColor(FrameworkColorRole::kSurface));
  EXPECT_EQ(roo_display::Color(3),
            framework.frameworkColor(FrameworkColorRole::kContent));
  EXPECT_EQ(roo_display::Color(4),
            framework.frameworkColor(FrameworkColorRole::kMutedContent));
  EXPECT_EQ(roo_display::Color(5),
            framework.frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(roo_display::Color(6),
            framework.frameworkColor(FrameworkColorRole::kOutline));
  EXPECT_EQ(roo_display::Color(7),
            framework.frameworkColor(FrameworkColorRole::kCritical));
  EXPECT_EQ(roo_display::Color(8),
            framework.frameworkColor(FrameworkColorRole::kOnCritical));
  EXPECT_EQ(49, framework.disabledContentOpacity());
  EXPECT_EQ(material.color.onPrimary.withA(87),
            framework.frameworkStateLayer(FrameworkColorRole::kEmphasis,
                                          InteractionState::kPressed));
}

// Verifies mutable and const accessors refer to the same owned settings and
// changes made through the accessor immediately affect framework colors.
TEST(ThemeTest, Material3AccessorBorrowsOwnedSettings) {
  Theme theme;
  const Theme& const_theme = theme;
  EXPECT_EQ(&theme.material3, &theme.material3Theme());
  EXPECT_EQ(&theme.material3Theme(), &const_theme.material3Theme());
  theme.material3Theme().color.primary = roo_display::Color(0xFF123456);
  EXPECT_EQ(roo_display::Color(0xFF123456),
            const_theme.frameworkColor(FrameworkColorRole::kEmphasis));
}

TEST(ThemeTest, DefaultThemeComposesMaterial3AndFrameworkThemes) {
  const Theme& theme = DefaultTheme();

  const material3::Material3Theme& material = theme.material3Theme();
  EXPECT_EQ(material.color.background,
            theme.frameworkColor(FrameworkColorRole::kCanvas));
  EXPECT_EQ(material.color.surface,
            theme.frameworkColor(FrameworkColorRole::kSurface));
  EXPECT_EQ(material.color.onSurface,
            theme.frameworkColor(FrameworkColorRole::kContent));
  EXPECT_EQ(material.color.onSurfaceVariant,
            theme.frameworkColor(FrameworkColorRole::kMutedContent));
  EXPECT_EQ(material.color.primary,
            theme.frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(material.color.outlineVariant,
            theme.frameworkColor(FrameworkColorRole::kOutline));
  EXPECT_EQ(material.color.error,
            theme.frameworkColor(FrameworkColorRole::kCritical));
  EXPECT_EQ(material.color.onError,
            theme.frameworkColor(FrameworkColorRole::kOnCritical));

  EXPECT_EQ(material.color.onPrimary.withA(20),
            material.stateLayer(material3::ColorToken::kPrimary,
                                InteractionState::kHover));
  EXPECT_EQ(material.color.onSurface.withA(26),
            material.stateLayer(material3::ColorToken::kSurface,
                                InteractionState::kPressed));
}

// Verifies copying the application theme owns an independent palette while
// owning state settings, and framework lookups never retain stale colors.
TEST(ThemeTest, OwnsIndependentPaletteAndResolvesFrameworkLive) {
  Theme original;
  Theme copy = original;
  copy.material3Theme().color.primary = roo_display::Color(0xFF123456);
  copy.material3Theme().color.onPrimary = roo_display::Color(0xFFABCDEF);
  EXPECT_NE(original.material3Theme().color.primary,
            copy.material3Theme().color.primary);
  copy.material3Theme().state.pressed = 63;
  EXPECT_EQ(26, original.material3Theme().state.pressed);
  EXPECT_EQ(copy.material3Theme().color.primary,
            copy.frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(copy.material3Theme().color.onPrimary.withA(20),
            copy.frameworkStateLayer(FrameworkColorRole::kEmphasis,
                                     InteractionState::kHover));
}

// Verifies construction with a custom palette derives interaction colors
// without modifying shared defaults, and projects into the framework theme.
TEST(Material3ThemeTest, BuildsStateLayersFromCustomPalette) {
  material3::ColorScheme colors = material3::DefaultColorScheme();
  colors.primary = roo_display::Color(0xFF123456);
  colors.onPrimary = roo_display::Color(0xFFABCDEF);
  colors.onSurface = roo_display::Color(0xFF654321);
  const material3::Material3Theme theme =
      material3::Material3Theme{.color = colors};
  EXPECT_EQ(colors.primary, theme.color.primary);
  EXPECT_EQ(colors.onPrimary.withA(20),
            theme.stateLayer(material3::ColorToken::kPrimary,
                             InteractionState::kHover));
  EXPECT_EQ(colors.onSurface.withA(26),
            theme.stateLayer(material3::ColorToken::kSurface,
                             InteractionState::kPressed));
  EXPECT_EQ(colors.primary,
            theme.frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(roo_display::Color(0xFF6750A4),
            DefaultTheme().material3Theme().color.primary);
}

// Verifies the public factory preserves every default color and state entry.
TEST(Material3ThemeTest, DefaultConstructionMatchesSharedDefaults) {
  const material3::Material3Theme theme;
  const material3::Material3Theme& defaults = DefaultTheme().material3Theme();
  EXPECT_EQ(defaults.state.disabledContentOpacity,
            theme.state.disabledContentOpacity);
  EXPECT_EQ(defaults.density, theme.density);
  for (uint8_t token = 0; token < 33; ++token) {
    const auto color_token = static_cast<material3::ColorToken>(token);
    EXPECT_EQ(defaults.color.resolve(color_token),
              theme.color.resolve(color_token));
    for (uint8_t state = 0; state < 6; ++state) {
      const auto interaction = static_cast<InteractionState>(state);
      EXPECT_EQ(defaults.stateLayer(color_token, interaction),
                theme.stateLayer(color_token, interaction));
    }
  }
}

}  // namespace
}  // namespace roo_windows
