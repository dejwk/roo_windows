#pragma once

namespace roo_windows::material3::internal {

/// Continuous visible interval, before direction mapping and rasterization.
struct ProgressInterval {
  float start = 0;
  float end = 0;
};

/// Bounded foreground partition of a linear track.
struct LinearProgressGeometry {
  ProgressInterval active[2];
  ProgressInterval track[3];
  ProgressInterval stop;
  int active_count = 0;
  int track_count = 0;
};

/// Centerline arc geometry in radians, clockwise from twelve o'clock.
struct ProgressArc {
  float start = 0;
  float end = 0;
  float thickness = 0;
};

/// Evaluates the standard 1800 ms disjoint linear waveform.
LinearProgressGeometry LinearIndeterminate(float width, float gap,
                                           unsigned phase_ms);

/// Evaluates the standard 5400 ms advancing circular waveform (radians).
ProgressInterval CircularIndeterminate(unsigned phase_ms);

/// Computes active, gap, inactive and stop domains in logical coordinates.
LinearProgressGeometry LinearDeterminate(float width, float thickness,
                                         float gap, float progress);

/// Subtracts the union of active intervals and their clear gaps from the track.
LinearProgressGeometry LinearSegments(float width, float gap,
                                      ProgressInterval first,
                                      ProgressInterval second = {});

/// Fits rounded caps inside a visible angular interval, shrinking tiny arcs.
ProgressArc FitProgressArc(float start, float end, float radius,
                           float thickness);

/// Computes the inactive circular interval after both visible clear gaps.
ProgressArc CircularTrack(float progress, float radius, float thickness,
                          float gap);

}  // namespace roo_windows::material3::internal
