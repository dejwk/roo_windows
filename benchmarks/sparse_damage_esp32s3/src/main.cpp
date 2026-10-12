// Sparse damage P8 benchmark for the Makerfabs 800x480 parallel ESP32-S3.
// Send 'b' over USB serial to run one 200-frame batch per scene.
#include <Arduino.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <memory>
#include <new>
#include <vector>

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
roo_scheduler::SchedulingService scheduler;
Environment environment(scheduler);

class BenchSurface final : public SurfaceWidget {
 public:
  BenchSurface(ApplicationContext& context, int height, uint8_t radius)
      : SurfaceWidget(context), height_(height), radius_(radius) {}
  Dimensions getSuggestedMinimumDimensions() const override {
    return {800, height_};
  }
  BorderStyle getBorderStyle() const override {
    return BorderStyle(radius_, 0);
  }
  Color background() const override { return Color(0xFF426281); }
  void paint(PaintContext& ctx) const override {
    ++paint_calls;
    ctx.clear();
  }
  mutable uint32_t paint_calls = 0;

 private:
  int height_;
  uint8_t radius_;
};

class BenchPanel final : public Panel {
 public:
  using Panel::add;

  BenchPanel(ApplicationContext& context, uint8_t radius)
      : Panel(context), radius_(radius) {}

  Color background() const override { return Color(0xFF426281); }

  BorderStyle getBorderStyle() const override {
    return BorderStyle(radius_, 0);
  }

 private:
  uint8_t radius_;
};

enum class Scene : uint8_t {
  kCorners,
  kSingle,
  kFull,
  kFour,
  kOverlap,
  kFifth,
  kRounded,
  kEightChildren,
  kThirtyTwoChildren,
  kNestedRounded,
  kSimpleScroll,
  kBlitScroll,
  kAcceleratedScroll
};
const char* SceneName(Scene scene) {
  switch (scene) {
    case Scene::kCorners:
      return "corners";
    case Scene::kSingle:
      return "single";
    case Scene::kFull:
      return "full";
    case Scene::kFour:
      return "four";
    case Scene::kOverlap:
      return "overlap";
    case Scene::kFifth:
      return "fifth";
    case Scene::kRounded:
      return "rounded";
    case Scene::kEightChildren:
      return "eight_children";
    case Scene::kThirtyTwoChildren:
      return "thirty_two_children";
    case Scene::kNestedRounded:
      return "nested_rounded";
    case Scene::kSimpleScroll:
      return "simple_scroll";
    case Scene::kBlitScroll:
      return "blit_scroll";
    case Scene::kAcceleratedScroll:
      return "accelerated_scroll";
  }
  return "invalid";
}

void Submit(Application& app, Scene scene, SimpleScrollablePanel* scroller,
            int frame) {
  switch (scene) {
    case Scene::kCorners:
    case Scene::kRounded:
    case Scene::kEightChildren:
    case Scene::kThirtyTwoChildren:
      app.root().invalidateDescending(Rect(10, 10, 19, 19));
      app.root().invalidateDescending(Rect(780, 460, 789, 469));
      break;
    case Scene::kNestedRounded:
      app.root().invalidateDescending(Rect(30, 30, 39, 39));
      app.root().invalidateDescending(Rect(760, 440, 769, 449));
      break;
    case Scene::kSingle:
      app.root().invalidateDescending(Rect(200, 100, 299, 199));
      break;
    case Scene::kFull:
      app.root().invalidateDescending();
      break;
    case Scene::kFour:
      for (const Rect& box :
           {Rect(10, 10, 19, 19), Rect(780, 10, 789, 19),
            Rect(10, 460, 19, 469), Rect(780, 460, 789, 469)}) {
        app.root().invalidateDescending(box);
      }
      break;
    case Scene::kOverlap:
      app.root().invalidateDescending(Rect(180, 120, 279, 219));
      app.root().invalidateDescending(Rect(230, 170, 329, 269));
      break;
    case Scene::kFifth:
      for (const Rect& box :
           {Rect(10, 10, 19, 19), Rect(200, 110, 209, 119),
            Rect(400, 210, 409, 219), Rect(600, 310, 609, 319),
            Rect(780, 460, 789, 469)}) {
        app.root().invalidateDescending(box);
      }
      break;
    case Scene::kSimpleScroll:
    case Scene::kBlitScroll:
    case Scene::kAcceleratedScroll:
      scroller->scrollTo(0, frame % 2 == 0 ? -80 : 0);
      break;
  }
}

