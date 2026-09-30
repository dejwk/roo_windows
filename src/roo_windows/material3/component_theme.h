#pragma once

#include "roo_windows/material3/color_token.h"

namespace roo_windows::material3 {

/// Selects neutral fills for the three Material card styles.
struct CardTheme {
  ColorToken elevatedContainer = ColorToken::kSurfaceContainerLow;
  ColorToken filledContainer = ColorToken::kSurfaceContainerHighest;
  ColorToken outlinedContainer = ColorToken::kSurface;
};

/// Selects the Material surface used by a page-level layout scaffold.
struct LayoutScaffoldTheme {
  ColorToken container = ColorToken::kSurfaceContainerLowest;
};

/// Selects flat and scrolled title app-bar surfaces.
struct AppBarTheme {
  ColorToken flatContainer = ColorToken::kSurfaceContainerLowest;
  ColorToken scrolledContainer = ColorToken::kSurfaceContainer;
};

/// Selects the standalone search-entry surface.
struct SearchBarTheme {
  ColorToken container = ColorToken::kSurfaceContainerHigh;
};

/// Selects outer and embedded surfaces for a search app bar.
struct SearchAppBarTheme {
  ColorToken flatContainer = ColorToken::kSurfaceContainerLowest;
  ColorToken scrolledContainer = ColorToken::kSurfaceContainer;
  ColorToken flatSearchContainer = ColorToken::kSurfaceContainer;
  ColorToken scrolledSearchContainer = ColorToken::kSurfaceContainerHighest;
};

/// Selects basic and full-screen dialog surfaces independently.
struct DialogTheme {
  ColorToken basicContainer = ColorToken::kSurfaceContainerHigh;
  ColorToken fullScreenContainer = ColorToken::kSurface;
};

/// Selects the shared modal, docked, and input date-picker panel surface.
struct DatePickerTheme {
  ColorToken container = ColorToken::kSurfaceContainerHigh;
};

/// Selects the Material surface used by the bottom navigation bar.
struct NavigationBarTheme {
  ColorToken container = ColorToken::kSurfaceContainer;
};

/// Selects optional fills for persistent navigation rail layouts.
struct NavigationRailTheme {
  ColorToken collapsedContainer = ColorToken::kSurface;
  ColorToken expandedContainer = ColorToken::kSurface;
};

/// Selects the shared strip and tab fill for each tabs variant.
struct TabsTheme {
  ColorToken primaryContainer = ColorToken::kSurface;
  ColorToken secondaryContainer = ColorToken::kSurface;
};

/// Selects unselected standard and segmented list-row surfaces.
struct ListTheme {
  ColorToken standardContainer = ColorToken::kSurface;
  // Spec says 'surface' but Compose uses 'surface container'.
  ColorToken segmentedContainer = ColorToken::kSurfaceContainer;
};

/// Selects baseline and standard expressive menu resting surfaces.
struct MenuTheme {
  ColorToken baselineContainer = ColorToken::kSurfaceContainer;
  ColorToken expressiveContainer = ColorToken::kSurfaceContainerLow;
};

/// Selects the enabled resting surface of filled text fields.
struct TextFieldTheme {
  ColorToken filledContainer = ColorToken::kSurfaceContainerHighest;
};

/// Selects the enabled resting surface of elevated buttons.
struct ButtonTheme {
  ColorToken elevatedContainer = ColorToken::kSurfaceContainerLow;
};

/// Selects the enabled unselected surface of filled toggle icon buttons.
struct ToggleIconButtonTheme {
  ColorToken filledUnselectedContainer = ColorToken::kSurfaceContainer;
};

/// Selects the enabled unselected track surface of switches.
struct SwitchTheme {
  ColorToken unselectedTrack = ColorToken::kSurfaceContainerHighest;
};

/// Collects application-wide Material component surface defaults.
///
/// Assign one of the neutral roles (`kSurface` or a
/// `kSurfaceContainer*` role) to these slots. Neutral roles preserve the
/// components' existing foreground and interaction-layer contracts. Accent,
/// inverse, error, and `kNone` roles need companion foreground or inheritance
/// policy and should therefore remain instance-specific customizations.
///
/// Configure this shared storage before constructing widgets and keep it alive
/// for as long as the Theme and its widgets borrow it. Component lookups are
/// live, so applications that later change a slot must invalidate affected
/// subtrees between completed frames.
///
/// The 27 compact token slots below cover the implemented component-surface
/// contract. Adding a slot changes the application-owned Material3Theme ABI;
/// rebuild the library and application together.
struct ComponentTheme {
  LayoutScaffoldTheme layoutScaffold;
  AppBarTheme appBar;
  SearchBarTheme searchBar;
  SearchAppBarTheme searchAppBar;
  CardTheme card;
  DialogTheme dialog;
  DatePickerTheme datePicker;
  NavigationBarTheme navigationBar;
  NavigationRailTheme navigationRail;
  TabsTheme tabs;
  ListTheme list;
  MenuTheme menu;
  TextFieldTheme textField;
  ButtonTheme button;
  ToggleIconButtonTheme toggleIconButton;
  SwitchTheme switchControl;
};

static_assert(sizeof(CardTheme) == 3,
              "Card surface defaults must occupy three compact tokens.");
static_assert(alignof(CardTheme) == 1,
              "Card surface defaults must not introduce padding.");
static_assert(sizeof(ComponentTheme) == 27,
              "Component theme must contain only implemented surface slots.");
static_assert(alignof(ComponentTheme) == 1,
              "Component theme must not introduce padding.");

}  // namespace roo_windows::material3
