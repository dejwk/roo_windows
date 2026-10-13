// Benchmark-only host comparison of compiled gaps and direct damage output.
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <vector>

#include "benchmarks/native_damage_filter.h"
#include "benchmarks/native_damage_scenes.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/rounded_clip.h"

namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
using roo_windows::BorderStyle;
using roo_windows::benchmarks::BuildNativeDamage;
using roo_windows::benchmarks::DrawNativeDamageScene;
using roo_windows::benchmarks::NativeDamageFilter;
using roo_windows::benchmarks::NativeDamageScene;
using roo_windows::benchmarks::NativeDamageScenes;
using roo_windows::internal::DamageRegion;
using roo_windows::internal::ExclusionFilter;
using roo_windows::internal::ExclusionUnion;
using roo_windows::internal::MaskedExclusion;
using roo_windows::internal::RoundedClip;

uint64_t CpuNanos() {
  timespec now;
  clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now);
  return static_cast<uint64_t>(now.tv_sec) * 1000000000 + now.tv_nsec;
}

/// Counts output without writing to the offscreen's backing pixel buffer.
class CountingDevice final : public roo_display::OffscreenDevice<Argb8888> {
 public:
  /// Creates a counting output using @p pixels only for its color format.
  explicit CountingDevice(roo::byte* pixels)
      : OffscreenDevice(800, 480, pixels, Argb8888()) {}

  /// Clears per-frame output counts.
  void reset() {
    pixels = 0;
    commands = 0;
    addresses = 0;
  }

  /// Counts a streamed address window.
  void setAddress(uint16_t, uint16_t, uint16_t, uint16_t,
                  BlendingMode) override {
    ++addresses;
  }

  /// Counts streamed colors.
  void write(Color*, uint32_t count) override {
    pixels += count;
    ++commands;
  }

  /// Counts a streamed fill.
  void fill(Color, uint32_t count) override {
    pixels += count;
    ++commands;
  }

  /// Counts sparse colors.
  void writePixels(BlendingMode, Color*, int16_t*, int16_t*,
                   uint16_t count) override {
    pixels += count;
    ++commands;
  }

  /// Counts sparse fills.
  void fillPixels(BlendingMode, Color, int16_t*, int16_t*,
                  uint16_t count) override {
    pixels += count;
    ++commands;
  }

  /// Counts colored rectangles.
  void writeRects(BlendingMode, Color*, int16_t* x0, int16_t* y0, int16_t* x1,
                  int16_t* y1, uint16_t count) override {
    CountRects(x0, y0, x1, y1, count);
  }

  /// Counts filled rectangles.
  void fillRects(BlendingMode, Color, int16_t* x0, int16_t* y0, int16_t* x1,
                 int16_t* y1, uint16_t count) override {
    CountRects(x0, y0, x1, y1, count);
  }

  uint64_t pixels = 0;
  uint64_t commands = 0;
  uint64_t addresses = 0;

 private:
  void CountRects(int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                  uint16_t count) {
    ++commands;
    for (uint16_t i = 0; i < count; ++i) {
      pixels += static_cast<uint64_t>(x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1);
    }
  }
};

struct Fixture {
  Fixture(const NativeDamageScene& scene, bool native)
      : damage(BuildNativeDamage(scene)),
        clip(damage.intersectionBounds(Box(0, 0, 799, 479))),
        semantic_box(40, 40, 65, 65),
        semantic_begin(scene.semantic_rect ? &semantic_box : nullptr),
        semantic_end(scene.semantic_rect ? &semantic_box + 1 : nullptr),
        masked{Box(280, 100, 520, 380), &rounded},
        masked_begin(scene.rounded_semantic ? &masked : nullptr),
        masked_end(scene.rounded_semantic ? &masked + 1 : nullptr) {
    if (scene.rounded_semantic) {
      rounded.reset(&rounded, masked.bounds, BorderStyle(32, 0));
    }
    if (!native) compiled_boxes.reserve(45);
  }

  DamageRegion damage;
  Box clip;
  Box semantic_box;
  const Box* semantic_begin;
  const Box* semantic_end;
  RoundedClip rounded;
  MaskedExclusion masked;
  const MaskedExclusion* masked_begin;
  const MaskedExclusion* masked_end;
  std::vector<Box> compiled_boxes;
};

