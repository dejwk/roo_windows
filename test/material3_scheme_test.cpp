#include <cmath>
#include <cstring>

#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/palette.h"
#include "roo_windows/material3/schemes/catalog.h"

namespace roo_windows::material3::schemes {
namespace {

using roo_display::Color;

// Computes WCAG relative luminance from resolved sRGB, without alpha blending.
double Luminance(Color color) {
  const auto linear = [](uint8_t channel) {
    const double value = channel / 255.0;
    return value <= 0.04045 ? value / 12.92
                            : std::pow((value + 0.055) / 1.055, 2.4);
  };
  return 0.2126 * linear(color.r()) + 0.7152 * linear(color.g()) +
         0.0722 * linear(color.b());
}

// Verifies a foreground/background pair meets normal-text contrast.
void ExpectReadable(Color foreground, Color background) {
  const double a = Luminance(foreground);
  const double b = Luminance(background);
  EXPECT_GE((std::fmax(a, b) + 0.05) / (std::fmin(a, b) + 0.05), 4.5);
}

// Verifies imported schemes have complete opaque roles and readable semantic
// pairs in both modes, including the neutral surface-container levels.
TEST(Material3SchemeTest, EveryPresetHasCompleteReadableLightAndDarkPalettes) {
  ASSERT_EQ(67u, kPresetCount);
  for (const SchemePreset& preset : kPresets) {
    SCOPED_TRACE(preset.id);
    const ColorSchemePair& pair = preset.colors();
    for (const ColorScheme* colors : {&pair.light, &pair.dark}) {
      SCOPED_TRACE(colors == &pair.light ? "light" : "dark");
      // Every concrete token is contiguous from kPrimary to kSurfaceTint.
      for (uint8_t token = 0;
           token <= static_cast<uint8_t>(ColorToken::kSurfaceTint); ++token) {
        EXPECT_EQ(255, colors->resolve(static_cast<ColorToken>(token)).a());
      }
      ExpectReadable(colors->onPrimary, colors->primary);
      ExpectReadable(colors->onPrimaryContainer, colors->primaryContainer);
      ExpectReadable(colors->onSecondary, colors->secondary);
      ExpectReadable(colors->onSecondaryContainer, colors->secondaryContainer);
      ExpectReadable(colors->onTertiary, colors->tertiary);
      ExpectReadable(colors->onTertiaryContainer, colors->tertiaryContainer);
      ExpectReadable(colors->onError, colors->error);
      ExpectReadable(colors->onErrorContainer, colors->errorContainer);
      ExpectReadable(colors->onBackground, colors->background);
      ExpectReadable(colors->onSurface, colors->surface);
      ExpectReadable(colors->onSurfaceVariant, colors->surfaceVariant);
      ExpectReadable(colors->inverseOnSurface, colors->inverseSurface);
      ExpectReadable(colors->onSurface, colors->surfaceContainerLowest);
      ExpectReadable(colors->onSurface, colors->surfaceContainerLow);
      ExpectReadable(colors->onSurface, colors->surfaceContainer);
      ExpectReadable(colors->onSurface, colors->surfaceContainerHigh);
      ExpectReadable(colors->onSurface, colors->surfaceContainerHighest);
    }
    EXPECT_GT(Luminance(pair.light.surface), Luminance(pair.dark.surface));
  }
}

// Verifies persisted IDs identify the intended imported data, independently of
// the catalog order. Anchors come from the pinned upstream export.
TEST(Material3SchemeTest, PreservesOriginalPresetIdentities) {
  struct Expected {
    const char* id;
    uint32_t light;
    uint32_t dark;
  };
  const Expected expected[] = {
      {"indigo-san-marino", 0xFF515B92, 0xFFBAC3FF},
      {"blue-stone-teal", 0xFF006A60, 0xFF82D5C8},
      {"camarone-green", 0xFF3B6939, 0xFFA1D39A},
      {"yukon-gold-yellow", 0xFF685F12, 0xFFD5C871},
      {"rust-deep-orange", 0xFF8F4B39, 0xFFFFB5A1},
      {"eggplant-purple", 0xFF7B4E7F, 0xFFEBB5ED},
  };
  ASSERT_GE(kPresetCount, sizeof(expected) / sizeof(expected[0]));
  for (const Expected& entry : expected) {
    size_t matches = 0;
    for (const SchemePreset& preset : kPresets) {
      if (std::strcmp(entry.id, preset.id) != 0) continue;
      ++matches;
      EXPECT_NE('\0', preset.name[0]);
      ASSERT_NE(nullptr, preset.colors);
      EXPECT_EQ(Color(entry.light), preset.colors().light.primary);
      EXPECT_EQ(Color(entry.dark), preset.colors().dark.primary);
    }
    EXPECT_EQ(1u, matches) << entry.id;
  }
}

// Verifies presets integrate with independent ownership and coordinated copies
// while preserving application interaction policy, density and component roles.
TEST(Material3SchemeTest, InstallsWithoutResettingApplicationSettings) {
  Theme theme;
  theme.framework.state.pressed = 17;
  theme.framework.click.durationMillis = 731;
  theme.material3.density = Density::kMinus2;
  theme.material3.components.button.elevatedContainer =
      ColorToken::kErrorContainer;
  for (const SchemePreset& preset : kPresets) {
    const ColorSchemePair& pair = preset.colors();
    for (const ColorScheme* colors : {&pair.light, &pair.dark}) {
      const Color previous_emphasis = theme.framework.emphasis;
      theme.material3.color = *colors;
      EXPECT_EQ(previous_emphasis, theme.framework.emphasis);
      ApplyCoordinatedMaterial3Palette(theme, *colors);
      EXPECT_EQ(colors->background, theme.framework.canvas);
      EXPECT_EQ(colors->surface, theme.framework.surface);
      EXPECT_EQ(colors->onSurface, theme.framework.content);
      EXPECT_EQ(colors->primary, theme.framework.emphasis);
      EXPECT_EQ(colors->outlineVariant, theme.framework.outline);
#if ROO_WINDOWS_ENABLE_MATERIAL2
      EXPECT_EQ(colors->primary, theme.material2.color.primary);
      EXPECT_EQ(colors->onPrimary, theme.material2.color.onPrimary);
#endif
      EXPECT_EQ(17, theme.framework.state.pressed);
      EXPECT_EQ(731u, theme.framework.click.durationMillis);
      EXPECT_EQ(Density::kMinus2, theme.material3.density);
      EXPECT_EQ(ColorToken::kErrorContainer,
                theme.material3.components.button.elevatedContainer);
      theme.material3.color.primary = Color(0xFF123456);
      EXPECT_NE(theme.material3.color.primary, colors->primary);
      EXPECT_EQ(colors->primary, theme.framework.emphasis);
    }
  }
}

}  // namespace
}  // namespace roo_windows::material3::schemes
