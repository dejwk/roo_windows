#include "roo_windows/material3/default_theme.h"

#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {

// Construction-only representation of the historical M3 opacity matrix.
// It is not part of the framework Theme API or retained in application state.
struct LegacyStateOpacity {
  uint8_t disabled;
  uint8_t hoverOnPrimary, hoverOnSecondary, hoverOnBackground, hoverOnSurface,
      hoverOnError;
  uint8_t focusOnPrimary, focusOnSecondary, focusOnBackground, focusOnSurface,
      focusOnError;
  uint8_t selectedOnPrimary, selectedOnSecondary, selectedOnBackground,
      selectedOnSurface, selectedOnError;
  uint8_t activatedOnPrimary, activatedOnSecondary, activatedOnBackground,
      activatedOnSurface, activatedOnError;
  uint8_t pressedOnPrimary, pressedOnSecondary, pressedOnBackground,
      pressedOnSurface, pressedOnError;
  uint8_t draggedOnPrimary, draggedOnSecondary, draggedOnBackground,
      draggedOnSurface, draggedOnError;
};

constexpr ColorScheme DefaultMaterial3Colors() {
  return {
      .primary = roo_display::Color(0xFF6750A4),
      .onPrimary = roo_display::Color(0xFFFFFFFF),
      .primaryContainer = roo_display::Color(0xFFEADDFF),
      .onPrimaryContainer = roo_display::Color(0xFF4F378B),
      .secondary = roo_display::Color(0xFF625B71),
      .onSecondary = roo_display::Color(0xFFFFFFFF),
      .secondaryContainer = roo_display::Color(0xFFE8DEF8),
      .onSecondaryContainer = roo_display::Color(0xFF4A4458),
      .tertiary = roo_display::Color(0xFF7D5260),
      .onTertiary = roo_display::Color(0xFFFFFFFF),
      .tertiaryContainer = roo_display::Color(0xFFFFD8E4),
      .onTertiaryContainer = roo_display::Color(0xFF633B48),
      .background = roo_display::Color(0xFFFFFBFE),
      .onBackground = roo_display::Color(0xFF1C1B1F),
      .surface = roo_display::Color(0xFFFEF7FF),
      .surfaceContainerLowest = roo_display::Color(0xFFFFFFFF),
      .surfaceContainerLow = roo_display::Color(0xFFF7F2FA),
      .surfaceContainer = roo_display::Color(0xFFF3EDF7),
      .surfaceContainerHigh = roo_display::Color(0xFFECE6F0),
      .surfaceContainerHighest = roo_display::Color(0xFFE6E0E9),
      .onSurface = roo_display::Color(0xFF1D1B20),
      .surfaceVariant = roo_display::Color(0xFFE7E0EC),
      .onSurfaceVariant = roo_display::Color(0xFF49454F),
      .error = roo_display::Color(0xFFB3261E),
      .onError = roo_display::Color(0xFFFFFFFF),
      .errorContainer = roo_display::Color(0xFFF9DEDC),
      .onErrorContainer = roo_display::Color(0xFF8C1D18),
      .outline = roo_display::Color(0xFF79747E),
      .outlineVariant = roo_display::Color(0xFFCAC4D0),
      .inverseSurface = roo_display::Color(0xFF322F35),
      .inverseOnSurface = roo_display::Color(0xFFF5EFF7),
      .inversePrimary = roo_display::Color(0xFFD0BCFF),
      .surfaceTint = roo_display::Color(0xFF6750A4),
  };
}

constexpr LegacyStateOpacity DefaultLegacyState() {
  return {
      .disabled = 49,
      .hoverOnPrimary = 20,
      .hoverOnSecondary = 20,
      .hoverOnBackground = 10,
      .hoverOnSurface = 10,
      .hoverOnError = 20,
      .focusOnPrimary = 61,
      .focusOnSecondary = 61,
      .focusOnBackground = 31,
      .focusOnSurface = 31,
      .focusOnError = 61,
      .selectedOnPrimary = 41,
      .selectedOnSecondary = 41,
      .selectedOnBackground = 20,
      .selectedOnSurface = 20,
      .selectedOnError = 41,
      .activatedOnPrimary = 61,
      .activatedOnSecondary = 61,
      .activatedOnBackground = 31,
      .activatedOnSurface = 31,
      .activatedOnError = 61,
      .pressedOnPrimary = 61,
      .pressedOnSecondary = 61,
      .pressedOnBackground = 31,
      .pressedOnSurface = 31,
      .pressedOnError = 61,
      .draggedOnPrimary = 41,
      .draggedOnSecondary = 41,
      .draggedOnBackground = 20,
      .draggedOnSurface = 20,
      .draggedOnError = 41,
  };
}

