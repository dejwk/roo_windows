#pragma once

#include "roo_locale.h"

/// Enables framebuffer-backed scroll reuse for the `ScrollablePanel` alias.
///
/// Set this globally to 1 only when the selected display keeps a framebuffer
/// and advertises blit-copy support. The default keeps scrollable widgets at
/// the smaller `SimpleScrollablePanel` size. `ScrollableBlitPanel` remains
/// available for explicit use regardless of this setting.
#ifndef ROO_WINDOWS_ENABLE_BLIT_CACHE
#define ROO_WINDOWS_ENABLE_BLIT_CACHE 0
#endif

#if ROO_WINDOWS_ENABLE_BLIT_CACHE != 0 && ROO_WINDOWS_ENABLE_BLIT_CACHE != 1
#error "ROO_WINDOWS_ENABLE_BLIT_CACHE must be 0 or 1"
#endif

/// Rejects framebuffer copies smaller than this device-specific pixel count.
///
/// Copy cost depends on framebuffer memory and the content that repaint would
/// replace, so the portable default accepts every proven rectangle. Set this
/// globally from target measurements when small copies cost more than repaint.
#ifndef ROO_WINDOWS_MIN_BLIT_COPY_PIXELS
#define ROO_WINDOWS_MIN_BLIT_COPY_PIXELS 0
#endif

#if ROO_WINDOWS_MIN_BLIT_COPY_PIXELS < 0
#error "ROO_WINDOWS_MIN_BLIT_COPY_PIXELS must not be negative"
#endif

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
