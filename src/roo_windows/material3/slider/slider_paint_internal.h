#pragma once

#include <cmath>

#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/material3/slider/slider_internal.h"
#include "roo_windows/material3/slider/slider_size_internal.h"
#include "roo_windows/material3/slider/value_indicator.h"

namespace roo_windows {
namespace material3 {
namespace internal {

inline constexpr float kPressedThumbWidthRatio = 0.5f;
inline constexpr int16_t kStopMarkRadiusPixels = Scaled(2);
inline constexpr int16_t kStopMarkSpanPixels = Scaled(4);

inline Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

struct SliderPaintTokens {
  Color active_track;
  Color inactive_track;
  Color handle;
  Color active_stop;
  Color inactive_stop;
};

template <typename SliderLike>
inline SliderPaintTokens ResolveTokens(const SliderLike& widget) {
  const Theme& theme = widget.theme();
  if (!widget.isEnabled()) {
    return SliderPaintTokens{
        DisabledComposite(theme.color.onSurface, 0x61, theme),
        DisabledComposite(theme.color.onSurface, 0x1F, theme),
        DisabledComposite(theme.color.onSurface, 0x61, theme),
        theme.color.surface,
        theme.color.surface,
    };
  }

  return SliderPaintTokens{
      theme.color.primary,
      theme.color.secondaryContainer,
      theme.color.primary,
      theme.color.onPrimary,
      theme.color.onSecondaryContainer,
  };
}

template <typename SliderLike>
inline bool ShouldRenderStops(const SliderLike& widget) {
  if (!IsDiscreteSliderRange(widget.range().step)) return false;
  return widget.style().tick_mode != SliderTickMode::kHidden;
}

inline float TrackShapeMinPrimary(float visible_min_primary,
                                  int16_t track_radius) {
  return visible_min_primary <= 0.0f
             ? -0.5f
             : visible_min_primary - (float)track_radius;
}

inline int16_t ThumbWidthForState(int16_t nominal_width, bool pressed) {
  if (!pressed) return nominal_width;
  int16_t width =
      (int16_t)roundf((float)nominal_width * kPressedThumbWidthRatio);
  return width < 1 ? 1 : width;
}

inline int16_t TrackGapForThumbWidth(const SliderSizeMetrics& size_metrics,
                                     int16_t thumb_width) {
  int16_t gap = size_metrics.track_handle_gap -
                (size_metrics.handle_width - thumb_width) / 2;
  return gap < 0 ? 0 : gap;
}

inline Rect GetTrackTileBounds(const Rect& widget_bounds,
                               const roo_display::Box& clip, bool vertical) {
  if (vertical) {
    return Rect(widget_bounds.xMin(), clip.yMin(), widget_bounds.xMax(),
                clip.yMax());
  }
  return Rect(clip.xMin(), widget_bounds.yMin(), clip.xMax(),
              widget_bounds.yMax());
}

inline Rect GetHandleTileBounds(const Rect& widget_bounds,
                                const roo_display::Box& handle_bounds,
                                int16_t track_gap, bool vertical) {
  Rect expanded =
      vertical ? Rect(widget_bounds.xMin(), handle_bounds.yMin() - track_gap,
                      widget_bounds.xMax(), handle_bounds.yMax() + track_gap)
               : Rect(handle_bounds.xMin() - track_gap, widget_bounds.yMin(),
                      handle_bounds.xMax() + track_gap, widget_bounds.yMax());
  return Rect::Intersect(expanded, widget_bounds);
}

inline void DrawTrackPiece(const Canvas& canvas,
                           const roo_display::Drawable& piece,
                           const Rect& widget_bounds,
                           const roo_display::Box& clip_bounds, bool vertical) {
  if (clip_bounds.empty()) return;
  Rect tile_bounds = GetTrackTileBounds(widget_bounds, clip_bounds, vertical);
  canvas.drawTiled(piece, tile_bounds, roo_display::kNoAlign);
}

template <typename SliderLike>
inline SliderAxisMetrics MakeSliderAxisMetrics(const SliderLike& slider) {
  const SliderSizeMetrics& size_metrics =
      ResolveSliderSizeMetrics(slider.style().size);
  return SliderAxisMetrics(
      slider.width(), slider.height(), size_metrics.handle_width,
      size_metrics.track_handle_gap,
      slider.style().orientation == SliderOrientation::kVertical);
}

inline Rect IndicatorDirtyRectFromSpan(const DirtySpan& span, int16_t width,
                                       int16_t height, SliderStyle style) {
  if (span.empty() || width <= 0 || height <= 0) return Rect(0, 0, -1, -1);
  const SliderSizeMetrics& size_metrics = ResolveSliderSizeMetrics(style.size);
  Rect conservative = ValueIndicatorBubble::ConservativeBounds(
      width, height, size_metrics.handle_width, style.value_indicator,
      style.orientation);
  if (conservative.empty()) return Rect(0, 0, -1, -1);
  if (style.orientation == SliderOrientation::kVertical) {
    return Rect(conservative.xMin(), span.min_coord, conservative.xMax(),
                span.max_coord);
  }
  return Rect(span.min_coord, conservative.yMin(), span.max_coord,
              conservative.yMax());
}

struct SliderPaintStopSegment {
  float min_primary;
  float max_primary;
  Color track_color;
  Color stop_color;
};

struct SliderPaintStopRun {
  bool has_marks = false;
  int16_t min_primary = 0;
  int16_t max_primary = -1;
};

struct SliderPaintStopSpan {
  int16_t min_primary;
  int16_t max_primary;
};

inline roo_display::Box StopSegmentClipBox(
    const SliderAxisMetrics& axis, const SliderPaintStopSegment& segment) {
  int16_t min_primary = (int16_t)ceilf(segment.min_primary);
  int16_t max_primary = (int16_t)floorf(segment.max_primary);
  if (min_primary < 0) min_primary = 0;
  if (max_primary >= axis.primarySpan()) max_primary = axis.primarySpan() - 1;
  if (max_primary < min_primary) return roo_display::Box(0, 0, -1, -1);
  int16_t cross_start = (axis.crossSpan() - kStopMarkSpanPixels) / 2;
  return axis.boxFromPrimaryCross(min_primary, cross_start, max_primary,
                                  cross_start + kStopMarkSpanPixels - 1);
}

inline bool SegmentContains(const SliderPaintStopSegment& segment,
                            float primary) {
  return primary >= segment.min_primary - 0.01f &&
         primary <= segment.max_primary + 0.01f;
}

inline bool SegmentContainsRange(const SliderPaintStopSegment& segment,
                                 float min_primary, float max_primary) {
  return min_primary >= segment.min_primary - 0.01f &&
         max_primary <= segment.max_primary + 0.01f;
}

inline roo_display::FpPoint StopMarkCenter(const SliderAxisMetrics& axis,
                                           float primary_center) {
  float cross_center = 0.5f * (float)axis.crossSpan() - 0.5f;
  if (axis.isVertical()) {
    return roo_display::FpPoint{cross_center,
                                axis.displayPrimary(primary_center)};
  }
  return roo_display::FpPoint{primary_center, cross_center};
}

inline void IncludeStopExtentsInRun(const SliderAxisMetrics& axis,
                                    const roo_display::Box& stop_extents,
                                    SliderPaintStopRun& run) {
  SliderPaintStopSpan span =
      axis.isVertical()
          ? SliderPaintStopSpan{(int16_t)(axis.primarySpan() - 1 -
                                          stop_extents.yMax()),
                                (int16_t)(axis.primarySpan() - 1 -
                                          stop_extents.yMin())}
          : SliderPaintStopSpan{stop_extents.xMin(), stop_extents.xMax()};
  if (!run.has_marks) {
    run.has_marks = true;
    run.min_primary = span.min_primary;
    run.max_primary = span.max_primary;
    return;
  }
  if (span.min_primary < run.min_primary) run.min_primary = span.min_primary;
  if (span.max_primary > run.max_primary) run.max_primary = span.max_primary;
}

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows