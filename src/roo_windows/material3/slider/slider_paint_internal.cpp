#include "roo_windows/material3/slider/slider_paint_internal.h"

#include <cmath>

#include "roo_display/shape/smooth.h"

namespace roo_windows {
namespace material3 {
namespace internal {

namespace {

roo_display::RoundRectRadii TrackSegmentRadii(
    const SliderAxisMetrics& axis, const SliderPaintStopSegment& segment,
    int16_t track_radius) {
  float start_radius = SegmentTouchesBoundary(axis, segment, true)
                           ? (float)track_radius
                           : (float)kTrackInnerEndRadiusPixels;
  float end_radius = SegmentTouchesBoundary(axis, segment, false)
                         ? (float)track_radius
                         : (float)kTrackInnerEndRadiusPixels;
  bool start_maps_to_low_display =
      axis.displayPrimary(0.0f) <=
      axis.displayPrimary((float)(axis.primarySpan() - 1));
  if (!axis.isVertical()) {
    float left_radius = start_maps_to_low_display ? start_radius : end_radius;
    float right_radius = start_maps_to_low_display ? end_radius : start_radius;
    return roo_display::RoundRectRadii{left_radius, right_radius, left_radius,
                                       right_radius};
  }
  float top_radius = start_maps_to_low_display ? start_radius : end_radius;
  float bottom_radius = start_maps_to_low_display ? end_radius : start_radius;
  return roo_display::RoundRectRadii{top_radius, top_radius, bottom_radius,
                                     bottom_radius};
}

}  // namespace

roo_display::Box TrackSegmentClipBox(const SliderAxisMetrics& axis,
                                     const SliderPaintStopSegment& segment,
                                     int16_t cross_start, int16_t cross_span) {
  int16_t min_primary = (int16_t)ceilf(segment.min_primary);
  int16_t max_primary = (int16_t)floorf(segment.max_primary);
  if (min_primary < 0) min_primary = 0;
  if (max_primary >= axis.primarySpan()) max_primary = axis.primarySpan() - 1;
  if (max_primary < min_primary || cross_span <= 0) {
    return roo_display::Box(0, 0, -1, -1);
  }
  int16_t cross_end = cross_start + cross_span - 1;
  return axis.boxFromPrimaryCross(min_primary, cross_start, max_primary,
                                  cross_end);
}

void PaintTrackSegments(const Canvas& canvas, const Rect& widget_bounds,
                        const SliderAxisMetrics& axis,
                        const SliderPaintStopSegment* segments,
                        int segment_count, const TrackCrossBand& cross_band,
                        int16_t track_radius, const SliderPaintTokens& tokens) {
  for (int i = 0; i < segment_count; ++i) {
    roo_display::Box track_clip =
        TrackSegmentClipBox(axis, segments[i], cross_band.track_cross_start,
                            cross_band.track_cross_span);
    if (track_clip.empty()) continue;
    SliderAxisMetrics::PaintRect track_bounds = axis.paintRectFromPrimaryCross(
        segments[i].min_primary, cross_band.track_min_cross,
        segments[i].max_primary, cross_band.track_max_cross);
    auto track_piece = roo_display::SmoothFilledRoundRect(
        track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
        track_bounds.y_max, TrackSegmentRadii(axis, segments[i], track_radius),
        TrackColorForSegment(tokens, segments[i]));
    DrawTrackPiece(canvas, track_piece, widget_bounds, track_clip,
                   axis.isVertical());
  }
}

namespace {

// True when a stop center belongs to the specified segment, allowing for small
// floating-point tolerance at segment boundaries.
bool SegmentContains(const SliderPaintStopSegment& segment, float primary) {
  return primary >= segment.min_primary - 0.01f &&
         primary <= segment.max_primary + 0.01f;
}

// Converts a stop's primary-axis center into a point centered on the track,
// independent of slider orientation.
roo_display::FpPoint StopMarkCenter(const SliderAxisMetrics& axis,
                                    float primary_center) {
  float cross_center = 0.5f * (float)axis.crossSpan() - 0.5f;
  float display_primary = axis.displayPrimary(primary_center);
  if (axis.isVertical()) {
    return roo_display::FpPoint{cross_center, display_primary};
  }
  return roo_display::FpPoint{display_primary, cross_center};
}

// Converts stop extents back into primary-axis coordinates so neighboring stop
// marks can bridge only the uncovered pixels between them.
SliderPaintStopSpan StopSpanFromExtents(const SliderAxisMetrics& axis,
                                        const roo_display::Box& stop_extents) {
  int16_t display_min_primary =
      axis.isVertical() ? stop_extents.yMin() : stop_extents.xMin();
  int16_t display_max_primary =
      axis.isVertical() ? stop_extents.yMax() : stop_extents.xMax();
  int16_t primary_a = (int16_t)axis.displayPrimary(display_min_primary);
  int16_t primary_b = (int16_t)axis.displayPrimary(display_max_primary);
  return SliderPaintStopSpan{std::min(primary_a, primary_b),
                             std::max(primary_a, primary_b)};
}

struct StopCrossSpan {
  int16_t min_cross;
  int16_t max_cross;
};

StopCrossSpan StopCrossSpanFromExtents(const SliderAxisMetrics& axis,
                                       const roo_display::Box& stop_extents) {
  return axis.isVertical()
             ? StopCrossSpan{stop_extents.xMin(), stop_extents.xMax()}
             : StopCrossSpan{stop_extents.yMin(), stop_extents.yMax()};
}

// Expands a segment run to include the exact pixels covered by one stop mark.
void IncludeStopExtentsInRun(const SliderAxisMetrics& axis,
                             const roo_display::Box& stop_extents,
                             SliderPaintStopRun& run) {
  SliderPaintStopSpan span = StopSpanFromExtents(axis, stop_extents);
  if (!run.has_marks) {
    run.has_marks = true;
    run.min_primary = span.min_primary;
    run.max_primary = span.max_primary;
    return;
  }
  if (span.min_primary < run.min_primary) run.min_primary = span.min_primary;
  if (span.max_primary > run.max_primary) run.max_primary = span.max_primary;
}

// Rebuilds the clip/exclusion box that covers the settled stop run for one
// segment.
roo_display::Box StopRunBox(const SliderAxisMetrics& axis,
                            const SliderPaintStopRun& run,
                            const StopCrossSpan& cross_span) {
  return axis.boxFromPrimaryCross(run.min_primary, cross_span.min_cross,
                                  run.max_primary, cross_span.max_cross);
}

// Computes stop extents without committing to a color so both passes can share
// the same geometry.
roo_display::Box StopMarkExtents(const SliderAxisMetrics& axis,
                                 float primary_center) {
  return roo_display::SmoothFilledCircle(StopMarkCenter(axis, primary_center),
                                         kStopMarkRadiusPixels,
                                         roo_display::Color())
      .extents();
}

StopCrossSpan ResolveStopCrossSpan(const SliderAxisMetrics& axis) {
  float primary_center = 0.5f * (float)(axis.primarySpan() - 1);
  return StopCrossSpanFromExtents(axis, StopMarkExtents(axis, primary_center));
}

// Converts a discrete stop index into a primary-axis center using the slider's
// logical range and orientation-aware axis geometry.
float StopPrimaryCenterFromIndex(const SliderAxisMetrics& axis,
                                 const SliderRange& range, int32_t stop_count,
                                 int32_t i) {
  float value = (i == stop_count) ? range.to : range.from + i * range.step;
  return axis.centerFromValue(range, value);
}

}  // namespace

void PaintStopRuns(const Canvas& canvas, Clipper& clipper,
                   const SliderPaintTokens& tokens,
                   const SliderAxisMetrics& axis,
                   const SliderPaintStopSegment* segments, int segment_count,
                   const SliderRange& range, bool render_active_segments,
                   const StopSuppressionFn& suppress_stop) {
  if (segment_count <= 0) return;
  StopCrossSpan stop_cross_span = ResolveStopCrossSpan(axis);
  int32_t stop_count =
      IsDiscreteSliderRange(range.step)
          ? (int32_t)lroundf((range.to - range.from) / range.step)
          : 1;

  SliderPaintStopRun runs[3];
  for (int32_t i = 0; i <= stop_count; ++i) {
    float primary_center =
        StopPrimaryCenterFromIndex(axis, range, stop_count, i);
    for (int segment_index = 0; segment_index < segment_count;
         ++segment_index) {
      if (!SegmentPaintsStopMarks(segments[segment_index],
                                  render_active_segments)) {
        continue;
      }
      if (!SegmentContains(segments[segment_index], primary_center)) continue;
      roo_display::Box stop_extents = StopMarkExtents(axis, primary_center);
      if (suppress_stop && suppress_stop(stop_extents)) break;
      IncludeStopExtentsInRun(axis, stop_extents, runs[segment_index]);
      break;
    }
  }

  for (int segment_index = 0; segment_index < segment_count; ++segment_index) {
    if (!SegmentPaintsStopMarks(segments[segment_index],
                                render_active_segments)) {
      continue;
    }
    if (!runs[segment_index].has_marks) continue;
    Color track_color = TrackColorForSegment(tokens, segments[segment_index]);
    Color stop_color = StopColorForSegment(tokens, segments[segment_index]);
    roo_display::Box segment_clip_box = TrackSegmentClipBox(
        axis, segments[segment_index], stop_cross_span.min_cross,
        stop_cross_span.max_cross - stop_cross_span.min_cross + 1);
    if (segment_clip_box.empty()) continue;
    roo_display::Box run_box = roo_display::Box::Intersect(
        StopRunBox(axis, runs[segment_index], stop_cross_span),
        segment_clip_box);
    if (run_box.empty()) continue;

    Canvas stop_canvas = canvas;
    stop_canvas.set_bgcolor(track_color);
    stop_canvas.clip(segment_clip_box.translate(canvas.dx(), canvas.dy()));
    bool has_previous_stop = false;
    SliderPaintStopSpan previous_span{0, -1};
    for (int32_t i = 0; i <= stop_count; ++i) {
      float primary_center =
          StopPrimaryCenterFromIndex(axis, range, stop_count, i);
      if (!SegmentContains(segments[segment_index], primary_center)) continue;

      roo_display::Box stop_extents = StopMarkExtents(axis, primary_center);
      if (suppress_stop && suppress_stop(stop_extents)) continue;
      SliderPaintStopSpan current_span =
          StopSpanFromExtents(axis, stop_extents);
      if (has_previous_stop) {
        int16_t gap_min_primary = previous_span.max_primary + 1;
        int16_t gap_max_primary = current_span.min_primary - 1;
        if (gap_max_primary >= gap_min_primary) {
          stop_canvas.fillRect(axis.boxFromPrimaryCross(
                                   gap_min_primary, stop_cross_span.min_cross,
                                   gap_max_primary, stop_cross_span.max_cross),
                               track_color);
        }
      }

      stop_canvas.drawObject(
          roo_display::SmoothFilledCircle(StopMarkCenter(axis, primary_center),
                                          kStopMarkRadiusPixels, stop_color));
      previous_span = current_span;
      has_previous_stop = true;
    }

    roo_display::Box run_device_box = roo_display::Box::Intersect(
        run_box.translate(canvas.dx(), canvas.dy()), canvas.clip_box());
    if (!run_device_box.empty()) {
      clipper.addExclusion(run_device_box);
    }
  }
}

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows