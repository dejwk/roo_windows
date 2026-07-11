#include "theme.h"

#include <assert.h>

#include "roo_display/color/color.h"
#include "roo_fonts/NotoSans_CondensedBold/15.h"
#include "roo_fonts/NotoSans_Regular/12.h"
#include "roo_fonts/NotoSans_Regular/15.h"
#include "roo_fonts/NotoSans_Regular/18.h"
#include "roo_fonts/NotoSans_Regular/27.h"
#include "roo_fonts/NotoSans_Regular/40.h"
#include "roo_fonts/NotoSans_Regular/60.h"
#include "roo_fonts/NotoSans_Regular/90.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows {
namespace {

constexpr material3::ColorScheme DefaultMaterial3Colors() {
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

constexpr StateOpacityTheme DefaultLegacyState() {
  return {
      .disabled = 49,
      .hoverOnPrimary = 20, .hoverOnSecondary = 20,
      .hoverOnBackground = 10, .hoverOnSurface = 10, .hoverOnError = 20,
      .focusOnPrimary = 61, .focusOnSecondary = 61,
      .focusOnBackground = 31, .focusOnSurface = 31, .focusOnError = 61,
      .selectedOnPrimary = 41, .selectedOnSecondary = 41,
      .selectedOnBackground = 20, .selectedOnSurface = 20,
      .selectedOnError = 41,
      .activatedOnPrimary = 61, .activatedOnSecondary = 61,
      .activatedOnBackground = 31, .activatedOnSurface = 31,
      .activatedOnError = 61,
      .pressedOnPrimary = 61, .pressedOnSecondary = 61,
      .pressedOnBackground = 31, .pressedOnSurface = 31,
      .pressedOnError = 61,
      .draggedOnPrimary = 41, .draggedOnSecondary = 41,
      .draggedOnBackground = 20, .draggedOnSurface = 20,
      .draggedOnError = 41,
  };
}

constexpr uint8_t StateOpacity(const StateOpacityTheme& state,
                               material3::ColorToken token,
                               InteractionState interaction) {
  const bool primary = token == material3::ColorToken::kPrimary ||
                       token == material3::ColorToken::kPrimaryContainer;
  const bool secondary = token == material3::ColorToken::kSecondary ||
                         token == material3::ColorToken::kSecondaryContainer ||
                         token == material3::ColorToken::kTertiary ||
                         token == material3::ColorToken::kTertiaryContainer;
  const bool error = token == material3::ColorToken::kError ||
                     token == material3::ColorToken::kErrorContainer;
  const bool surface = token == material3::ColorToken::kSurface ||
                       token == material3::ColorToken::kSurfaceContainerLowest ||
                       token == material3::ColorToken::kSurfaceContainerLow ||
                       token == material3::ColorToken::kSurfaceContainer ||
                       token == material3::ColorToken::kSurfaceContainerHigh ||
                       token == material3::ColorToken::kSurfaceContainerHighest ||
                       token == material3::ColorToken::kSurfaceVariant ||
                       token == material3::ColorToken::kInverseSurface;
  switch (interaction) {
    case InteractionState::kHover:
      return primary ? state.hoverOnPrimary : secondary ? state.hoverOnSecondary
                     : error ? state.hoverOnError
                             : surface ? state.hoverOnSurface
                                       : state.hoverOnBackground;
    case InteractionState::kFocus:
      return primary ? state.focusOnPrimary : secondary ? state.focusOnSecondary
                     : error ? state.focusOnError
                             : surface ? state.focusOnSurface
                                       : state.focusOnBackground;
    case InteractionState::kSelected:
      return primary ? state.selectedOnPrimary
                     : secondary ? state.selectedOnSecondary
                     : error ? state.selectedOnError
                             : surface ? state.selectedOnSurface
                                       : state.selectedOnBackground;
    case InteractionState::kActivated:
      return primary ? state.activatedOnPrimary
                     : secondary ? state.activatedOnSecondary
                     : error ? state.activatedOnError
                             : surface ? state.activatedOnSurface
                                       : state.activatedOnBackground;
    case InteractionState::kPressed:
      return primary ? state.pressedOnPrimary
                     : secondary ? state.pressedOnSecondary
                     : error ? state.pressedOnError
                             : surface ? state.pressedOnSurface
                                       : state.pressedOnBackground;
    case InteractionState::kDragged:
      return primary ? state.draggedOnPrimary
                     : secondary ? state.draggedOnSecondary
                     : error ? state.draggedOnError
                             : surface ? state.draggedOnSurface
                                       : state.draggedOnBackground;
  }
  return state.pressedOnBackground;
}

constexpr roo_display::Color StateLayerContent(
    const material3::ColorScheme& color, material3::ColorToken token) {
  switch (token) {
    case material3::ColorToken::kPrimary:
      return color.onPrimary;
    case material3::ColorToken::kPrimaryContainer:
      return color.onPrimaryContainer;
    case material3::ColorToken::kSecondary:
      return color.onSecondary;
    case material3::ColorToken::kSecondaryContainer:
      return color.onSecondaryContainer;
    case material3::ColorToken::kTertiary:
      return color.onTertiary;
    case material3::ColorToken::kTertiaryContainer:
      return color.onTertiaryContainer;
    case material3::ColorToken::kError:
      return color.onError;
    case material3::ColorToken::kErrorContainer:
      return color.onErrorContainer;
    case material3::ColorToken::kSurfaceVariant:
      return color.onSurfaceVariant;
    case material3::ColorToken::kInverseSurface:
      return color.inverseOnSurface;
    case material3::ColorToken::kBackground:
      return color.onBackground;
    default:
      return color.onSurface;
  }
}

constexpr ColorTheme MakeLegacyColorTheme(
    const material3::ColorScheme& color) {
  return {
      .primary = color.primary, .primaryVariant = color.primary,
      .primaryContainer = color.primaryContainer,
      .onPrimaryContainer = color.onPrimaryContainer,
      .secondary = color.secondary, .secondaryVariant = color.secondary,
      .secondaryContainer = color.secondaryContainer,
      .onSecondaryContainer = color.onSecondaryContainer,
      .tertiary = color.tertiary, .onTertiary = color.onTertiary,
      .tertiaryContainer = color.tertiaryContainer,
      .onTertiaryContainer = color.onTertiaryContainer,
      .background = color.background, .onBackground = color.onBackground,
      .surface = color.surface,
      .surfaceContainerLowest = color.surfaceContainerLowest,
      .surfaceContainerLow = color.surfaceContainerLow,
      .surfaceContainer = color.surfaceContainer,
      .surfaceContainerHigh = color.surfaceContainerHigh,
      .surfaceContainerHighest = color.surfaceContainerHighest,
      .onSurface = color.onSurface, .surfaceVariant = color.surfaceVariant,
      .onSurfaceVariant = color.onSurfaceVariant, .error = color.error,
      .errorContainer = color.errorContainer, .onPrimary = color.onPrimary,
      .onSecondary = color.onSecondary, .onError = color.onError,
      .onErrorContainer = color.onErrorContainer, .outline = color.outline,
      .outlineVariant = color.outlineVariant,
      .inverseSurface = color.inverseSurface,
      .inverseOnSurface = color.inverseOnSurface,
      .inversePrimary = color.inversePrimary, .surfaceTint = color.surfaceTint,
  };
}

const material3::Material3Theme& DefaultMaterial3Theme() {
  static constexpr material3::Material3Theme theme = [] constexpr {
    material3::Material3Theme result = {};
    result.color = DefaultMaterial3Colors();
    const StateOpacityTheme legacy_state = DefaultLegacyState();
    result.state.disabledContentOpacity = legacy_state.disabled;
    for (uint8_t token = 0; token < 33; ++token) {
      for (uint8_t interaction = 0; interaction < 6; ++interaction) {
        const auto color_token = static_cast<material3::ColorToken>(token);
        const auto state = static_cast<InteractionState>(interaction);
        result.state.layer[token][interaction] =
            StateLayerContent(result.color, color_token)
                .withA(StateOpacity(legacy_state, color_token, state));
      }
    }
    return result;
  }();
  return theme;
}

}  // namespace

const Theme& DefaultTheme() {
  static const Theme theme = [] {
    const material3::Material3Theme& material = DefaultMaterial3Theme();
    return Theme{
        .color = MakeLegacyColorTheme(material.color),
        .state = DefaultLegacyState(),
        .framework = material3::MakeFrameworkTheme(material),
        .material3_theme = &material,
    };
  }();
  return theme;
}

const material3::Material3Theme& Theme::material3Theme() const {
  assert(material3_theme != nullptr && "Material 3 theme is not installed");
  return *material3_theme;
}

const KeyboardColorTheme& DefaultKeyboardColorTheme() {
  static KeyboardColorTheme theme = {
      .background = roo_display::Color(0xFF303030),
      .normalButton = roo_display::Color(0xFF4C4C4C),
      .modifierButton = roo_display::Color(0xFF3C3C3C),
      .acceptButton = roo_display::Color(0xFF018786),
      .text = roo_display::Color(0xFFFFFFFF)};
  return theme;
}

}  // namespace roo_windows
