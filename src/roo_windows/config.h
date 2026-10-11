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

#ifndef ROO_WINDOWS_MAX_BITS_PER_CHANNEL
// For RGB565, leave '6'. If you use RGB888 display, change to 8.
#define ROO_WINDOWS_MAX_BITS_PER_CHANNEL 6
#endif

/// Gaussian renderer: 1 uses a shared edge table; 2 uses opacity-baked tables.
/// Zero requires analytical mode. Integer elevation 0..31 uses half-dp steps
/// (0..15.5 logical dp, clamped). Authoritative fitted parameters use reference
/// heights 0/3/5/8/10/14 dp; zero disables both layers. Define globally for the
/// whole build; this is experimental.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT 2
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

#if !ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC && \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 0
#error "Shadow rendering requires one/two LUTs or analytical mode"
#endif

/// Preserves two-table output while tightening support and reusing flat spans.
/// This optimization branch enables it by default; the benchmark can disable
/// it.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED 1
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED != 0 && \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED != 1
#error "Experimental shadow optimization must be 0 or 1"
#endif
#define ROO_WINDOWS_SHADOW_FAST_LUT             \
  (ROO_WINDOWS_EXPERIMENTAL_SHADOW_OPTIMIZED && \
   ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 2)

/// Precomputed flash gradients: 0 none, 1 straight sides, 2 sides and corners
/// at Scaled(4/8/12/16/20/24) radii. Other corners retain exact calculation.
#ifndef ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS
#define ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS 0
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS < 0 || \
    ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS > 2
#error "Experimental shadow gradients must be 0, 1, or 2"
#endif
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS && !ROO_WINDOWS_SHADOW_FAST_LUT
#error "Precomputed gradients require optimized two-table shadows"
#endif

/// Optional shadow-bounds cutoff: 0 preserves full bounds (default),
/// 2 trims bounds where combined alpha <= 2/255. Sampled alphas are unchanged.
/// Requires optimized
/// two-LUT rendering. This is an approximation when composing over RGB888;
/// it is exact for black source-over shadows on existing RGB565 pixels using
/// roo_display's RGB565 decode/blend/encode path. Define for the whole build.
#ifndef ROO_WINDOWS_SHADOW_ALPHA_CUTOFF
#if ROO_WINDOWS_MAX_BITS_PER_CHANNEL <= 6
#define ROO_WINDOWS_SHADOW_ALPHA_CUTOFF 2
#else
#define ROO_WINDOWS_SHADOW_ALPHA_CUTOFF 0
#endif
#endif
#if ROO_WINDOWS_SHADOW_ALPHA_CUTOFF != 0 && ROO_WINDOWS_SHADOW_ALPHA_CUTOFF != 2
#error "Shadow alpha cutoff must be 0 or 2"
#endif
#if ROO_WINDOWS_SHADOW_ALPHA_CUTOFF && !ROO_WINDOWS_SHADOW_FAST_LUT
#error "Shadow alpha cutoff requires optimized two-LUT rendering"
#endif
