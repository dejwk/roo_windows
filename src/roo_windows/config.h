#pragma once

#include "roo_locale.h"

/// Select UI language (defaults to `ROO_LANG`, which defaults to English).
/// Example override:
/// `#define ROO_WINDOWS_LANG ROO_LANG_pl`
#define ROO_WINDOWS_LANG ROO_LANG

/// roo_windows supports four zoom levels.
///
/// Select one explicitly by uncommenting one `ROO_DISPLAY_ZOOM` line below, or
/// define `ROO_DISPLAY_DPI` and let the library choose automatically.

// #define ROO_DISPLAY_ZOOM 75
#define ROO_DISPLAY_ZOOM 100
// #define ROO_DISPLAY_ZOOM 150
// #define ROO_DISPLAY_ZOOM 200

// #define ROO_DISPLAY_DPI 180
// (used only when no explicit ROO_DISPLAY_ZOOM is selected)
