#pragma once

#include "roo_locale.h"

// Use one of the supported languages. If undefined, defaults to 'en'.
// #define ROO_WINDOWS_LANG ROO_LANG_pl
#define ROO_WINDOWS_LANG ROO_LANG

// roo_windows support 4 zoom levels. You can either select one directly (by
// uncommenting a particular setting below), or let the library pick one for
// you, based on your display's DPI (by defining ROO_DISPLAY_DPI).

// #define ROO_DISPLAY_ZOOM 75
#define ROO_DISPLAY_ZOOM 100
// #define ROO_DISPLAY_ZOOM 150
// #define ROO_DISPLAY_ZOOM 200

// #define ROO_DISPLAY_DPI 180
