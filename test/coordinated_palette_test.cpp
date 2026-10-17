#include <cstring>

#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/palette.h"

namespace roo_windows {
namespace {

using material3::ColorScheme;
using roo_display::Color;

constexpr Color ColorScheme::* kFields[] = {
    &ColorScheme::primary,
    &ColorScheme::onPrimary,
    &ColorScheme::primaryContainer,
    &ColorScheme::onPrimaryContainer,
    &ColorScheme::secondary,
    &ColorScheme::onSecondary,
    &ColorScheme::secondaryContainer,
    &ColorScheme::onSecondaryContainer,
    &ColorScheme::tertiary,
    &ColorScheme::onTertiary,
    &ColorScheme::tertiaryContainer,
    &ColorScheme::onTertiaryContainer,
    &ColorScheme::background,
    &ColorScheme::onBackground,
    &ColorScheme::surface,
    &ColorScheme::surfaceContainerLowest,
    &ColorScheme::surfaceContainerLow,
    &ColorScheme::surfaceContainer,
    &ColorScheme::surfaceContainerHigh,
    &ColorScheme::surfaceContainerHighest,
    &ColorScheme::onSurface,
    &ColorScheme::surfaceVariant,
    &ColorScheme::onSurfaceVariant,
    &ColorScheme::error,
    &ColorScheme::onError,
    &ColorScheme::errorContainer,
    &ColorScheme::onErrorContainer,
    &ColorScheme::outline,
    &ColorScheme::outlineVariant,
    &ColorScheme::inverseSurface,
    &ColorScheme::inverseOnSurface,
    &ColorScheme::inversePrimary,
    &ColorScheme::surfaceTint,
};
static_assert(sizeof(kFields) / sizeof(kFields[0]) * sizeof(Color) ==
                  sizeof(ColorScheme),
              "Cover every palette field.");

// Supplies distinct values so an incorrect mapping cannot match by accident.
ColorScheme DistinctPalette() {
  ColorScheme colors{};
  uint32_t value = 0xFF010203;
  for (auto field : kFields) {
    colors.*field = Color(value);
    value += 0x00030507;
  }
  return colors;
}

// Verifies every M3 field and all five framework root mappings are copied.
TEST(CoordinatedPalette, CopiesMaterial3AndFrameworkColors) {
  Theme theme;
  const ColorScheme colors = DistinctPalette();
  ApplyCoordinatedMaterial3Palette(theme, colors);
  for (auto field : kFields)
    EXPECT_EQ(colors.*field, theme.material3.color.*field);
  EXPECT_EQ(colors.background, theme.framework.canvas);
  EXPECT_EQ(colors.surface, theme.framework.surface);
  EXPECT_EQ(colors.onSurface, theme.framework.content);
  EXPECT_EQ(colors.primary, theme.framework.emphasis);
  EXPECT_EQ(colors.outlineVariant, theme.framework.outline);
}

#if ROO_WINDOWS_ENABLE_MATERIAL2
// Verifies every independently owned M2 color is copied by its matching name.
TEST(CoordinatedPalette, CopiesAllMatchingMaterial2Fields) {
  Theme theme;
  const ColorScheme colors = DistinctPalette();
  ApplyCoordinatedMaterial3Palette(theme, colors);
  EXPECT_EQ(colors.primary, theme.material2.color.primary);
  EXPECT_EQ(colors.onPrimary, theme.material2.color.onPrimary);
  EXPECT_EQ(colors.onPrimaryContainer,
            theme.material2.color.onPrimaryContainer);
  EXPECT_EQ(colors.onSecondary, theme.material2.color.onSecondary);
  EXPECT_EQ(colors.onSecondaryContainer,
            theme.material2.color.onSecondaryContainer);
  EXPECT_EQ(colors.onTertiary, theme.material2.color.onTertiary);
  EXPECT_EQ(colors.onTertiaryContainer,
            theme.material2.color.onTertiaryContainer);
  EXPECT_EQ(colors.background, theme.material2.color.background);
  EXPECT_EQ(colors.onBackground, theme.material2.color.onBackground);
  EXPECT_EQ(colors.surface, theme.material2.color.surface);
  EXPECT_EQ(colors.onSurface, theme.material2.color.onSurface);
  EXPECT_EQ(colors.onSurfaceVariant, theme.material2.color.onSurfaceVariant);
  EXPECT_EQ(colors.inverseOnSurface, theme.material2.color.inverseOnSurface);
  EXPECT_EQ(colors.inversePrimary, theme.material2.color.inversePrimary);
  EXPECT_EQ(colors.onError, theme.material2.color.onError);
  EXPECT_EQ(colors.onErrorContainer, theme.material2.color.onErrorContainer);
  EXPECT_EQ(colors.outline, theme.material2.color.outline);
  static_assert(sizeof(material2::ColorScheme) == 17 * sizeof(Color),
                "Update mapping coverage when Material 2 gains a color.");
}
#endif

// Verifies copying does not retain references, and reapplying an aliased M3
// palette deliberately resynchronizes roots and optional borrowed surfaces.
TEST(CoordinatedPalette, LaterEditsStayIndependentUntilReapplied) {
  Theme theme;
  ColorScheme colors = DistinctPalette();
  ApplyCoordinatedMaterial3Palette(theme, colors);
  const Color original = colors.primary;
  colors.primary = Color(0xFFABCDEF);
  EXPECT_EQ(original, theme.material3.color.primary);
  theme.material3.color.primary = Color(0xFF123456);
  EXPECT_EQ(original, theme.framework.emphasis);
#if ROO_WINDOWS_ENABLE_MATERIAL2
  EXPECT_EQ(original, theme.material2.color.primary);
  theme.material2.color.primary = Color(0xFF654321);
  EXPECT_EQ(Color(0xFF123456), theme.material3.color.primary);
#endif
  ApplyCoordinatedMaterial3Palette(theme, theme.material3.color);
  EXPECT_EQ(Color(0xFF123456), theme.framework.emphasis);
#if ROO_WINDOWS_ENABLE_MATERIAL2
  EXPECT_EQ(Color(0xFF123456), theme.material2.color.primary);
#endif
}

// Verifies palette installation leaves all non-color theme data untouched.
TEST(CoordinatedPalette, PreservesInteractionAndComponentSettings) {
  Theme theme;
  theme.framework.state = {3, 7, 11, 19, 37};
  theme.framework.click.durationMillis = 731;
  theme.framework.click.reveal = ClickOverlayAnimation::kFade;
  theme.material3.density = material3::Density::kMinus2;
  theme.material3.components.button.elevatedContainer =
      material3::ColorToken::kErrorContainer;
  unsigned char saved[sizeof(theme.material3.components)];
  std::memcpy(saved, &theme.material3.components, sizeof(saved));
  ApplyCoordinatedMaterial3Palette(theme, DistinctPalette());
  EXPECT_EQ(3, theme.framework.state.hover);
  EXPECT_EQ(7, theme.framework.state.focus);
  EXPECT_EQ(11, theme.framework.state.pressed);
  EXPECT_EQ(19, theme.framework.state.dragged);
  EXPECT_EQ(37, theme.framework.state.disabledContentOpacity);
  EXPECT_EQ(731u, theme.framework.click.durationMillis);
  EXPECT_EQ(ClickOverlayAnimation::kFade, theme.framework.click.reveal);
  EXPECT_EQ(material3::Density::kMinus2, theme.material3.density);
  EXPECT_EQ(0, std::memcmp(saved, &theme.material3.components, sizeof(saved)));
}

}  // namespace
}  // namespace roo_windows
