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

/// Opt-in shadow benchmark prototype: 0 keeps the production legacy renderer;
/// 1 uses a shared Gaussian edge table, 2 uses opacity-baked key/ambient
/// tables. Nonzero modes interpret elevation as Material Web level 0..5
/// (clamped). Define globally for the whole build; this is not a stable public
/// API.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT 0
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT < 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT > 2
#error "Experimental shadow LUT count must be 0, 1, or 2"
#endif

/// Places experimental LUT bytes in internal ESP32 DRAM instead of flash.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM 0
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM != 0 && \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM != 1
#error "Experimental shadow LUT RAM must be 0 or 1"
#endif

/// Evaluates the experimental Gaussian edge analytically with Q14 integer
/// arithmetic instead of a LUT. Uses the same Material levels and geometry.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC 0
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC != 0 && \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC != 1
#error "Experimental analytical shadow mode must be 0 or 1"
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC && \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0
#error "Select analytical shadows or LUT shadows, not both"
#endif

/// Retunes only legacy linear-shadow parameters to Material Web levels 0..5.
/// Keeps the legacy rasterizer and additive alpha. Fitted only at zoom 100%.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED 0
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED != 0 && \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED != 1
#error "Experimental tuned legacy shadow mode must be 0 or 1"
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED &&    \
    (ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT != 0 || \
     ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC)
#error "Select tuned legacy shadows or Gaussian shadows, not both"
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_LEGACY_TUNED && ROO_WINDOWS_ZOOM != 100
#error "Tuned legacy shadow parameters are calibrated only at zoom 100%"
#endif
