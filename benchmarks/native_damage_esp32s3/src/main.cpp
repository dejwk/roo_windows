// Native-versus-compiled damage filter probe for the Makerfabs ESP32-S3.
// Send 'b' over USB serial to run one 200-frame batch per scene.
#include <Arduino.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <memory>
#include <new>
#include <vector>

#include "../../native_damage_filter.h"
#include "../../native_damage_scenes.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "roo_display.h"
#include "roo_display/driver/esp32s3_dma_parallel_rgb565.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/accelerated_scrollable_panel.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/damage_region.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/rounded_clip.h"

SET_LOOP_TASK_STACK_SIZE(32768);

namespace {
bool g_track_allocations = false;
uint32_t g_allocation_count = 0;
size_t g_allocated_bytes = 0;
void* Allocate(size_t size) {
  if (g_track_allocations) {
    ++g_allocation_count;
    g_allocated_bytes += size;
  }
  return std::malloc(size == 0 ? 1 : size);
}
}  // namespace
void* operator new(size_t size) {
  void* ptr = Allocate(size);
  if (ptr == nullptr) std::abort();
  return ptr;
}
void* operator new[](size_t size) { return ::operator new(size); }
void* operator new(size_t, const std::nothrow_t&) noexcept;
void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}
void* operator new[](size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}
void operator delete(void* ptr) noexcept { std::free(ptr); }
void operator delete[](void* ptr) noexcept { std::free(ptr); }
void operator delete(void* ptr, size_t) noexcept { std::free(ptr); }
void operator delete[](void* ptr, size_t) noexcept { std::free(ptr); }

namespace {
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
using roo_display::DisplayDevice;
using roo_windows::AcceleratedScrollablePanel;
using roo_windows::Application;
using roo_windows::ApplicationContext;
using roo_windows::BorderStyle;
using roo_windows::Dimensions;
using roo_windows::Environment;
using roo_windows::PaintContext;
using roo_windows::Panel;
using roo_windows::Rect;
using roo_windows::ScrollableBlitPanel;
using roo_windows::SimpleScrollablePanel;
using roo_windows::SurfaceWidget;
using roo_windows::WidgetRef;

struct FrameCounters {
  uint64_t pixels = 0;
  uint64_t copied_pixels = 0;
  uint32_t commands = 0;
  uint32_t address_calls = 0;
  uint32_t blit_calls = 0;
  Box last_source = Box(0, 0, -1, -1);
  Box last_destination = Box(0, 0, -1, -1);
};

class InstrumentedDevice final : public DisplayDevice {
 public:
  explicit InstrumentedDevice(DisplayDevice& output)
      : DisplayDevice(output.raw_width(), output.raw_height()),
        output_(output) {}

  void resetCounters() { counters_ = FrameCounters(); }

  const FrameCounters& counters() const { return counters_; }

  void init() override { output_.init(); }
  void begin() override { output_.begin(); }
  void end() override { output_.end(); }
  void flush() override { output_.flush(); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    ++counters_.address_calls;
    output_.setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    ++counters_.commands;
    counters_.pixels += count;
    output_.write(colors, count);
  }

  void fill(Color color, uint32_t count) override {
    ++counters_.commands;
    counters_.pixels += count;
    output_.fill(color, count);
  }

  void writePixels(BlendingMode mode, Color* colors, int16_t* x, int16_t* y,
                   uint16_t count) override {
    ++counters_.commands;
    counters_.pixels += count;
    output_.writePixels(mode, colors, x, y, count);
  }

  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t count) override {
    ++counters_.commands;
    counters_.pixels += count;
    output_.fillPixels(mode, color, x, y, count);
  }

