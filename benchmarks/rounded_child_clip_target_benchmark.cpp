// ESP32-S3 runtime benchmark for rounded child clipping P8.
//
// Copy this source into the PlatformIO target fixture described in
// docs/design/in_progress/rounded_child_clipping_design.md. It is not part of
// the library.
#include <Arduino.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <new>

#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "roo_display.h"
#include "roo_display/products/makerfabs/esp32s3_parallel_ips_capacitive.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/menu/menu.h"

#ifndef ROO_WINDOWS_MIN_BLIT_COPY_PIXELS
#define ROO_WINDOWS_MIN_BLIT_COPY_PIXELS 0
#endif

#if defined(__cpp_exceptions)
#error "P8 target benchmark must be built with -fno-exceptions"
#endif

#if defined(__GXX_RTTI)
#error "P8 target benchmark must be built with -fno-rtti"
#endif

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

#ifndef P8_SCENE_WIDTH
#define P8_SCENE_WIDTH 192
#endif
#ifndef P8_SCENE_HEIGHT
#define P8_SCENE_HEIGHT 128
#endif
#ifndef P8_SCENE_RADIUS
#define P8_SCENE_RADIUS 16
#endif

constexpr int kSceneWidth = P8_SCENE_WIDTH;
constexpr int kSceneHeight = P8_SCENE_HEIGHT;
constexpr int kSceneRadius = P8_SCENE_RADIUS;

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

std::unique_ptr<roo_windows::material3::MenuGroup> MakeMenuContents(
    ApplicationContext& context) {
  using roo_windows::material3::MenuGroup;
  using roo_windows::material3::MenuRow;
  using roo_windows::material3::StandardMenuItem;
  using roo_windows::material3::StandardMenuItemFlags;
  using roo_windows::material3::StandardMenuItemInit;
  static constexpr const char* kLabels[] = {
      "Security",       "Open",          "WEP",
      "WPA2 Personal",  "WPA3 Personal", "WPA2 Enterprise",
      "Hidden network", "Advanced"};
  static constexpr const char* kSupporting[] = {
      "Recommended", "No password",      "Legacy",      "AES encryption",
      "Strongest",   "Managed identity", "Manual SSID", "More options"};
  auto group = std::make_unique<MenuGroup>(context);
  for (size_t i = 0; i < std::size(kLabels); ++i) {
    StandardMenuItemInit init;
    init.headline = kLabels[i];
    init.supporting = kSupporting[i];
    init.flags = StandardMenuItemFlags::kSelectable;
    if (i == 3) {
      init.flags = init.flags | StandardMenuItemFlags::kSelected;
    }
    group->add(std::make_unique<MenuRow<StandardMenuItem>>(context, init));
  }
  return group;
}

class Badge final : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(36, 20);
  }

  Color background() const override { return Color(0xFFFF7A45); }
  BorderStyle getBorderStyle() const override { return BorderStyle(8, 0); }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
};

// A fixed opaque header creates sibling coverage independent of the blit.
// The text rows underneath still have expensive rasterization to avoid.
class HeaderCover final : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, 0);
  }
  BorderStyle getBorderStyle() const override { return BorderStyle(0, 0); }

  Color background() const override { return Color(0xFF24405C); }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
};

class AcceptancePanel final : public Panel {
 public:
  AcceptancePanel(ApplicationContext& context, int width, int height,
                  int radius, bool cached, bool badge, bool header = false)
      : Panel(context), radius_(radius) {
    auto contents = MakeMenuContents(context);
    if (cached) {
      auto scroller = std::make_unique<ScrollableBlitPanel>(
          context, WidgetRef(std::move(contents)));
      scroller_ = scroller.get();
      add(std::move(scroller), Rect(0, 0, width - 1, height - 1));
    } else {
      auto scroller = std::make_unique<SimpleScrollablePanel>(
          context, WidgetRef(std::move(contents)));
      scroller_ = scroller.get();
      add(std::move(scroller), Rect(0, 0, width - 1, height - 1));
    }
    if (badge) {
      add(std::make_unique<Badge>(context),
          Rect(width - 48, 8, width - 13, 27));
    }
    if (header) {
      add(std::make_unique<HeaderCover>(context),
          Rect(0, 0, width - 1, height / 2 - 1));
    }
  }

  bool clipsChildrenToRoundedBounds() const override { return true; }
  BorderStyle getBorderStyle() const override {
    return BorderStyle(radius_, 0);
  }
  Color background() const override { return Color(0xFFF4EAD0); }

  SimpleScrollablePanel& scroller() { return *scroller_; }

 private:
  int radius_;
  SimpleScrollablePanel* scroller_ = nullptr;
};

roo_display::products::makerfabs::Esp32s3ParallelIpsCapacitive1024x600
    physical_device;
InstrumentedDevice instrumented_device(physical_device.display());
roo_display::Display display(instrumented_device);
roo_scheduler::SchedulingService scheduler;
Environment environment(scheduler);
Application app(&environment, display);

struct Sample {
  uint32_t duration_us;
  uint32_t commands;
  uint64_t pixels;
  uint64_t copied_pixels;
};

