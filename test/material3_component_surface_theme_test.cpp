#include <array>

#include "gtest/gtest.h"
#include "roo_display/color/color_modes.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {

// One entry for every ComponentTheme byte. Keeping the fixture as role output
// makes the shared storage policy testable without duplicating widget paint
// logic already covered by each component's focused rendering suite.
template <typename Components>
auto ComponentSurfaceSlots(Components& components) {
  using SlotPointer = decltype(&components.layoutScaffold.container);
  return std::array<SlotPointer, 27>{
      &components.layoutScaffold.container,
      &components.appBar.flatContainer,
      &components.appBar.scrolledContainer,
      &components.searchBar.container,
      &components.searchAppBar.flatContainer,
      &components.searchAppBar.scrolledContainer,
      &components.searchAppBar.flatSearchContainer,
      &components.searchAppBar.scrolledSearchContainer,
      &components.card.elevatedContainer,
      &components.card.filledContainer,
      &components.card.outlinedContainer,
      &components.dialog.basicContainer,
      &components.dialog.fullScreenContainer,
      &components.datePicker.container,
      &components.navigationBar.container,
      &components.navigationRail.collapsedContainer,
      &components.navigationRail.expandedContainer,
      &components.tabs.primaryContainer,
      &components.tabs.secondaryContainer,
      &components.list.standardContainer,
      &components.list.segmentedContainer,
      &components.menu.baselineContainer,
      &components.menu.expressiveContainer,
      &components.textField.filledContainer,
      &components.button.elevatedContainer,
      &components.toggleIconButton.filledUnselectedContainer,
      &components.switchControl.unselectedTrack,
  };
}

void SetDistinctNeutralSurfacePalette(Material3Theme& material, uint8_t base) {
  material.color.surface =
      roo_display::Color(255, base + 1, base + 2, base + 3);
  material.color.surfaceContainerLowest =
      roo_display::Color(255, base + 11, base + 12, base + 13);
  material.color.surfaceContainerLow =
      roo_display::Color(255, base + 21, base + 22, base + 23);
  material.color.surfaceContainer =
      roo_display::Color(255, base + 31, base + 32, base + 33);
  material.color.surfaceContainerHigh =
      roo_display::Color(255, base + 41, base + 42, base + 43);
  material.color.surfaceContainerHighest =
      roo_display::Color(255, base + 51, base + 52, base + 53);
}

roo_display::Color Rgb565Output(const ColorScheme& colors, ColorToken role) {
  roo_display::Rgb565 mode;
  const roo_display::Color color = colors.resolve(role);
  return mode.toArgbColor(mode.fromArgbColor(color));
}

void ExpectRgb565SurfaceOutput(const Material3Theme& material,
                               const std::array<ColorToken, 27>& expected) {
  const auto slots = ComponentSurfaceSlots(material.components);
  for (size_t i = 0; i < slots.size(); ++i) {
    const ColorToken* slot = slots[i];
    ASSERT_NE(nullptr, slot);
    EXPECT_EQ(expected[i], *slot);
    // This is the color that reaches an RGB565 display after the component
    // slot selects a palette role.
    EXPECT_EQ(Rgb565Output(material.color, expected[i]),
              Rgb565Output(material.color, *slot));
  }
}

TEST(Material3ComponentSurfaceThemeTest,
     RendersDefaultAndCustomizedLightDarkRoleOutputsInRgb565) {
  Material3Theme light = DefaultTheme().material3Theme();
  Material3Theme dark = DefaultTheme().material3Theme();
  SetDistinctNeutralSurfacePalette(light, 10);
  SetDistinctNeutralSurfacePalette(dark, 80);
  constexpr std::array<ColorToken, 27> kDefaultRoles = {
      ColorToken::kSurfaceContainerLowest,
      ColorToken::kSurface,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurfaceContainerHigh,
      ColorToken::kSurface,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurfaceContainerHighest,
      ColorToken::kSurfaceContainerLow,
      ColorToken::kSurfaceContainerHighest,
      ColorToken::kSurface,
      ColorToken::kSurfaceContainerHigh,
      ColorToken::kSurface,
      ColorToken::kSurfaceContainerHigh,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurface,
      ColorToken::kSurface,
      ColorToken::kSurface,
      ColorToken::kSurface,
      ColorToken::kSurface,
      ColorToken::kSurface,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurfaceContainerLow,
      ColorToken::kSurfaceContainerHighest,
      ColorToken::kSurfaceContainerLow,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurfaceContainerHighest,
  };

  // Default roles are exercised against independently colored light and dark
  // palettes. The fixture deliberately quantizes at the RGB565 output edge.
  ExpectRgb565SurfaceOutput(light, kDefaultRoles);
  ExpectRgb565SurfaceOutput(dark, kDefaultRoles);

  constexpr std::array<ColorToken, 6> kNeutralRoles = {
      ColorToken::kSurface,
      ColorToken::kSurfaceContainerLowest,
      ColorToken::kSurfaceContainerLow,
      ColorToken::kSurfaceContainer,
      ColorToken::kSurfaceContainerHigh,
      ColorToken::kSurfaceContainerHighest,
  };
  std::array<ColorToken, 27> light_roles;
  std::array<ColorToken, 27> dark_roles;
  size_t index = 0;
  for (ColorToken* slot : ComponentSurfaceSlots(light.components)) {
    *slot = light_roles[index] = kNeutralRoles[index % kNeutralRoles.size()];
    ++index;
  }
  index = 0;
  for (ColorToken* slot : ComponentSurfaceSlots(dark.components)) {
    *slot = dark_roles[index] =
        kNeutralRoles[(index + 3) % kNeutralRoles.size()];
    ++index;
  }

  ExpectRgb565SurfaceOutput(light, light_roles);
  ExpectRgb565SurfaceOutput(dark, dark_roles);
}

}  // namespace
}  // namespace roo_windows::material3
