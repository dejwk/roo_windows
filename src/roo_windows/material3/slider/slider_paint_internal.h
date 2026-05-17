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

// Stop marks are only meaningful for discrete sliders and can still be hidden
// explicitly through the style.
template <typename SliderLike>
inline bool ShouldRenderStops(const SliderLike& widget) {
  if (!IsDiscreteSliderRange(widget.range().step)) return false;
  return widget.style().tick_mode != SliderTickMode::kHidden;
}

// Extends the rounded track tile slightly beyond the first visible pixel so the
// end cap still lands cleanly when the segment begins inside the clip.
inline float TrackShapeMinPrimary(float visible_min_primary,
                                  int16_t track_radius) {
  return visible_min_primary <= 0.0f
             ? -0.5f
             : visible_min_primary - (float)track_radius;
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

inline Color TrackColorForSegment(const SliderPaintTokens& tokens,
                                  const SliderPaintStopSegment& segment) {
  return segment.active ? tokens.active_track : tokens.inactive_track;
}

inline Color StopColorForSegment(const SliderPaintTokens& tokens,
                                 const SliderPaintStopSegment& segment) {
  return segment.active ? tokens.active_stop : tokens.inactive_stop;
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
                   const SliderRange& range,
                   const StopSuppressionFn& suppress_stop);

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows