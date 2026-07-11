#include "gtest/gtest.h"

#include "roo_windows/core/framework_theme.h"
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
      roo_display::Color(1), roo_display::Color(2), roo_display::Color(3),
      roo_display::Color(4), roo_display::Color(5), roo_display::Color(6),
      roo_display::Color(7), roo_display::Color(8), roo_display::Color(9),
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
      EXPECT_EQ(color, state.resolve(static_cast<material3::ColorToken>(token),
                                     static_cast<InteractionState>(interaction)));
    }
  }
}

TEST(Material3ThemeTest, MakesTheDocumentedFrameworkMapping) {
  material3::Theme material = {};
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

}  // namespace
}  // namespace roo_windows