/// Executes one scene through either adapter, preserving common draw inputs.
void RunFrame(bool native, const NativeDamageScene& scene, Fixture& fixture,
              CountingDevice& device, const roo::byte* direct_pixels) {
  device.reset();
  if (native) {
    ExclusionUnion semantic(fixture.semantic_begin, fixture.semantic_end);
    semantic.reset(fixture.semantic_begin, fixture.semantic_end,
                   fixture.masked_begin, fixture.masked_end);
    const bool has_semantic = fixture.semantic_begin != fixture.semantic_end ||
                              fixture.masked_begin != fixture.masked_end;
    NativeDamageFilter filter(device, fixture.damage, semantic, has_semantic);
    DrawNativeDamageScene(scene, filter, fixture.clip, direct_pixels);
  } else {
    fixture.compiled_boxes.clear();
    fixture.damage.appendComplement(fixture.clip, fixture.compiled_boxes);
    if (fixture.semantic_begin != nullptr) {
      fixture.compiled_boxes.push_back(fixture.semantic_box);
    }
    const Box* begin = fixture.compiled_boxes.empty()
                           ? nullptr
                           : fixture.compiled_boxes.data();
    const Box* end =
        fixture.compiled_boxes.empty()
            ? nullptr
            : fixture.compiled_boxes.data() + fixture.compiled_boxes.size();
    ExclusionUnion compiled(begin, end);
    compiled.reset(begin, end, fixture.masked_begin, fixture.masked_end);
    ExclusionFilter filter(device, &compiled);
    DrawNativeDamageScene(scene, filter, fixture.clip, direct_pixels);
  }
}

/// Reports one warmed 200-frame pass distribution for one adapter.
void Measure(bool native, int batch, const NativeDamageScene& scene,
             std::vector<roo::byte>& pixels,
             std::vector<roo::byte>& direct_pixels) {
  constexpr int kFrames = 200;
  Fixture fixture(scene, native);
  CountingDevice device(pixels.data());
  for (int i = 0; i < 12; ++i) {
    RunFrame(native, scene, fixture, device, direct_pixels.data());
  }
  std::array<uint64_t, kFrames> durations;
  uint64_t written_pixels = 0;
  uint64_t commands = 0;
  uint64_t addresses = 0;
  for (int i = 0; i < kFrames; ++i) {
    const uint64_t start = CpuNanos();
    RunFrame(native, scene, fixture, device, direct_pixels.data());
    durations[i] = CpuNanos() - start;
    written_pixels += device.pixels;
    commands += device.commands;
    addresses += device.addresses;
  }
  std::sort(durations.begin(), durations.end());
  std::vector<Box> gaps;
  fixture.damage.appendComplement(fixture.clip, gaps);
  std::printf(
      "P9_HOST variant=%s batch=%d scene=%s frames=%d median_ns=%llu "
      "p95_ns=%llu pixels=%llu commands=%llu addresses=%llu "
      "members=%u gaps=%zu retained_gap_capacity=%zu\n",
      native ? "native" : "hybrid", batch, scene.name, kFrames,
      static_cast<unsigned long long>(durations[kFrames / 2]),
      static_cast<unsigned long long>(durations[kFrames * 95 / 100 - 1]),
      static_cast<unsigned long long>(written_pixels / kFrames),
      static_cast<unsigned long long>(commands / kFrames),
      static_cast<unsigned long long>(addresses / kFrames),
      fixture.damage.size(), gaps.size(), fixture.compiled_boxes.capacity());
}

}  // namespace

int main() {
  std::vector<roo::byte> pixels(800 * 480 * 4);
  std::vector<roo::byte> direct_pixels(800 * 480 * 4);
  for (int batch = 1; batch <= 5; ++batch) {
    for (const NativeDamageScene& scene : NativeDamageScenes()) {
      if (batch % 2 == 1) {
        Measure(false, batch, scene, pixels, direct_pixels);
        Measure(true, batch, scene, pixels, direct_pixels);
      } else {
        Measure(true, batch, scene, pixels, direct_pixels);
        Measure(false, batch, scene, pixels, direct_pixels);
      }
    }
  }
}
