#pragma once

#include <functional>

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
inline constexpr int16_t kTrackInnerEndRadiusPixels = Scaled(2);

inline Rect ExpandRectAlongPrimary(const SliderAxisMetrics& axis, Rect rect,
                                   int16_t amount) {
  if (rect.empty() || amount <= 0) return rect;
  if (!axis.isVertical()) {
    return Rect(std::max<int16_t>(0, rect.xMin() - amount), rect.yMin(),
                std::min<int16_t>(axis.primarySpan() - 1, rect.xMax() + amount),
                rect.yMax());
  }
  return Rect(rect.xMin(), std::max<int16_t>(0, rect.yMin() - amount),
              rect.xMax(),
              std::min<int16_t>(axis.primarySpan() - 1, rect.yMax() + amount));
}

// Applies the disabled Material 3 alpha treatment on top of the current
// surface color so disabled slider pieces still blend with the theme.
inline Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

// Shared slider palette after resolving enabled vs disabled treatment.
struct SliderPaintTokens {
  Color active_track;
  Color inactive_track;
  Color handle;
  Color active_stop;
  Color inactive_stop;
};

// Resolves the effective track, thumb, and stop colors for the current state.
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

// Stop indicators can remain visible on any slider when explicitly enabled.
// Discrete sliders also render step marks when tick rendering is requested,
// even if explicit stop indicators are disabled.
template <typename SliderLike>
inline bool ShouldRenderStops(const SliderLike& widget) {
  if (widget.style().show_stop_indicators) return true;
  return IsDiscreteSliderRange(widget.range().step) &&
         widget.style().tick_mode != SliderTickMode::kHidden;
}

template <typename SliderLike>
inline bool ShouldRenderTicks(const SliderLike& widget) {
  return IsDiscreteSliderRange(widget.range().step) &&
         widget.style().tick_mode != SliderTickMode::kHidden;
}

// Narrows the handle in the pressed state without changing the allocated
// slider bounds.
inline int16_t ThumbWidthForState(bool pressed) {
  if (!pressed) return kHandleWidth;
  int16_t width =
      (int16_t)roundf((float)kHandleWidth * kPressedThumbWidthRatio);
  return width < 1 ? 1 : width;
}

// Adjusts the painted track/thumb clearance so a narrower pressed handle keeps
// the same perceived gap.
inline int16_t TrackGapForThumbWidth(int16_t thumb_width) {
  int16_t gap = kTrackHandleGap - (kHandleWidth - thumb_width) / 2;
  return gap < 0 ? 0 : gap;
}

// Restricts drawTiled() to the dirty primary-axis run while still spanning the
// full cross-axis of the slider.
inline Rect GetTrackTileBounds(const Rect& widget_bounds,
                               const roo_display::Box& clip, bool vertical) {
  if (vertical) {
    return Rect(widget_bounds.xMin(), clip.yMin(), widget_bounds.xMax(),
                clip.yMax());
  }
  return Rect(clip.xMin(), widget_bounds.yMin(), clip.xMax(),
              widget_bounds.yMax());
}

// Expands handle redraw just enough to include the visual gap that belongs to
// the thumb silhouette.
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

// Tiles a rounded track piece only across the dirty slice for one segment.
inline void DrawTrackPiece(const Canvas& canvas,
                           const roo_display::Drawable& piece,
                           const Rect& widget_bounds,
                           const roo_display::Box& clip_bounds, bool vertical) {
  if (clip_bounds.empty()) return;
  Rect tile_bounds = GetTrackTileBounds(widget_bounds, clip_bounds, vertical);
  canvas.drawTiled(piece, tile_bounds, roo_display::kNoAlign);
}

// Builds the orientation- and direction-aware axis helper used by both slider
// flavors for value/position conversion and local geometry.
template <typename SliderLike>
inline SliderAxisMetrics MakeSliderAxisMetrics(const SliderLike& slider) {
  SliderStyle style = slider.style();
  return SliderAxisMetrics(slider.width(), slider.height(),
                           style.orientation == SliderOrientation::kVertical,
                           IsSliderDirectionInverted(style));
}