  void writeRects(BlendingMode mode, Color* colors, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    ++counters_.commands;
    counters_.pixels += RectPixels(x0, y0, x1, y1, count);
    output_.writeRects(mode, colors, x0, y0, x1, y1, count);
  }

  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    ++counters_.commands;
    counters_.pixels += RectPixels(x0, y0, x1, y1, count);
    output_.fillRects(mode, color, x0, y0, x1, y1, count);
  }

  void drawDirectRect(const roo::byte* data, size_t row_width_bytes,
                      int16_t src_x0, int16_t src_y0, int16_t src_x1,
                      int16_t src_y1, int16_t dst_x0, int16_t dst_y0) override {
    ++counters_.commands;
    counters_.pixels += Area(src_x0, src_y0, src_x1, src_y1);
    output_.drawDirectRect(data, row_width_bytes, src_x0, src_y0, src_x1,
                           src_y1, dst_x0, dst_y0);
  }

  void drawDirectRectAsync(const roo::byte* data, size_t row_width_bytes,
                           int16_t src_x0, int16_t src_y0, int16_t src_x1,
                           int16_t src_y1, int16_t dst_x0,
                           int16_t dst_y0) override {
    ++counters_.commands;
    counters_.pixels += Area(src_x0, src_y0, src_x1, src_y1);
    output_.drawDirectRectAsync(data, row_width_bytes, src_x0, src_y0, src_x1,
                                src_y1, dst_x0, dst_y0);
  }

  const ColorFormat& getColorFormat() const override {
    return output_.getColorFormat();
  }

  const Capabilities& getCapabilities() const override {
    return output_.getCapabilities();
  }

  void blitCopy(int16_t src_x0, int16_t src_y0, int16_t src_x1, int16_t src_y1,
                int16_t dst_x0, int16_t dst_y0) override {
    ++counters_.commands;
    ++counters_.blit_calls;
    const uint64_t pixels = Area(src_x0, src_y0, src_x1, src_y1);
    counters_.pixels += pixels;
    counters_.copied_pixels += pixels;
    counters_.last_source = Box(src_x0, src_y0, src_x1, src_y1);
    counters_.last_destination =
        Box(dst_x0, dst_y0, dst_x0 + src_x1 - src_x0, dst_y0 + src_y1 - src_y0);
    output_.blitCopy(src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0);
  }

  void setBgColorHint(Color color) override { output_.setBgColorHint(color); }

 protected:
  void orientationUpdated() override { output_.setOrientation(orientation()); }

 private:
  static uint64_t Area(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    if (x1 < x0 || y1 < y0) return 0;
    return static_cast<uint64_t>(x1 - x0 + 1) * (y1 - y0 + 1);
  }

  static uint64_t RectPixels(const int16_t* x0, const int16_t* y0,
                             const int16_t* x1, const int16_t* y1,
                             uint16_t count) {
    uint64_t result = 0;
    for (uint16_t i = 0; i < count; ++i) {
      result += Area(x0[i], y0[i], x1[i], y1[i]);
    }
    return result;
  }

  DisplayDevice& output_;
  FrameCounters counters_;
};

constexpr roo_display::esp32s3_dma::Config kPanelConfig = {
    .width = 800,
    .height = 480,
    .de = 40,
    .hsync = 39,
    .vsync = 41,
    .pclk = 42,
    .hsync_pulse_width = 4,
    .hsync_back_porch = 8,
    .hsync_front_porch = 8,
    .hsync_polarity = 0,
    .vsync_pulse_width = 4,
    .vsync_back_porch = 8,
    .vsync_front_porch = 8,
    .vsync_polarity = 0,
    .pclk_active_neg = 1,
    .prefer_speed = 16000000,
    .r0 = 45,
    .r1 = 48,
    .r2 = 47,
    .r3 = 21,
    .r4 = 14,
    .g0 = 5,
    .g1 = 6,
    .g2 = 7,
    .g3 = 15,
    .g4 = 16,
    .g5 = 4,
    .b0 = 8,
    .b1 = 3,
    .b2 = 46,
    .b3 = 9,
    .b4 = 1,
    .bswap = false};
roo_display::esp32s3_dma::ParallelRgb565<
    roo_display::esp32s3_dma::FLUSH_MODE_LAZY>
    panel(kPanelConfig);

InstrumentedDevice instrumented_device(panel);
roo_display::Display display(instrumented_device);

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

roo::byte* direct_pixels = nullptr;

