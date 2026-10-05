// Compares rectangle-aware effects with the previous scalar application path.
// Uses thread CPU time and reports medians; timing is descriptive, not a test
// threshold. All buffers and effect records are allocated before measurement.
#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <vector>

#include "roo_windows/core/paint_effect.h"

namespace {

using roo_display::Box;
using roo_display::Color;
using roo_windows::PressOverlay;
using roo_windows::internal::PaintEffect;
using roo_windows::internal::PaintEffectStack;

volatile uint32_t checksum = 0;

uint64_t CpuNanos() {
  timespec now;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
  return uint64_t(now.tv_sec) * 1000000000 + now.tv_nsec;
}

// Preserves the pre-optimization uniform-source guard and per-pixel chain walk.
bool ScalarApply(const PaintEffect& effect, Box bounds, Color* pixels,
                 bool uniform) {
  Color tint;
  if (uniform &&
      PaintEffectStack(&effect).readUniformColorRect(
          bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax(), &tint)) {
    pixels[0] = roo_display::ApplyBlending(
        roo_display::BlendingMode::kSourceAtop, pixels[0], tint);
    return true;
  }
  if (uniform) roo_display::FillColor(pixels + 1, bounds.area() - 1, pixels[0]);
  Color* pixel = pixels;
  for (int y = bounds.yMin(); y <= bounds.yMax(); ++y) {
    for (int x = bounds.xMin(); x <= bounds.xMax(); ++x, ++pixel) {
      *pixel = PaintEffectStack(&effect, nullptr).apply(x, y, *pixel);
    }
  }
  for (int i = 1; i < bounds.area(); ++i) {
    if (pixels[i] != pixels[0]) return false;
  }
  return true;
}

bool Read(const PaintEffect& effect, Box bounds, Color* pixels, bool uniform,
          bool raster, bool bulk) {
  if (raster) {
    if (bulk) {
      return PaintEffectStack(&effect).readColorRect(
          bounds.xMin(), bounds.yMin(), bounds.xMax(), bounds.yMax(), pixels);
    }
    int i = 0;
    for (int y = bounds.yMin(); y <= bounds.yMax(); ++y) {
      for (int x = bounds.xMin(); x <= bounds.xMax(); ++x) {
        Color tint(0);
        for (const PaintEffect* e = &effect; e != nullptr; e = e->parent()) {
          if (!e->bounds().contains(x, y)) continue;
          tint = roo_display::AlphaBlend(
              tint, e->press() == nullptr ? e->tint() : e->press()->get(x, y));
        }
        pixels[i++] = tint;
      }
    }
    for (int j = 1; j < bounds.area(); ++j) {
      if (pixels[j] != pixels[0]) return false;
    }
    return true;
  }
  return bulk ? PaintEffectStack(&effect, nullptr)
                    .applyRect(bounds, pixels, uniform)
              : ScalarApply(effect, bounds, pixels, uniform);
}

// Each sample includes identical source-buffer preparation in both paths.
double Measure(const PaintEffect& effect, Box bounds,
               const std::vector<Color>& source, std::vector<Color>& pixels,
               bool uniform, bool raster, bool bulk) {
  const int iterations = std::max(128, 1000000 / bounds.area());
  std::array<double, 5> samples;
  for (double& sample : samples) {
    const uint64_t start = CpuNanos();
    for (int i = 0; i < iterations; ++i) {
      if (!raster) {
        if (uniform) {
          pixels[0] = source[0];
        } else {
          std::copy(source.begin(), source.end(), pixels.begin());
        }
      }
      const bool one =
          Read(effect, bounds, pixels.data(), uniform, raster, bulk);
      checksum += pixels[one ? 0 : i % bounds.area()].asArgb();
    }
    sample = (CpuNanos() - start) / (1000.0 * iterations);
  }
  std::sort(samples.begin(), samples.end());
  return samples[samples.size() / 2];
}

// Checks equivalence before measuring flat/ripple chains at two tile sizes.
bool Run(int width, int height, int depth, bool ripple, bool uniform,
         bool raster, bool partial) {
  const Box bounds(0, 0, width - 1, height - 1);
  const PressOverlay press(width / 2, height / 2, height / 2,
                           Color(0xA04080C0));
  std::vector<PaintEffect> effects;
  effects.reserve(depth);
  for (int i = 0; i < depth; ++i) {
    effects.emplace_back(
        i == 0 ? nullptr : &effects.back(),
        partial && i == depth - 1 ? Box(2, 1, width - 3, height - 2) : bounds,
        Color(40 + i * 9, 20, 180, 90),
        ripple && i == depth - 1 ? &press : nullptr);
  }
  const PaintEffect& effect = effects.back();
  std::vector<Color> source(bounds.area());
  for (int i = 0; i < bounds.area(); ++i) {
    source[i] = Color(32 + i % 224, (i * 3) % 256, (i * 7) % 256, i % 256);
  }
  std::vector<Color> scalar = source;
  std::vector<Color> bulk = source;
  const bool scalar_one =
      Read(effect, bounds, scalar.data(), uniform, raster, false);
  const bool bulk_one =
      Read(effect, bounds, bulk.data(), uniform, raster, true);
  for (int i = 0; i < bounds.area(); ++i) {
    if (scalar[scalar_one ? 0 : i] != bulk[bulk_one ? 0 : i]) return false;
  }
  const double scalar_us =
      Measure(effect, bounds, source, scalar, uniform, raster, false);
  const double bulk_us =
      Measure(effect, bounds, source, bulk, uniform, raster, true);
  std::printf(
      "%s %dx%d depth=%d effect=%s scope=%s source=%s scalar_us=%.3f "
      "rect_us=%.3f speedup=%.2fx\n",
      raster ? "raster" : "apply", width, height, depth,
      ripple ? "ripple" : "flat", partial ? "partial" : "full",
      raster    ? "-"
      : uniform ? "uniform"
                : "varying",
      scalar_us, bulk_us, scalar_us / bulk_us);
  return true;
}

}  // namespace

int main() {
  for (int depth : {1, 4}) {
    for (bool ripple : {false, true}) {
      for (int width : {8, 64}) {
        const int height = width == 8 ? 8 : 32;
        for (bool partial : {false, true}) {
          if (!Run(width, height, depth, ripple, false, false, partial))
            return 1;
          if (!Run(width, height, depth, ripple, true, false, partial))
            return 1;
          if (!Run(width, height, depth, ripple, false, true, partial))
            return 1;
        }
      }
    }
  }
}
