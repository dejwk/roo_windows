#pragma once

#include <algorithm>
#include <stdint.h>

#include "roo_windows/core/rect.h"

namespace roo_windows {
namespace material3 {
namespace internal {

struct SliderAxisMetrics {
  SliderAxisMetrics(int16_t primary_span, int16_t cross_span,
                    int16_t thumb_primary_span, int16_t interaction_radius)
      : primary_span(primary_span),
        cross_span(cross_span),
        thumb_primary_span(thumb_primary_span),
        interaction_radius(interaction_radius) {}

  int16_t normalizedRange() const {
    int16_t range = primary_span - thumb_primary_span;
    return range < 1 ? 1 : range;
  }

  float centerFromPos(uint16_t pos) const {
    return 0.5f * (float)(thumb_primary_span - 1) +
           (float)(((uint32_t)pos * (uint32_t)normalizedRange() + 32768u) >>
                   16);
  }

  uint16_t posFromPrimaryCoord(int16_t primary_coord) const {
    int32_t pos = primary_coord + 1 - thumb_primary_span / 2;
    int16_t range = normalizedRange();
    if (pos < 0) pos = 0;
    if (pos >= range) pos = range - 1;
    return (((uint32_t)pos << 16) + (uint32_t)(range / 2)) / (uint32_t)range;
  }

  bool hitsThumb(uint16_t pos, int16_t primary_coord, int16_t touch_slop) const {
    uint16_t min_pos = posFromPrimaryCoord(primary_coord - touch_slop);
    uint16_t max_pos = posFromPrimaryCoord(primary_coord + touch_slop);
    return pos >= min_pos && pos <= max_pos;
  }

  Rect invalidationRectForPosChange(uint16_t old_pos, uint16_t new_pos) const {
    float old_center = centerFromPos(old_pos);
    float new_center = centerFromPos(new_pos);
    int16_t min_primary =
        (int16_t)std::min(old_center, new_center) - interaction_radius;
    int16_t max_primary =
        (int16_t)std::max(old_center, new_center) + interaction_radius;
    return Rect(min_primary, 0, max_primary, cross_span - 1);
  }

  int16_t centeredCrossStart(int16_t span) const {
    return (cross_span - span) / 2;
  }

  int16_t primary_span;
  int16_t cross_span;
  int16_t thumb_primary_span;
  int16_t interaction_radius;
};

inline bool ShouldCaptureHorizontalSliderScroll(bool is_dragging, XDim dx,
                                                YDim dy) {
  return is_dragging || !((dy * dy > dx * dx) && dy * dy > 25);
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

inline SliderVisualMetrics ResolveHorizontalSliderVisualMetrics(
    const SliderAxisMetrics& axis, uint16_t pos, int16_t track_cross_span,
    int16_t track_handle_gap, int16_t thumb_cross_span) {
  float thumb_center_primary = axis.centerFromPos(pos);
  int16_t track_cross_start = axis.centeredCrossStart(track_cross_span);
  float track_min_cross = track_cross_start - 0.5f;
  float track_max_cross = track_cross_start + track_cross_span - 0.5f;
  float thumb_min_primary =
      thumb_center_primary - 0.5f * (float)(axis.thumb_primary_span - 1) -
      0.5f;
  float thumb_max_primary = thumb_min_primary + axis.thumb_primary_span;
  int16_t thumb_cross_start = axis.centeredCrossStart(thumb_cross_span);
  float thumb_min_cross = thumb_cross_start - 0.5f;
  float thumb_max_cross = thumb_min_cross + thumb_cross_span;
  float active_track_max_primary = thumb_min_primary - (float)track_handle_gap;
  float inactive_track_min_primary =
      thumb_max_primary + (float)track_handle_gap;
  return SliderVisualMetrics{thumb_center_primary,
                             track_cross_start,
                             track_min_cross,
                             track_max_cross,
                             thumb_min_primary,
                             thumb_max_primary,
                             thumb_cross_start,
                             thumb_min_cross,
                             thumb_max_cross,
                             active_track_max_primary,
                             inactive_track_min_primary};
}

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows