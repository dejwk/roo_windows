#pragma once

#include <stdint.h>

#include <algorithm>
#include <cmath>

#include "roo_display/core/box.h"
#include "roo_logging.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {
namespace material3 {
namespace internal {

static constexpr float kSliderStepDivisibilityTolerance = 1e-4f;

inline bool IsDiscreteSliderRange(float step) { return step > 0.0f; }

inline bool IsCompatibleSliderStep(float from, float to, float step) {
  if (!IsDiscreteSliderRange(step)) return true;
  float steps = (to - from) / step;
  return std::fabs(steps - roundf(steps)) <= kSliderStepDivisibilityTolerance;
}

inline bool IsValidSliderRange(float from, float to, float step) {
  return std::isfinite(from) && std::isfinite(to) && std::isfinite(step) &&
         from < to && step >= 0.0f && IsCompatibleSliderStep(from, to, step);
}

inline void CheckValidSliderRange(float from, float to, float step) {
  CHECK(IsValidSliderRange(from, to, step))
      << "Invalid slider range: from=" << from << ", to=" << to
      << ", step=" << step;
}

inline float ClampSliderValue(float value, float from, float to) {
  if (std::isnan(value)) return from;
  if (value < from) return from;
  if (value > to) return to;
  return value;
}

inline float SnapSliderValueToRange(float value, float from, float to,
                                    float step) {
  float clamped = ClampSliderValue(value, from, to);
  if (!IsDiscreteSliderRange(step)) return clamped;
  float steps = roundf((clamped - from) / step);
  float snapped = from + steps * step;
  return ClampSliderValue(snapped, from, to);
}

inline float NormalizeSliderValueForRange(float value, float from, float to,
                                          float step) {
  return SnapSliderValueToRange(value, from, to, step);
}

inline float SliderValueFromNormalizedPos(float from, float to, uint16_t pos) {
  return from + (to - from) * ((float)pos / 65535.0f);
}

inline uint16_t SliderPosFromValue(float from, float to, float value) {
  if (!(to > from)) return 0;
  float clamped = ClampSliderValue(value, from, to);
  float normalized = (clamped - from) / (to - from);
  int32_t pos = (int32_t)floorf(normalized * 65535.0f + 0.5f);
  if (pos < 0) pos = 0;
  if (pos > 65535) pos = 65535;
  return (uint16_t)pos;
}

// Orientation-aware geometry adapter for slider math. It lets the slider and
// range slider express movement and painting in a single logical primary axis
// while converting to widget-local display coordinates for horizontal and
// vertical layouts.
class SliderAxisMetrics {
 public:
  struct PaintRect {
    float x_min;
    float y_min;
    float x_max;
    float y_max;
  };

  // Builds axis metrics for a slider surface. `thumb_primary_span` is the
  // thumb length along the travel axis. `track_handle_gap` is both the
  // painted gap between the track and the handle and the logical stop-mark
  // offset from each widget edge.
  SliderAxisMetrics(int16_t width, int16_t height, int16_t thumb_primary_span,
                    int16_t track_handle_gap, bool vertical = false)
      : primary_span_(vertical ? height : width),
        cross_span_(vertical ? width : height),
        thumb_primary_span_(thumb_primary_span),
        track_handle_gap_(track_handle_gap),
        vertical_(vertical) {}

  // Returns the widget extent along the slider's travel axis.
  int16_t primarySpan() const { return primary_span_; }

  // Returns the widget extent perpendicular to the slider's travel axis.
  int16_t crossSpan() const { return cross_span_; }

  // Returns the thumb length along the logical travel axis.
  int16_t thumbPrimarySpan() const { return thumb_primary_span_; }

  // Returns whether display coordinates are mapped vertically.
  bool isVertical() const { return vertical_; }

  // Converts a normalized 16-bit position into the thumb center in logical
  // primary-axis coordinates.
  float centerFromPos(uint16_t pos) const {
    return (float)track_handle_gap_ - 0.5f +
           (float)(((uint32_t)pos * (uint32_t)normalizedRange() + 32768u) >>
                   16);
  }

  // Converts a normalized 16-bit position into the thumb center in
  // display-space coordinates on the travel axis.
  float displayCenterFromPos(uint16_t pos) const {
    return displayPrimary(centerFromPos(pos));
  }

