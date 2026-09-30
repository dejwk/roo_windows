#pragma once

#include "roo_windows/core/rect.h"

namespace roo_windows {

/// Content origin relative to the viewport, excluding content margins.
/// Scrolling down/right produces negative y/x. Overscroll and alignment of
/// content smaller than the viewport may produce positive coordinates.
struct ScrollPosition {
  XDim x;
  YDim y;
};

}  // namespace roo_windows