template <size_t N>
void Report(const char* name, AcceptancePanel& panel) {
  std::array<Sample, N> samples{};
  uint32_t max_duration = 0;
  uint64_t min_copied = UINT64_MAX;
  uint64_t max_copied = 0;
  uint64_t sum_pixels = 0;
  uint64_t sum_copied = 0;
  uint64_t sum_commands = 0;

  panel.scroller().scrollTo(0, -32);
  app.refresh();
  for (int i = 0; i < 12; ++i) {
    panel.scroller().scrollTo(0, (i & 1) == 0 ? -40 : -32);
    app.refresh();
  }

  g_allocation_count = 0;
  g_allocated_bytes = 0;
  g_track_allocations = true;
  for (size_t i = 0; i < N; ++i) {
    instrumented_device.resetCounters();
    panel.scroller().scrollTo(0, (i & 1) == 0 ? -40 : -32);
    const uint32_t start = micros();
    app.refresh();
    const uint32_t duration = micros() - start;
    const FrameCounters& counters = instrumented_device.counters();
    samples[i] = {duration, counters.commands, counters.pixels,
                  counters.copied_pixels};
    max_duration = std::max(max_duration, duration);
    min_copied = std::min(min_copied, counters.copied_pixels);
    max_copied = std::max(max_copied, counters.copied_pixels);
    sum_pixels += counters.pixels;
    sum_copied += counters.copied_pixels;
    sum_commands += counters.commands;
  }
  g_track_allocations = false;

  std::sort(samples.begin(), samples.end(),
            [](const Sample& a, const Sample& b) {
              return a.duration_us < b.duration_us;
            });
  const Sample median = samples[N / 2];
  const Sample p95 = samples[(N * 95 + 99) / 100 - 1];
  Serial.printf(
      "P8_RESULT name=%s samples=%u median_us=%u p95_us=%u max_us=%u "
      "mean_pixels=%llu mean_drawn=%llu mean_commands=%llu min_copied=%llu "
      "max_copied=%llu "
      "new_calls=%u new_bytes=%u free_heap=%u min_free_heap=%u "
      "largest_free=%u stack_headroom=%u\n",
      name, N, median.duration_us, p95.duration_us, max_duration,
      sum_pixels / N, (sum_pixels - sum_copied) / N, sum_commands / N,
      min_copied, max_copied, g_allocation_count,
      static_cast<unsigned>(g_allocated_bytes),
      heap_caps_get_free_size(MALLOC_CAP_8BIT),
      heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
      heap_caps_get_largest_free_block(MALLOC_CAP_8BIT),
      uxTaskGetStackHighWaterMark(nullptr) * sizeof(StackType_t));
}

AcceptancePanel* ordinary = nullptr;
AcceptancePanel* ordinary_badge = nullptr;
AcceptancePanel* cached = nullptr;
AcceptancePanel* cached_badge = nullptr;

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.printf(
      "P8_BEGIN board=ESP32-S3 display=Makerfabs_1024x600 scene=%dx%d "
      "radius=%d min_blit_pixels=%d widget_size=%u container_size=%u "
      "clipper_output_size=%u\n",
      kSceneWidth, kSceneHeight, kSceneRadius, ROO_WINDOWS_MIN_BLIT_COPY_PIXELS,
      sizeof(roo_windows::Widget), sizeof(roo_windows::Container),
      sizeof(roo_windows::internal::ClipperOutput));
  physical_device.initTransport();
  display.setBackgroundColor(Color(0xFF101418));
  display.init();

  auto ordinary_owner = std::make_unique<AcceptancePanel>(
      app.context(), kSceneWidth, kSceneHeight, kSceneRadius, false, false);
  ordinary = ordinary_owner.get();
  app.add(std::move(ordinary_owner),
          Box(48, 48, 48 + kSceneWidth - 1, 48 + kSceneHeight - 1));

  auto ordinary_badge_owner = std::make_unique<AcceptancePanel>(
      app.context(), kSceneWidth, kSceneHeight, kSceneRadius, false, true);
  ordinary_badge = ordinary_badge_owner.get();
  app.add(std::move(ordinary_badge_owner),
          Box(768, 48, 768 + kSceneWidth - 1, 48 + kSceneHeight - 1));

  auto cached_owner = std::make_unique<AcceptancePanel>(
      app.context(), kSceneWidth, kSceneHeight, kSceneRadius, true, false);
  cached = cached_owner.get();
  app.add(std::move(cached_owner),
          Box(288, 48, 288 + kSceneWidth - 1, 48 + kSceneHeight - 1));

  auto badge_owner = std::make_unique<AcceptancePanel>(
      app.context(), kSceneWidth, kSceneHeight, kSceneRadius, true, true);
  cached_badge = badge_owner.get();
  app.add(std::move(badge_owner),
          Box(528, 48, 528 + kSceneWidth - 1, 48 + kSceneHeight - 1));

  app.refresh();
  Report<101>("ordinary", *ordinary);
  Report<101>("ordinary_badge", *ordinary_badge);
  Report<101>("blit", *cached);
  Report<101>("blit_badge", *cached_badge);
  // Add these only after the original four reports so their exclusions cannot
  // change the original acceptance workload. The header covers the top half of
  // the content, exercising early trimming from an ordinary foreground sibling.
  auto header_owner = std::make_unique<AcceptancePanel>(
      app.context(), kSceneWidth, kSceneHeight, kSceneRadius, false, false,
      true);
  AcceptancePanel* header = header_owner.get();
  app.add(std::move(header_owner),
          Box(48, 240, 48 + kSceneWidth - 1, 240 + kSceneHeight - 1));
  auto cached_header_owner = std::make_unique<AcceptancePanel>(
      app.context(), kSceneWidth, kSceneHeight, kSceneRadius, true, false,
      true);
  AcceptancePanel* cached_header = cached_header_owner.get();
  app.add(std::move(cached_header_owner),
          Box(288, 240, 288 + kSceneWidth - 1, 240 + kSceneHeight - 1));
  app.refresh();
  Report<101>("ordinary_header", *header);
  Report<101>("blit_header", *cached_header);
  Serial.println("P8_DONE");
}

void loop() { delay(1000); }
