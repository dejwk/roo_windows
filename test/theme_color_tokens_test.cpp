#include <cstddef>

#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {
namespace {

static_assert(sizeof(FrameworkStateTheme) == 5);
static_assert(sizeof(Theme) >= sizeof(FrameworkTheme) +
                                   sizeof(material3::Material3Theme) +
                                   sizeof(material2::Material2Theme));

// Verifies the five standalone root colors and shared opacity defaults.
TEST(FrameworkThemeTest, FiveRootsMatchDocumentedDefaults) {
  const Theme theme;
  const auto& colors = theme.material3Theme().color;
  EXPECT_EQ(colors.background,
            theme.frameworkColor(FrameworkColorRole::kCanvas));
  EXPECT_EQ(colors.surface, theme.frameworkColor(FrameworkColorRole::kSurface));
  EXPECT_EQ(colors.onSurface,
            theme.frameworkColor(FrameworkColorRole::kContent));
  EXPECT_EQ(colors.primary,
            theme.frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(colors.outlineVariant,
            theme.frameworkColor(FrameworkColorRole::kOutline));
  const uint8_t expected[] = {20, 26, 0, 0, 26, 41};
  for (unsigned i = 0; i < 6; ++i) {
    EXPECT_EQ(expected[i],
              theme.framework.state.opacity(static_cast<InteractionState>(i)));
  }
  EXPECT_EQ(49, theme.disabledContentOpacity());
}

// Verifies that live framework edits and copied themes own separate values.
TEST(FrameworkThemeTest, LiveRootsAndOpacityAreIndependentOfSystemPalettes) {
  Theme original;
  Theme theme = original;
  theme.framework.content = roo_display::Color(0xFF123456);
  theme.framework.canvas = roo_display::Color(1);
  theme.framework.surface = roo_display::Color(2);
  theme.framework.emphasis = roo_display::Color(4);
  theme.framework.outline = roo_display::Color(5);
  theme.framework.state.hover = 73;
  theme.framework.state.disabledContentOpacity = 64;
  theme.material3Theme().color.onSurface = roo_display::Color(0xFFABCDEF);
  theme.material2Theme().color.onSurface = roo_display::Color(0xFF987654);
  EXPECT_EQ(roo_display::Color(0xFF123456),
            theme.frameworkColor(FrameworkColorRole::kContent));
  EXPECT_EQ(roo_display::Color(1),
            theme.frameworkColor(FrameworkColorRole::kCanvas));
  EXPECT_EQ(roo_display::Color(2),
            theme.frameworkColor(FrameworkColorRole::kSurface));
  EXPECT_EQ(roo_display::Color(4),
            theme.frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(roo_display::Color(5),
            theme.frameworkColor(FrameworkColorRole::kOutline));
  EXPECT_EQ(73, theme.framework.state.opacity(InteractionState::kHover));
  EXPECT_EQ(64, theme.disabledContentOpacity());
  EXPECT_EQ(roo_display::Color(0xFF1D1B20),
            original.frameworkColor(FrameworkColorRole::kContent));
  EXPECT_EQ(20, original.framework.state.opacity(InteractionState::kHover));
}

// Verifies the Material 3 palette still resolves every semantic token.
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

// Verifies unchanged component role defaults after state storage moves out.
TEST(Material3ThemeTest, InitializesCompactComponentSurfaceDefaults) {
  const material3::Material3Theme material;
  EXPECT_EQ(static_cast<size_t>(3), sizeof(material3::CardTheme));
  EXPECT_EQ(static_cast<size_t>(1), sizeof(material3::ToolbarTheme));
  EXPECT_EQ(static_cast<size_t>(29), sizeof(material3::ComponentTheme));
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

// Verifies an application default and a fresh Material 3 theme share colors.
TEST(Material3ThemeTest, DefaultConstructionMatchesSharedPalette) {
  const material3::Material3Theme theme;
  const auto& defaults = DefaultTheme().material3Theme();
  EXPECT_EQ(defaults.density, theme.density);
  for (uint8_t token = 0; token < 33; ++token) {
    const auto color_token = static_cast<material3::ColorToken>(token);
    EXPECT_EQ(defaults.color.resolve(color_token),
              theme.color.resolve(color_token));
  }
}

}  // namespace
}  // namespace roo_windows