// Turns the cached main-axis value-indicator span into a conservative local
// dirty rect for parent invalidation.
inline Rect IndicatorDirtyRectFromSpan(const DirtySpan& span, int16_t width,
                                       int16_t height, SliderStyle style) {
  if (span.empty() || width <= 0 || height <= 0) return Rect(0, 0, -1, -1);
  Rect conservative = ValueIndicatorBubble::ConservativeBounds(
      width, height, kHandleWidth, style.value_indicator, style.orientation);
  if (conservative.empty()) return Rect(0, 0, -1, -1);
  if (style.orientation == SliderOrientation::kVertical) {
    return Rect(conservative.xMin(), span.min_coord, conservative.xMax(),
                span.max_coord);
  }
  return Rect(span.min_coord, conservative.yMin(), span.max_coord,
              conservative.yMax());
}

// Describes one contiguous active or inactive track segment.
struct SliderPaintStopSegment {
  float min_primary;
  float max_primary;
  bool active;
};

inline bool SegmentTouchesBoundary(const SliderAxisMetrics& axis,
                                   const SliderPaintStopSegment& segment,
                                   bool start_boundary) {
  int16_t min_primary = (int16_t)ceilf(segment.min_primary);
  int16_t max_primary = (int16_t)floorf(segment.max_primary);
  if (min_primary < 0) min_primary = 0;
  if (max_primary >= axis.primarySpan()) max_primary = axis.primarySpan() - 1;
  if (max_primary < min_primary) return false;
  return start_boundary ? min_primary == 0
                        : max_primary == axis.primarySpan() - 1;
}

inline bool HasBoundarySegment(const SliderAxisMetrics& axis,
                               const SliderPaintStopSegment* segments,
                               int segment_count, bool start_boundary) {
  for (int i = 0; i < segment_count; ++i) {
    if (SegmentTouchesBoundary(axis, segments[i], start_boundary)) {
      return true;
    }
  }
  return false;
}

inline bool BoundaryRadiusDependsOnWidth(const SliderAxisMetrics& axis,
                                         const SliderPaintStopSegment& segment,
                                         int16_t track_radius,
                                         bool start_boundary) {
  bool touches_start = SegmentTouchesBoundary(axis, segment, true);
  bool touches_end = SegmentTouchesBoundary(axis, segment, false);
  if (!(start_boundary ? touches_start : touches_end)) {
    return false;
  }
  float min_primary = segment.min_primary < 0.0f ? 0.0f : segment.min_primary;
  float max_primary = segment.max_primary;
  float max_allowed_primary = (float)(axis.primarySpan() - 1);
  if (max_primary > max_allowed_primary) {
    max_primary = max_allowed_primary;
  }
  if (max_primary < min_primary) return false;
  float start_radius =
      touches_start ? (float)track_radius : (float)kTrackInnerEndRadiusPixels;
  float end_radius =
      touches_end ? (float)track_radius : (float)kTrackInnerEndRadiusPixels;
  return max_primary - min_primary < start_radius + end_radius;
}

inline bool HasBoundaryDependentRadius(const SliderAxisMetrics& axis,
                                       const SliderPaintStopSegment* segments,
                                       int segment_count, int16_t track_radius,
                                       bool start_boundary) {
  for (int i = 0; i < segment_count; ++i) {
    if (BoundaryRadiusDependsOnWidth(axis, segments[i], track_radius,
                                     start_boundary)) {
      return true;
    }
  }
  return false;
}

inline Rect BoundaryTrackStripRect(const SliderAxisMetrics& axis,
                                   bool start_boundary) {
  int16_t primary = start_boundary ? 0 : axis.primarySpan() - 1;
  return Rect(
      axis.boxFromPrimaryCross(primary, 0, primary, axis.crossSpan() - 1));
}