  // Converts a logical primary coordinate into a display-space coordinate.
  // Vertical sliders invert the primary axis so larger logical values paint
  // higher on screen.
  float displayPrimary(float primary) const {
    return vertical_ ? (float)(primary_span_ - 1) - primary : primary;
  }

  // Converts a display-space primary coordinate back into a normalized 16-bit
  // slider position, clamping to the valid travel range.
  uint16_t posFromPrimaryCoord(int16_t primary_coord) const {
    int16_t range = normalizedRange();
    if (range <= 0) return 0;
    int32_t pos = primary_coord + 1 - track_handle_gap_;
    if (pos <= 0) return 0;
    if (pos >= range) return 65535;
    return (((uint32_t)pos << 16) + (uint32_t)(range / 2)) / (uint32_t)range;
  }

  // Extracts the display-space coordinate on the travel axis from a widget
  // point.
  int16_t primaryCoordFromPoint(XDim x, YDim y) const {
    return vertical_ ? (primary_span_ - 1 - y) : x;
  }

  // Projects a display-space drag delta onto the logical travel axis.
  int16_t primaryDelta(XDim dx, YDim dy) const { return vertical_ ? -dy : dx; }

  // Returns whether a scroll gesture should be captured by the slider based on
  // the dominant gesture direction and current drag state.
  bool shouldCaptureScroll(bool is_dragging, XDim dx, YDim dy) const {
    if (!vertical_) {
      return is_dragging || !((dy * dy > dx * dx) && dy * dy > 25);
    }
    return is_dragging || !((dx * dx > dy * dy) && dx * dx > 25);
  }

  // Returns whether the thumb at `pos` should be considered hit by a press at
  // `primary_coord`, expanded by `touch_slop` pixels on both sides.
  bool hitsThumb(uint16_t pos, int16_t primary_coord,
                 int16_t touch_slop) const {
    uint16_t min_pos = posFromPrimaryCoord(primary_coord - touch_slop);
    uint16_t max_pos = posFromPrimaryCoord(primary_coord + touch_slop);
    return pos >= min_pos && pos <= max_pos;
  }

  // Returns the widget-local dirty rectangle that covers the painted handle
  // tile sweep between `old_pos` and `new_pos`.
  Rect invalidationRectForPosChange(uint16_t old_pos, uint16_t new_pos) const {
    float old_center = centerFromPos(old_pos);
    float new_center = centerFromPos(new_pos);
    int16_t min_primary = (int16_t)ceilf(std::min(old_center, new_center) -
                                         0.5f * (float)thumb_primary_span_) -
                          track_handle_gap_;
    int16_t max_primary = (int16_t)floorf(std::max(old_center, new_center) +
                                          0.5f * (float)thumb_primary_span_) +
                          track_handle_gap_;
    return rectFromPrimaryCross(min_primary, 0, max_primary, cross_span_ - 1);
  }

  // Returns the cross-axis center in display coordinates.
  float centeredCross() const { return 0.5f * (float)(cross_span_ - 1); }

  // Converts a logical primary/cross box into a display-space box.
  roo_display::Box boxFromPrimaryCross(int16_t min_primary, int16_t min_cross,
                                       int16_t max_primary,
                                       int16_t max_cross) const {
    Rect rect =
        rectFromPrimaryCross(min_primary, min_cross, max_primary, max_cross);
    return roo_display::Box(rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax());
  }

  // Converts logical primary/cross paint bounds into display-space floating
  // rectangle coordinates.
  PaintRect paintRectFromPrimaryCross(float min_primary, float min_cross,
                                      float max_primary,
                                      float max_cross) const {
    if (!vertical_) {
      return PaintRect{min_primary, min_cross, max_primary, max_cross};
    }
    return PaintRect{min_cross, displayPrimary(max_primary), max_cross,
                     displayPrimary(min_primary)};
  }

 private:
  int16_t normalizedRange() const {
    int16_t range = primary_span_ - 2 * track_handle_gap_;
    return range < 0 ? 0 : range;
  }

  int16_t centeredCrossStart(int16_t span) const {
    return (cross_span_ - span) / 2;
  }

  Rect rectFromPrimaryCross(int16_t min_primary, int16_t min_cross,
                            int16_t max_primary, int16_t max_cross) const {
    if (!vertical_) {
      return Rect(min_primary, min_cross, max_primary, max_cross);
    }
    return Rect(min_cross, primary_span_ - 1 - max_primary, max_cross,
                primary_span_ - 1 - min_primary);
  }

