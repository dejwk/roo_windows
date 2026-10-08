#pragma once

#include <stdint.h>

#include "roo_windows/core/dimensions.h"

namespace roo_windows::material3::internal {

// Pixel metrics independent of focus, editing, and floating-label state.
// Zero icon height means absent; present icons include the nominal slot height.
struct TextFieldMetrics {
  int body_height;
  int small_height;
  int leading_height;
  int trailing_height;
};

// Ephemeral measured inputs, never retained by a widget.
struct TextFieldSlotInput {
  Dimensions bounds;
  bool outlined;
  bool floating;
  bool rtl;
  TextFieldMetrics metrics;
  int label_width;
  int prefix_width;
  int suffix_width;
};

// Owner-local rectangles for painted regions and the editor viewport.
struct TextFieldSlots {
  Rect container;
  Rect label;
  Rect viewport;
  Rect prefix;
  Rect suffix;
  Rect leading;
  Rect trailing;
  Rect assist;
};

/// Resolves the state-independent container for valid density levels [-5, 0].
/// Level zero retains the legacy 56 dp height even with oversized content.
int ResolveTextFieldContainerHeight(bool outlined,
                                    const TextFieldMetrics& metrics,
                                    int8_t level);

/// Adds outlined label clearance and the unchanged optional assistive band.
int ResolveTextFieldNaturalHeight(bool outlined, bool assistive,
                                  const TextFieldMetrics& metrics,
                                  int8_t level);

/// Places and clips all slots using the same container resolver as measurement.
/// Mirroring preserves logical prefix/suffix roles; tight bounds may clip
/// content.
TextFieldSlots ResolveTextFieldSlots(const TextFieldSlotInput& input,
                                     int8_t level);

}  // namespace roo_windows::material3::internal