// Unions `rect` with the boundary strips whose pixels can change when a short
// boundary-touching segment forces the round-rect radii to renormalize.
inline Rect IncludeBoundaryTrackStrips(
    Rect rect, const SliderAxisMetrics& axis,
    const SliderPaintStopSegment* old_segments, int old_segment_count,
    const SliderPaintStopSegment* new_segments, int new_segment_count,
    int16_t track_radius) {
  for (bool start_boundary : {true, false}) {
    if (HasBoundaryDependentRadius(axis, old_segments, old_segment_count,
                                   track_radius, start_boundary) ||
        HasBoundaryDependentRadius(axis, new_segments, new_segment_count,
                                   track_radius, start_boundary)) {
      rect = Rect::Extent(rect, BoundaryTrackStripRect(axis, start_boundary));
    }
  }
  return rect;
}

// Visual band the slider track occupies on the cross axis. Both flavors paint
// the same band, so both layouts (single-slider's only layout and either of
// the range slider's per-thumb layouts) carry identical values here.
struct TrackCrossBand {
  int16_t track_cross_start;
  int16_t track_cross_span;
  float track_min_cross;
  float track_max_cross;
};

// Renders every active/inactive run of `segments` using explicit per-end
// corner radii and tiles only the dirty slice so transparent corner pixels
// resolve against the widget background in the same pass.
void PaintTrackSegments(const Canvas& canvas, const Rect& widget_bounds,
                        const SliderAxisMetrics& axis,
                        const SliderPaintStopSegment* segments,
                        int segment_count, const TrackCrossBand& cross_band,
                        int16_t track_radius, const SliderPaintTokens& tokens);

inline Color TrackColorForSegment(const SliderPaintTokens& tokens,
                                  const SliderPaintStopSegment& segment) {
  return segment.active ? tokens.active_track : tokens.inactive_track;
}

inline Color StopColorForSegment(const SliderPaintTokens& tokens,
                                 const SliderPaintStopSegment& segment) {
  return segment.active ? tokens.active_stop : tokens.inactive_stop;
}

inline bool SegmentPaintsStopMarks(const SliderPaintStopSegment& segment,
                                   bool render_active_segments) {
  return render_active_segments || !segment.active;
}

// Converts a logical track segment into the concrete clip box used to paint it
// for a specific cross-axis band.
roo_display::Box TrackSegmentClipBox(const SliderAxisMetrics& axis,
                                     const SliderPaintStopSegment& segment,
                                     int16_t cross_start, int16_t cross_span);

// Accumulates the exact settled primary-axis span occupied by stop circles in a
// segment.
struct SliderPaintStopRun {
  bool has_marks = false;
  int16_t min_primary = 0;
  int16_t max_primary = -1;
};

// Primary-axis span covered by a single stop mark.
struct SliderPaintStopSpan {
  int16_t min_primary;
  int16_t max_primary;
};

// True when the full primary-axis span belongs to the specified segment.
inline bool SegmentContainsRange(const SliderPaintStopSegment& segment,
                                 float min_primary, float max_primary) {
  return min_primary >= segment.min_primary - 0.01f &&
         max_primary <= segment.max_primary + 0.01f;
}

using StopSuppressionFn = std::function<bool(const roo_display::Box&)>;

// Paints stop circles in two passes: first discover the exact span settled by
// the marks inside each segment, then draw only those marks and exclude the
// resulting run so later track rendering can tessellate around it. The caller
// only supplies slider-specific suppression for decorations that own part of
// the track, such as inset icons. An empty std::function disables suppression.
void PaintStopRuns(const Canvas& canvas, Clipper& clipper,
                   const SliderPaintTokens& tokens,
                   const SliderAxisMetrics& axis,
                   const SliderPaintStopSegment* segments, int segment_count,
                   const SliderRange& range, bool render_active_segments,
                   const StopSuppressionFn& suppress_stop);

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows