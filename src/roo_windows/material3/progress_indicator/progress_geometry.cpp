#include "roo_windows/material3/progress_indicator/progress_geometry.h"

#include <algorithm>
#include <cmath>

namespace roo_windows::material3::internal {
namespace {
constexpr float kTurn = 6.283185307179586f;
}

LinearProgressGeometry LinearSegments(float width, float gap,
                                      ProgressInterval first,
                                      ProgressInterval second) {
  LinearProgressGeometry result;
  if (second.start < first.start) std::swap(first, second);
  for (ProgressInterval interval : {first, second}) {
    interval.start = std::max(0.0f, std::min(width, interval.start));
    interval.end = std::max(interval.start, std::min(width, interval.end));
    if (interval.end <= interval.start) continue;
    if (result.active_count && interval.start <= result.active[0].end) {
      result.active[0].end = std::max(result.active[0].end, interval.end);
    } else {
      result.active[result.active_count++] = interval;
    }
  }
  float cursor = 0;
  for (int i = 0; i < result.active_count; ++i) {
    float end = std::max(0.0f, result.active[i].start - gap);
    if (end > cursor) result.track[result.track_count++] = {cursor, end};
    cursor = std::max(cursor, std::min(width, result.active[i].end + gap));
  }
  if (cursor < width) result.track[result.track_count++] = {cursor, width};
  return result;
}

LinearProgressGeometry LinearDeterminate(float width, float thickness,
                                         float gap, float progress) {
  LinearProgressGeometry result =
      LinearSegments(width, gap, {0, progress * width});
  if (progress < 1 && result.track_count) {
    // The stop is clipped by the same continuous residual domain as the track.
    result.stop = {std::max(result.track[0].start, width - thickness), width};
  }
  return result;
}

ProgressArc FitProgressArc(float start, float end, float radius,
                           float thickness) {
  if (radius <= 0 || end <= start) return {};
  float sweep = end - start;
  if (sweep >= kTurn) return {start, start + kTurn, thickness};
  // A cap subtends asin(cap_radius / centerline_radius). Keep both cap
  // edges inside the requested sweep, including at tiny positive progress.
  float cap =
      std::min(thickness * 0.5f,
               radius * std::sin(std::min(sweep * 0.25f, 1.57079632679f)));
  float offset = std::asin(std::min(1.0f, cap / radius));
  return {start + offset, end - offset, 2 * cap};
}

ProgressArc CircularTrack(float progress, float radius, float thickness,
                          float gap) {
  if (progress <= 0) return {0, kTurn, thickness};
  if (progress >= 1 || radius <= 0) return {};
  return FitProgressArc(progress * kTurn + gap / radius, kTurn - gap / radius,
                        radius, thickness);
}
}  // namespace roo_windows::material3::internal
