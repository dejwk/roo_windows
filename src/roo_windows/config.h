#pragma once

#include "roo_locale.h"

/// Select UI language (defaults to `ROO_LANG`, which defaults to English).
/// Example override:
/// `#define ROO_WINDOWS_LANG ROO_LANG_pl`
#define ROO_WINDOWS_LANG ROO_LANG

/// roo_windows supports four zoom levels.
///
/// Select one explicitly by uncommenting one `ROO_WINDOWS_ZOOM` line below, or
/// define `ROO_WINDOWS_DPI` and let the library choose automatically.

#ifndef ROO_WINDOWS_ZOOM

// #define ROO_WINDOWS_ZOOM 75
#define ROO_WINDOWS_ZOOM 100
// #define ROO_WINDOWS_ZOOM 150
// #define ROO_WINDOWS_ZOOM 200

// #define ROO_WINDOWS_DPI 180
// (used only when no explicit ROO_WINDOWS_ZOOM is selected)

#endif