  int16_t primary_span_;
  int16_t cross_span_;
  int16_t thumb_primary_span_;
  int16_t track_handle_gap_;
  bool vertical_;
};

struct DirtySpan {
  DirtySpan() : min_coord(0), max_coord(-1) {}
  DirtySpan(int16_t min_coord, int16_t max_coord)
      : min_coord(min_coord), max_coord(max_coord) {}

  bool empty() const { return max_coord < min_coord; }

  void include(const DirtySpan& other) {
    if (other.empty()) return;
    if (empty()) {
      *this = other;
      return;
    }
    if (other.min_coord < min_coord) min_coord = other.min_coord;
    if (other.max_coord > max_coord) max_coord = other.max_coord;
  }

  int16_t min_coord;
  int16_t max_coord;
};

inline DirtySpan DisplayMainSpanFromRect(const Rect& rect, bool vertical) {
  if (rect.empty()) return DirtySpan();
  return vertical ? DirtySpan((int16_t)rect.yMin(), (int16_t)rect.yMax())
                  : DirtySpan(rect.xMin(), rect.xMax());
}

inline Rect ContentRectFromDisplayMainSpan(const DirtySpan& span, int16_t width,
                                           int16_t height, bool vertical) {
  if (span.empty() || width <= 0 || height <= 0) return Rect(0, 0, -1, -1);
  if (vertical) {
    return Rect(0, span.min_coord, width - 1, span.max_coord);
  }
  return Rect(span.min_coord, 0, span.max_coord, height - 1);
}

struct SliderVisualMetrics {
  float thumb_center_primary;
  int16_t track_cross_start;
  float track_min_cross;
  float track_max_cross;
  float thumb_min_primary;
  float thumb_max_primary;
  int16_t thumb_cross_start;
  float thumb_min_cross;
  float thumb_max_cross;
  float active_track_max_primary;
  float inactive_track_min_primary;

  int16_t activeClipMax(int16_t primary_span) const {
    int16_t clip_max = (int16_t)floorf(active_track_max_primary);
    if (clip_max < 0) clip_max = -1;
    if (clip_max >= primary_span) clip_max = primary_span - 1;
    return clip_max;
  }

  int16_t inactiveClipMin(int16_t primary_span) const {
    int16_t clip_min = (int16_t)ceilf(inactive_track_min_primary);
    if (clip_min < 0) clip_min = 0;
    if (clip_min > primary_span) clip_min = primary_span;
    return clip_min;
  }
};

inline SliderVisualMetrics ResolveSliderVisualMetrics(
    const SliderAxisMetrics& axis, float thumb_center_primary,
    int16_t thumb_primary_span, int16_t track_cross_span,
    int16_t track_handle_gap, int16_t thumb_cross_span) {
  int16_t track_cross_start = (axis.crossSpan() - track_cross_span) / 2;
  float track_min_cross = track_cross_start - 0.5f;
  float track_max_cross = track_cross_start + track_cross_span - 0.5f;
  float thumb_min_primary =
      thumb_center_primary - 0.5f * (float)(thumb_primary_span - 1) - 0.5f;
  float thumb_max_primary = thumb_min_primary + thumb_primary_span;
  int16_t thumb_cross_start = (axis.crossSpan() - thumb_cross_span) / 2;
  float thumb_min_cross = thumb_cross_start - 0.5f;
  float thumb_max_cross = thumb_min_cross + thumb_cross_span;
  float active_track_max_primary = thumb_min_primary - (float)track_handle_gap;
  float inactive_track_min_primary =
      thumb_max_primary + (float)track_handle_gap;
  return SliderVisualMetrics{
      thumb_center_primary,     track_cross_start,         track_min_cross,
      track_max_cross,          thumb_min_primary,         thumb_max_primary,
      thumb_cross_start,        thumb_min_cross,           thumb_max_cross,
      active_track_max_primary, inactive_track_min_primary};
}

inline SliderVisualMetrics ResolveHorizontalSliderVisualMetrics(
    const SliderAxisMetrics& axis, uint16_t pos, int16_t track_cross_span,
    int16_t track_handle_gap, int16_t thumb_cross_span) {
  return ResolveSliderVisualMetrics(axis, axis.centerFromPos(pos),
                                    axis.thumbPrimarySpan(), track_cross_span,
                                    track_handle_gap, thumb_cross_span);
}

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows