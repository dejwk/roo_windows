// Host-only decomposition of direct-query, complement, and filter CPU work.
// Timings are descriptive and use a counting output with no pixel memory
// writes.
#include <array>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <vector>

#include "roo_display/core/offscreen.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/damage_region.h"
#include "roo_windows/core/exclusion_filter.h"

namespace roo_windows::internal {

struct DamageRegionRenderTestPeer {
  static bool prepared(const ClipperOutput& out) {
    return out.exclusions_valid_;
  }
};

}  // namespace roo_windows::internal

namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
using roo_windows::internal::ClipperOutput;
using roo_windows::internal::ClipperState;
using roo_windows::internal::DamageRegion;
using roo_windows::internal::DamageRegionRenderTestPeer;
using roo_windows::internal::ExclusionFilter;
using roo_windows::internal::ExclusionUnion;

volatile uint64_t g_checksum = 0;

uint64_t CpuNanos() {
  timespec now;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
  return static_cast<uint64_t>(now.tv_sec) * 1000000000 + now.tv_nsec;
}

/// Counts output calls and pixels without touching the framebuffer.
class CountingDevice final : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit CountingDevice(roo::byte* pixels)
      : OffscreenDevice(800, 480, pixels, Argb8888()) {}

  void fillRects(BlendingMode, Color, int16_t* x0, int16_t* y0, int16_t* x1,
                 int16_t* y1, uint16_t count) override {
    ++commands;
    for (uint16_t i = 0; i < count; ++i) {
      writes += static_cast<uint64_t>(x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1);
    }
  }

  uint64_t writes = 0;
  uint64_t commands = 0;
};

struct Scene {
  const char* name;
  std::array<Box, 5> boxes;
  int count;
};

/// Measures the three independent CPU stages for a fixed region.
void Measure(const Scene& scene, std::vector<roo::byte>& pixels) {
  DamageRegion damage;
  for (int i = 0; i < scene.count; ++i) damage.add(scene.boxes[i]);
  const Box bounds = damage.bounds();
  const Box gap_query(400, 240, 410, 250);
  const Box hit_query = damage.at(0);
  constexpr int kQueryIterations = 100000;
  constexpr int kComplementIterations = 10000;
  constexpr int kFilterIterations = 1000;
  constexpr int kPrepareIterations = 1000;
  int rejected = 0;
  uint64_t start = CpuNanos();
  for (int i = 0; i < kQueryIterations; ++i) {
    const Box result = damage.intersectionBounds(gap_query);
    rejected += result.empty();
    g_checksum += result.xMin();
  }
  const uint64_t gap_query_ns = (CpuNanos() - start) / kQueryIterations;
  start = CpuNanos();
  for (int i = 0; i < kQueryIterations; ++i) {
    const Box result = damage.intersectionBounds(hit_query);
    g_checksum += result.xMax();
  }
  const uint64_t hit_query_ns = (CpuNanos() - start) / kQueryIterations;

  std::vector<Box> gaps;
  gaps.reserve(45);
  start = CpuNanos();
  for (int i = 0; i < kComplementIterations; ++i) {
    gaps.clear();
    damage.appendComplement(bounds, gaps);
    g_checksum += gaps.size();
  }
  const uint64_t complement_ns = (CpuNanos() - start) / kComplementIterations;

  CountingDevice output(pixels.data());
  ClipperState state;
  int prepared_calls = 0;
  start = CpuNanos();
  for (int i = 0; i < kPrepareIterations; ++i) {
    ClipperOutput clipped(state, output, &damage);
    g_checksum += clipped.prepareClip(bounds).xMin();
    prepared_calls += DamageRegionRenderTestPeer::prepared(clipped);
  }
  const uint64_t prepare_ns = (CpuNanos() - start) / kPrepareIterations;
  int rejected_clips = 0;
  int rejected_prepares = 0;
  start = CpuNanos();
  for (int i = 0; i < kPrepareIterations; ++i) {
    ClipperOutput clipped(state, output, &damage);
    rejected_clips += clipped.prepareClip(gap_query).empty();
    rejected_prepares += DamageRegionRenderTestPeer::prepared(clipped);
  }
  const uint64_t reject_ns = (CpuNanos() - start) / kPrepareIterations;

  const Box* begin = gaps.empty() ? nullptr : gaps.data();
  const Box* end = gaps.empty() ? nullptr : gaps.data() + gaps.size();
  ExclusionUnion exclusions(begin, end);
  ExclusionFilter filter(output, &exclusions);
  for (int i = 0; i < 10; ++i) {
    static_cast<roo_display::DisplayOutput&>(filter).fillRect(
        BlendingMode::kSource, bounds, Color(0xFF426281));
  }
  output.writes = 0;
  output.commands = 0;
  start = CpuNanos();
  for (int i = 0; i < kFilterIterations; ++i) {
    static_cast<roo_display::DisplayOutput&>(filter).fillRect(
        BlendingMode::kSource, bounds, Color(0xFF426281));
  }
  const uint64_t filter_ns = (CpuNanos() - start) / kFilterIterations;
  std::printf(
      "P8_CPU name=%s members=%u gaps=%zu scratch_capacity=%zu "
      "gap_query_ns=%llu hit_query_ns=%llu rejected=%d "
      "complement_ns=%llu prep_ns=%llu prep_calls=%d "
      "reject_ns=%llu rejected_clips=%d rejected_prepares=%d "
      "filter_ns=%llu output_pixels=%llu "
      "output_commands=%llu\n",
      scene.name, damage.size(), gaps.size(), gaps.capacity(),
      static_cast<unsigned long long>(gap_query_ns),
      static_cast<unsigned long long>(hit_query_ns), rejected,
      static_cast<unsigned long long>(complement_ns),
      static_cast<unsigned long long>(prepare_ns), prepared_calls,
      static_cast<unsigned long long>(reject_ns), rejected_clips,
      rejected_prepares, static_cast<unsigned long long>(filter_ns),
      static_cast<unsigned long long>(output.writes / kFilterIterations),
      static_cast<unsigned long long>(output.commands / kFilterIterations));
}

}  // namespace

int main() {
  std::vector<roo::byte> pixels(800 * 480 * 4);
  const Box empty(0, 0, -1, -1);
  const std::array<Scene, 5> scenes = {{
      {"single", {Box(200, 100, 299, 199), empty, empty, empty, empty}, 1},
      {"corners",
       {Box(10, 10, 19, 19), Box(780, 460, 789, 469), empty, empty, empty},
       2},
      {"four",
       {Box(10, 10, 19, 19), Box(780, 10, 789, 19), Box(10, 460, 19, 469),
        Box(780, 460, 789, 469), empty},
       4},
      {"overlap",
       {Box(180, 120, 279, 219), Box(230, 170, 329, 269), empty, empty, empty},
       2},
      {"fifth",
       {Box(10, 10, 19, 19), Box(200, 110, 209, 119), Box(400, 210, 409, 219),
        Box(600, 310, 609, 319), Box(780, 460, 789, 469)},
       5},
  }};
  for (const Scene& scene : scenes) Measure(scene, pixels);
  std::printf("P8_CHECKSUM %llu\n",
              static_cast<unsigned long long>(g_checksum));
}