struct Fixture {
  explicit Fixture(const NativeDamageScene& scene)
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
#if !ROO_BENCH_NATIVE
    compiled_boxes.reserve(45);
#endif
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

void RunFrame(const NativeDamageScene& scene, Fixture& fixture) {
  instrumented_device.resetCounters();
  instrumented_device.begin();
#if ROO_BENCH_NATIVE
  ExclusionUnion semantic(fixture.semantic_begin, fixture.semantic_end);
  semantic.reset(fixture.semantic_begin, fixture.semantic_end,
                 fixture.masked_begin, fixture.masked_end);
  const bool has_semantic = fixture.semantic_begin != fixture.semantic_end ||
                            fixture.masked_begin != fixture.masked_end;
  NativeDamageFilter filter(instrumented_device, fixture.damage, semantic,
                            has_semantic);
  DrawNativeDamageScene(scene, filter, fixture.clip, direct_pixels);
#else
  fixture.compiled_boxes.clear();
  fixture.damage.appendComplement(fixture.clip, fixture.compiled_boxes);
  if (fixture.semantic_begin != nullptr) {
    fixture.compiled_boxes.push_back(fixture.semantic_box);
  }
  const Box* begin =
      fixture.compiled_boxes.empty() ? nullptr : fixture.compiled_boxes.data();
  const Box* end =
      fixture.compiled_boxes.empty()
          ? nullptr
          : fixture.compiled_boxes.data() + fixture.compiled_boxes.size();
  ExclusionUnion compiled(begin, end);
  compiled.reset(begin, end, fixture.masked_begin, fixture.masked_end);
  ExclusionFilter filter(instrumented_device, &compiled);
  DrawNativeDamageScene(scene, filter, fixture.clip, direct_pixels);
#endif
  instrumented_device.end();
}

void RunScene(const NativeDamageScene& scene) {
  constexpr int kFrames = 200;
  Fixture fixture(scene);
  for (int frame = 0; frame < 12; ++frame) RunFrame(scene, fixture);
  std::array<uint32_t, kFrames> durations{};
  uint64_t pixels = 0;
  uint64_t commands = 0;
  uint64_t addresses = 0;
  uint64_t copies = 0;
  g_allocation_count = 0;
  g_allocated_bytes = 0;
  g_track_allocations = true;
  for (int frame = 0; frame < kFrames; ++frame) {
    const uint64_t start = esp_timer_get_time();
    RunFrame(scene, fixture);
    durations[frame] = esp_timer_get_time() - start;
    const FrameCounters& counters = instrumented_device.counters();
    pixels += counters.pixels;
    commands += counters.commands;
    addresses += counters.address_calls;
    copies += counters.blit_calls;
  }
  g_track_allocations = false;
  std::sort(durations.begin(), durations.end());
  std::vector<Box> gaps;
  fixture.damage.appendComplement(fixture.clip, gaps);
  Serial.printf(
      "P9_SCENE name=%s frames=%d median_us=%u p95_us=%u "
      "pixels=%llu commands=%llu addresses=%llu copies=%llu "
      "members=%u gaps=%u allocations=%u allocation_bytes=%u "
      "free_heap=%u min_free_heap=%u stack_headroom=%u\n",
      scene.name, kFrames, durations[kFrames / 2],
      durations[kFrames * 95 / 100 - 1], pixels / kFrames, commands / kFrames,
      addresses / kFrames, copies / kFrames, fixture.damage.size(),
      static_cast<unsigned>(gaps.size()), g_allocation_count,
      static_cast<unsigned>(g_allocated_bytes),
      heap_caps_get_free_size(MALLOC_CAP_8BIT),
      heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
      uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t));
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1200);
  setCpuFrequencyMhz(240);
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);
  display.init();
  direct_pixels = static_cast<roo::byte*>(
      heap_caps_calloc(800 * 480 * 2, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
  if (direct_pixels == nullptr) std::abort();
  Serial.printf(
      "P9_READY variant=%s cpu_mhz=%u flash_hz=%u psram=%u "
      "stack_size=%u filter=%u\n",
#if ROO_BENCH_NATIVE
      "native",
#else
      "hybrid",
#endif
      ESP.getCpuFreqMHz(), ESP.getFlashChipSpeed(), ESP.getPsramSize(), 32768u,
      sizeof(NativeDamageFilter));
}

void loop() {
  if (Serial.available() > 0 && Serial.read() == 'b') {
    for (const NativeDamageScene& scene : NativeDamageScenes()) RunScene(scene);
    Serial.println("P9_DONE");
  }
  delay(10);
}