constexpr uint8_t StateOpacity(const LegacyStateOpacity& state,
                               ColorToken token, InteractionState interaction) {
  const bool primary =
      token == ColorToken::kPrimary || token == ColorToken::kPrimaryContainer;
  const bool secondary = token == ColorToken::kSecondary ||
                         token == ColorToken::kSecondaryContainer ||
                         token == ColorToken::kTertiary ||
                         token == ColorToken::kTertiaryContainer;
  const bool error =
      token == ColorToken::kError || token == ColorToken::kErrorContainer;
  const bool surface = token == ColorToken::kSurface ||
                       token == ColorToken::kSurfaceContainerLowest ||
                       token == ColorToken::kSurfaceContainerLow ||
                       token == ColorToken::kSurfaceContainer ||
                       token == ColorToken::kSurfaceContainerHigh ||
                       token == ColorToken::kSurfaceContainerHighest ||
                       token == ColorToken::kSurfaceVariant ||
                       token == ColorToken::kInverseSurface;
  switch (interaction) {
    case InteractionState::kHover:
      return primary     ? state.hoverOnPrimary
             : secondary ? state.hoverOnSecondary
             : error     ? state.hoverOnError
             : surface   ? state.hoverOnSurface
                         : state.hoverOnBackground;
    case InteractionState::kFocus:
      return primary     ? state.focusOnPrimary
             : secondary ? state.focusOnSecondary
             : error     ? state.focusOnError
             : surface   ? state.focusOnSurface
                         : state.focusOnBackground;
    case InteractionState::kSelected:
      return primary     ? state.selectedOnPrimary
             : secondary ? state.selectedOnSecondary
             : error     ? state.selectedOnError
             : surface   ? state.selectedOnSurface
                         : state.selectedOnBackground;
    case InteractionState::kActivated:
      return primary     ? state.activatedOnPrimary
             : secondary ? state.activatedOnSecondary
             : error     ? state.activatedOnError
             : surface   ? state.activatedOnSurface
                         : state.activatedOnBackground;
    case InteractionState::kPressed:
      return primary     ? state.pressedOnPrimary
             : secondary ? state.pressedOnSecondary
             : error     ? state.pressedOnError
             : surface   ? state.pressedOnSurface
                         : state.pressedOnBackground;
    case InteractionState::kDragged:
      return primary     ? state.draggedOnPrimary
             : secondary ? state.draggedOnSecondary
             : error     ? state.draggedOnError
             : surface   ? state.draggedOnSurface
                         : state.draggedOnBackground;
  }
  return state.pressedOnBackground;
}

// Shared rules retain the historical opacity policy without storing palette
// RGB.
constexpr StateLayerRules BuildStateLayerRules() {
  StateLayerRules result{};
  const LegacyStateOpacity legacy_state = DefaultLegacyState();
  for (uint8_t token = 0; token < 33; ++token) {
    for (uint8_t interaction = 0; interaction < 6; ++interaction) {
      const auto color_token = static_cast<ColorToken>(token);
      const auto state = static_cast<InteractionState>(interaction);
      result.layer[token][interaction] = {
          ContentColorTokenFor(color_token),
          StateOpacity(legacy_state, color_token, state)};
    }
  }
  return result;
}

constexpr StateLayerRules kDefaultStateLayerRules = BuildStateLayerRules();

}  // namespace

ColorScheme DefaultColorScheme() { return DefaultMaterial3Colors(); }

StateLayerTheme DefaultStateLayerTheme() {
  return {.disabledContentOpacity = DefaultLegacyState().disabled,
          .rules = &kDefaultStateLayerRules};
}

}  // namespace roo_windows::material3
