#include "roo_windows/core/clipper.h"
#include "roo_windows/material3/progress_indicator/progress_indicator.h"

// Named target-ABI sizes; unreferenced sections are discarded from firmware.
extern "C" {
char progress_size_widget[sizeof(roo_windows::Widget)];
char progress_size_linear[sizeof(
    roo_windows::material3::LinearProgressIndicator)];
char progress_size_circular[sizeof(
    roo_windows::material3::CircularProgressIndicator)];
char progress_size_clipper_state[sizeof(roo_windows::internal::ClipperState)];
char progress_size_shape[sizeof(roo_display::SmoothShape)];
char progress_size_overlay[sizeof(roo_windows::internal::ClippedOverlay)];
char progress_size_overlay_stack[sizeof(roo_display::RasterizableStack)];
}