void RunScene(Scene scene) {
  constexpr int kFrames = 200;
  Application app(&environment, display);
  BenchSurface* leaf = nullptr;
  std::vector<BenchSurface*> leaves;
  SimpleScrollablePanel* scroller = nullptr;
  if (scene == Scene::kSimpleScroll || scene == Scene::kBlitScroll ||
      scene == Scene::kAcceleratedScroll) {
    auto contents = std::make_unique<BenchSurface>(app.context(), 960, 0);
    leaf = contents.get();
    leaves.push_back(leaf);
    std::unique_ptr<SimpleScrollablePanel> viewport;
    if (scene == Scene::kBlitScroll) {
      viewport.reset(new ScrollableBlitPanel(app.context(),
                                             WidgetRef(std::move(contents))));
    } else if (scene == Scene::kAcceleratedScroll) {
      viewport.reset(new AcceleratedScrollablePanel(
          app.context(), WidgetRef(std::move(contents))));
      app.window().setAdvisoryPaintBudget(roo_time::Millis(1));
    } else {
      viewport.reset(new SimpleScrollablePanel(app.context(),
                                               WidgetRef(std::move(contents))));
    }
    scroller = viewport.get();
    app.add(std::move(viewport), Box(0, 0, 799, 399));
  } else if (scene == Scene::kEightChildren ||
             scene == Scene::kThirtyTwoChildren) {
    const int cols = scene == Scene::kEightChildren ? 4 : 8;
    const int rows = scene == Scene::kEightChildren ? 2 : 4;
    auto panel = std::make_unique<BenchPanel>(app.context(), 0);
    for (int row = 0; row < rows; ++row) {
      for (int col = 0; col < cols; ++col) {
        const int left = col * 800 / cols;
        const int top = row * 480 / rows;
        auto child =
            std::make_unique<BenchSurface>(app.context(), 480 / rows, 0);
        leaves.push_back(child.get());
        panel->add(std::move(child), Rect(left, top, (col + 1) * 800 / cols - 1,
                                          (row + 1) * 480 / rows - 1));
      }
    }
    leaf = leaves.front();
    app.add(std::move(panel), Box(0, 0, 799, 479));
  } else if (scene == Scene::kNestedRounded) {
    auto outer = std::make_unique<BenchPanel>(app.context(), 16);
    auto inner = std::make_unique<BenchPanel>(app.context(), 12);
    auto child = std::make_unique<BenchSurface>(app.context(), 420, 8);
    leaf = child.get();
    leaves.push_back(leaf);
    inner->add(std::move(child), Rect(10, 10, 769, 449));
    outer->add(std::move(inner), Rect(10, 10, 789, 469));
    app.add(std::move(outer), Box(0, 0, 799, 479));
  } else {
    auto surface = std::make_unique<BenchSurface>(
        app.context(), 480, scene == Scene::kRounded ? 16 : 0);
    leaf = surface.get();
    leaves.push_back(leaf);
    app.add(std::move(surface), Box(0, 0, 799, 479));
  }
  app.refresh();
  for (int i = 0; i < 12; ++i) {
    Submit(app, scene, scroller, i);
    app.refresh();
  }
  for (BenchSurface* surface : leaves) surface->paint_calls = 0;
  std::array<uint32_t, kFrames> durations{};
  std::array<uint32_t, kFrames> submit_durations{};
  std::array<uint32_t, kFrames> refresh_durations{};
  uint64_t pixels = 0;
  uint64_t commands = 0;
  uint64_t addresses = 0;
  uint64_t copies = 0;
  g_allocation_count = 0;
  g_allocated_bytes = 0;
  g_track_allocations = true;
  for (int frame = 0; frame < kFrames; ++frame) {
    instrumented_device.resetCounters();
    const uint64_t start = esp_timer_get_time();
    Submit(app, scene, scroller, frame);
    const uint64_t after_submit = esp_timer_get_time();
    app.refresh();
    const uint64_t after_refresh = esp_timer_get_time();
    submit_durations[frame] = after_submit - start;
    refresh_durations[frame] = after_refresh - after_submit;
    durations[frame] = after_refresh - start;
    const FrameCounters& counters = instrumented_device.counters();
    pixels += counters.pixels;
    commands += counters.commands;
    addresses += counters.address_calls;
    copies += counters.blit_calls;
  }
  g_track_allocations = false;
  std::sort(durations.begin(), durations.end());
  std::sort(submit_durations.begin(), submit_durations.end());
  std::sort(refresh_durations.begin(), refresh_durations.end());
  uint32_t hooks = 0;
  for (BenchSurface* surface : leaves) hooks += surface->paint_calls;
  Serial.printf(
      "P8_SCENE name=%s frames=%d median_us=%u p95_us=%u "
      "submit_us=%u refresh_us=%u "
      "pixels=%llu commands=%llu addresses=%llu copies=%llu "
      "hooks=%u allocations=%u allocation_bytes=%u "
      "free_heap=%u min_free_heap=%u stack_headroom=%u\n",
      SceneName(scene), kFrames, durations[kFrames / 2],
      durations[kFrames * 95 / 100 - 1], submit_durations[kFrames / 2],
      refresh_durations[kFrames / 2], pixels / kFrames, commands / kFrames,
      addresses / kFrames, copies / kFrames, hooks / kFrames,
      g_allocation_count, static_cast<unsigned>(g_allocated_bytes),
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
  Serial.printf(
      "P8_READY cpu_mhz=%u flash_hz=%u psram=%u widget=%u "
      "container=%u window=%u clipper=%u damage=%u\n",
      ESP.getCpuFreqMHz(), ESP.getFlashChipSpeed(), ESP.getPsramSize(),
      sizeof(roo_windows::Widget), sizeof(roo_windows::Container),
      sizeof(roo_windows::MainWindow),
      sizeof(roo_windows::internal::ClipperOutput),
      sizeof(roo_windows::internal::DamageRegion));
}
void loop() {
  if (Serial.available() > 0 && Serial.read() == 'b') {
    for (Scene scene :
         {Scene::kSingle, Scene::kCorners, Scene::kFull, Scene::kFour,
          Scene::kOverlap, Scene::kFifth, Scene::kRounded,
          Scene::kEightChildren, Scene::kThirtyTwoChildren,
          Scene::kNestedRounded, Scene::kSimpleScroll, Scene::kBlitScroll,
          Scene::kAcceleratedScroll}) {
      RunScene(scene);
    }
    Serial.println("P8_DONE");
  }
  delay(10);
}
