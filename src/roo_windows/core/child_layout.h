#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

/// Adds a child's margins to its surface size, clamping negative totals to
/// zero.
Dimensions AddChildMargins(const Widget& child, Dimensions dimensions);

/// Measures inside child margins while preserving each constraint's kind.
/// Returns the occupied size including margins, bounded by the supplied specs.
/// Unspecified axes remain free to grow. Gone children occupy no space.
Dimensions MeasureChildWithMargins(Widget& child, WidthSpec width,
                                   HeightSpec height);

/// Removes physical child margins from a slot; empty slots remain empty even
/// with negative margins. Returns empty bounds when margins exhaust either
/// axis.
Rect ChildBoundsWithoutMargins(const Widget& child, const Rect& slot);

/// Lays out the child inside its slot margins, clearing gone children.
void LayoutChildWithMargins(Widget& child, const Rect& slot);

}  // namespace roo_windows
