#include "roo_windows/material3/progress_indicator/progress_geometry.h"

#include <algorithm>
#include <cmath>

#include "roo_windows/core/animation_evaluator.h"

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
namespace {
/// Clamps channel time before applying the framework's bounded Bezier solve.
float Channel(float time, float delay, float duration, float x1, float x2) {
  float fraction = std::max(0.0f, std::min(1.0f, (time - delay) / duration));
  return ::roo_windows::internal::EvaluateEasing(
      {EasingKind::kCubicBezier, x1, 0, x2, 1}, fraction);
}
}  // namespace

LinearProgressGeometry LinearIndeterminate(float width, float gap,
                                           unsigned phase_ms) {
  float t = phase_ms % 1800;
  return LinearSegments(width, gap,
                        {width * Channel(t, 1267, 533, 0.2f, 0.8f),
                         width * Channel(t, 1000, 567, 0.4f, 1)},
                        {width * Channel(t, 333, 850, 0, 0.65f),
                         width * Channel(t, 0, 750, 0.1f, 0.45f)});
}

ProgressInterval CircularIndeterminate(unsigned phase_ms) {
  float t = phase_ms % 5400;
  float start = 1520 * t / 5400 - 20;
  float end = 1520 * t / 5400;
  for (int i = 0; i < 4; ++i) {
    start += 250 * Channel(t, 667 + 1350 * i, 667, 0.4f, 0.2f);
    end += 250 * Channel(t, 1350 * i, 667, 0.4f, 0.2f);
  }
  return {start * (kTurn / 360), end * (kTurn / 360)};
}

}  // namespace roo_windows::material3::internal
