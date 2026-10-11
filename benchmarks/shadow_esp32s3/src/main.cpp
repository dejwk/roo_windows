// Hardware benchmark for Makerfabs MaTouch ESP32-S3 Parallel TFT 5", 800x480.
// Send 'b' over native USB serial to run. All timed work excludes serial I/O.
// The host target //:shadow_gallery displays the same gallery without timings.
#include <Arduino.h>

#include <algorithm>
#include <memory>

#include "roo_display.h"
#ifdef ROO_TESTING
#include "roo_display/fake/reference_device.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#else
#include "esp_memory_utils.h"
#include "esp_timer.h"
#include "roo_display/driver/esp32s3_dma_parallel_rgb565.h"
#endif
#include "roo_display/font/font_adafruit_fixed_5x7.h"
#include "roo_display/ui/text_label.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/decoration/decoration.h"
#include "roo_windows/decoration/shadow_lookup.h"
#include "roo_windows/material3/internal/elevation.h"

namespace {
using namespace roo_windows;
using roo_display::Box;
using roo_display::Color;

#ifdef ROO_TESTING
// Preserve the hardware display's RGB565 quantization and framebuffer blending.
roo_testing_transducers::FltkViewport viewport;
roo_display::ReferenceDisplayDevice panel(
    800, 480, roo_display::ReferenceDisplayDevice::ColorMode::kRgb565,
    /*blendable=*/true, viewport);
#else
// Manufacturer's display_and_touch_test: 16MHz, 8/4/8 porches, RGB565 wiring.
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
#endif
roo_display::Display display(panel);
roo_scheduler::SchedulingService scheduler;
Environment environment(scheduler);
#ifndef ROO_TESTING
volatile uint32_t checksum_sink;
Color colors[512];
int16_t xs[512];
int16_t ys[512];
#endif

const char* Variant() {
#if ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS == 2
  return "double_corners";
#elif ROO_WINDOWS_EXPERIMENTAL_SHADOW_GRADIENTS == 1
  return "double_sides";
#elif ROO_WINDOWS_SHADOW_FAST_LUT
  return "double_optimized";
#elif ROO_WINDOWS_EXPERIMENTAL_SHADOW_ANALYTIC
  return "analytic";
#elif ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT == 1
  return ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM ? "single_ram"
                                                 : "single_flash";
#else
  return ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_RAM ? "double_ram"
                                                 : "double_flash";
#endif
}

// Keep benchmark scene labels in Material levels while renderer units vary.
uint8_t RendererLevel(int level) {
  return roo_windows::material3::internal::RendererElevation(level);
}

class BenchSurface : public SurfaceWidget {
 public:
  BenchSurface(ApplicationContext& context, int width, int height,
               BorderStyle border, int level, Color color)
      : SurfaceWidget(context),
        dimensions_(width, height),
        border_(border),
        level_(RendererLevel(level)),
        color_(color) {}
  Color background() const override { return color_; }
  BorderStyle getBorderStyle() const override { return border_; }
  uint8_t getElevation() const override { return level_; }
  Color getOutlineColor() const override { return Color(0xFF657080); }
  Dimensions getSuggestedMinimumDimensions() const override {
    return dimensions_;
  }
  void paint(PaintContext& context) const override { context.clear(); }

 private:
  Dimensions dimensions_;
  BorderStyle border_;
  uint8_t level_;
  Color color_;
};

#ifndef ROO_TESTING
struct SceneCase {
  const char* name;
  int width;
  int height;
  BorderStyle style;
  int level;
};

const SceneCase kCases[] = {
    {"fab56_l0", 56, 56, BorderStyle(16, 0), 0},
    {"fab56_l1", 56, 56, BorderStyle(16, 0), 1},
    {"fab56_l3", 56, 56, BorderStyle(16, 0), 3},
    {"fab56_l5", 56, 56, BorderStyle(16, 0), 5},
    {"fab96_l3", 96, 96, BorderStyle(28, 0), 3},
    {"menu240_l2", 240, 160, BorderStyle(12, 0), 2},
    {"mixed_l3", 120, 72, BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)),
     3},
    {"circle_l3", 56, 56, BorderStyle(28, 0), 3},
    {"sharp_l3", 56, 56, BorderStyle(0, 0), 3},
    {"radius4_l3", 56, 56, BorderStyle(4, 0), 3},
    {"radius8_l3", 56, 56, BorderStyle(8, 0), 3},
    {"radius12_l3", 56, 56, BorderStyle(12, 0), 3},
    {"radius20_l3", 56, 56, BorderStyle(20, 0), 3},
    {"radius24_l3", 56, 56, BorderStyle(24, 0), 3},
};

// Emits raw batch means and dispersion; keep aggregation on the host.
void Emit(const char* metric, const char* scene, int batch, int iterations,
          uint64_t elapsed, uint32_t pixels) {
  Serial.printf(
      "{\"variant\":\"%s\",\"metric\":\"%s\",\"scene\":\"%s\","
      "\"batch\":%d,\"iterations\":%d,\"us\":%.4f,\"pixels\":%lu}\n",
      Variant(), metric, scene, batch, iterations, double(elapsed) / iterations,
      (unsigned long)pixels);
}

// Samples the entire native shadow extent, including the solid fast path.
void SampleNative(const Decoration& decoration, bool tiled) {
  const Box box = decoration.extents();
  if (!tiled) {
    int count = box.width();
    for (int i = 0; i < count; ++i) xs[i] = box.xMin() + i;
    for (int y = box.yMin(); y <= box.yMax(); ++y) {
      std::fill_n(ys, count, y);
      decoration.readColors(xs, ys, count, colors);
      checksum_sink = colors[count / 2].asArgb();
    }
  } else {
    for (int y = box.yMin(); y <= box.yMax(); y += 8) {
      for (int x = box.xMin(); x <= box.xMax(); x += 8) {
        decoration.readColorRect(x, y, std::min(x + 7, int(box.xMax())),
                                 std::min(y + 7, int(box.yMax())), colors);
        checksum_sink = colors[0].asArgb();
      }
    }
  }
}

// Equal-cost input count across variants: the same 512 exterior sample points.
void SampleFixed(const Decoration& decoration) {
  decoration.readColors(xs, ys, 512, colors);
  checksum_sink = colors[511].asArgb();
}

void RunRaster(const SceneCase& item) {
  const Box box(64, 64, 63 + item.width, 63 + item.height);
  const Decoration decoration(
      box, RendererLevel(item.level), OverlaySpec(), nullptr, Color(0xFFF8F8F8),
      item.style.corner_radii(), item.style.outline_width(), Color(0xFF657080));
  for (int i = 0; i < 512; ++i) {
    int d = 1 + (i / 4) % 12;
    int along = (i * 17) % item.width;
    switch (i % 4) {
      case 0:
        xs[i] = 64 + along;
        ys[i] = 64 - d;
        break;
      case 1:
        xs[i] = 64 + along;
        ys[i] = 63 + item.height + d;
        break;
      case 2:
        xs[i] = 64 - d;
        ys[i] = 64 + (along % item.height);
        break;
      default:
        xs[i] = 63 + item.width + d;
        ys[i] = 64 + (along % item.height);
        break;
    }
  }
  for (int i = 0; i < 10; ++i) SampleFixed(decoration);
  for (int batch = 0; batch < 7; ++batch) {
    delay(2);
    uint64_t start = esp_timer_get_time();
    for (int i = 0; i < 150; ++i) SampleFixed(decoration);
    Emit("fixed512", item.name, batch, 150, esp_timer_get_time() - start, 512);
  }
  uint32_t area = decoration.extents().width() * decoration.extents().height();
  for (bool tiled : {false, true}) {
    for (int i = 0; i < 3; ++i) SampleNative(decoration, tiled);
    for (int batch = 0; batch < 7; ++batch) {
      delay(2);
      uint64_t start = esp_timer_get_time();
      for (int i = 0; i < 12; ++i) SampleNative(decoration, tiled);
      Emit(tiled ? "tiles8" : "native", item.name, batch, 12,
           esp_timer_get_time() - start, area);
    }
  }
}

void RunRefresh(const SceneCase& item, int count) {
  Application app(&environment, display);
  auto backdrop = std::make_unique<BenchSurface>(
      app.context(), 800, 480, BorderStyle(0, 0), 0, Color(0xFFF0F0F0));
  app.add(WidgetRef(std::move(backdrop)), Box(0, 0, 799, 479));
  BenchSurface* cards[12];
  for (int i = 0; i < count; ++i) {
    auto card = std::make_unique<BenchSurface>(app.context(), item.width,
                                               item.height, item.style,
                                               item.level, Color(0xFFF8F8F8));
    cards[i] = card.get();
    int x = 55 + (i % 6) * 120;
    int y = 70 + (i / 6) * 200;
    app.add(WidgetRef(std::move(card)),
            Box(x, y, x + item.width - 1, y + item.height - 1));
  }
  app.refresh();
  for (int batch = -1; batch < 7; ++batch) {
    delay(2);
    uint64_t start = esp_timer_get_time();
    for (int frame = 0; frame < 25; ++frame) {
      for (int i = 0; i < count; ++i) cards[i]->invalidateInterior();
      app.refresh();
    }
    uint64_t elapsed = esp_timer_get_time() - start;
    if (batch >= 0)
      Emit(count == 1 ? "refresh1" : "refresh12", item.name, batch, 25, elapsed,
           count * item.width * item.height);
  }
}

#endif  // !ROO_TESTING

void ShowGallery() {
  Application app(&environment, display);
  for (int row = 0; row < 3; ++row) {
    Color background = row == 0   ? Color(0xFFFFFFFF)
                       : row == 1 ? Color(0xFFE0E0E0)
                                  : Color(0xFF202020);
    auto backdrop = std::make_unique<BenchSurface>(
        app.context(), 800, 160, BorderStyle(0, 0), 0, background);
    app.add(WidgetRef(std::move(backdrop)),
            Box(0, row * 160, 799, row * 160 + 159));
    for (int level = 1; level <= 5; ++level) {
      auto card = std::make_unique<BenchSurface>(
          app.context(), 56, 56, BorderStyle(16, 0), level,
          row == 2 ? Color(0xFF404040) : Color(0xFFF8F8F8));
      int x = 60 + (level - 1) * 150;
      int y = row * 160 + 50;
      app.add(WidgetRef(std::move(card)), Box(x, y, x + 55, y + 55));
    }
  }
  app.refresh();
}

#ifndef ROO_TESTING
// Reports the actual library's blend and RGB565 conversion on the target.
void EmitRgb565Profile() {
  Decoration decoration(Box(64, 64, 119, 119), RendererLevel(3), OverlaySpec(),
                        nullptr, Color(0xFFF8F8F8),
                        BorderStyle(16, 0).corner_radii(), SmallNumber(0),
                        Color(0));
  for (int bg : {255, 224, 32}) {
    Serial.printf(
        "{\"profile\":true,\"variant\":\"%s\",\"background\":%d,\"rgb565\":[",
        Variant(), bg);
    for (int y = 120; y < 144; ++y) {
      int16_t px = 92;
      int16_t py = y;
      Color shadow;
      decoration.readColors(&px, &py, 1, &shadow);
      Color blended =
          roo_display::AlphaBlendOverOpaque(Color(bg, bg, bg), shadow);
      Serial.printf("%s%u", y == 120 ? "" : ",",
                    roo_display::Rgb565().fromArgbColor(blended));
    }
    Serial.println("]}");
  }
}

void RunAll() {
  const uint8_t* table = roo_windows::internal::ShadowLookupTable(false);
  Serial.printf(
      "{\"meta\":true,\"variant\":\"%s\",\"cpu_mhz\":%u,\"flash_hz\":%u,"
      "\"psram_bytes\":%u,\"heap_bytes\":%u,\"decoration_bytes\":%u,"
      "\"table_address\":\"%p\",\"table_internal\":%d,\"table_drom\":%d,"
      "\"table_bytes\":%d,\"gradient_bytes\":%lu,\"measurements\":%u,\"zoom\":"
      "100,\"display\":\"800x480 RGB565 16MHz\"}\n",
      Variant(), ESP.getCpuFreqMHz(), ESP.getFlashChipSpeed(),
      ESP.getPsramSize(), ESP.getFreeHeap(), unsigned(sizeof(Decoration)),
      table, table == nullptr ? 0 : esp_ptr_internal(table),
      table == nullptr ? 0 : esp_ptr_in_drom(table),
      ROO_WINDOWS_EXPERIMENTAL_SHADOW_LUT_COUNT * 256,
      (unsigned long)roo_windows::internal::ShadowGradientBytes(),
      unsigned((4 * (sizeof(kCases) / sizeof(kCases[0])) + 1) * 7));
  for (const SceneCase& item : kCases) {
    RunRaster(item);
    RunRefresh(item, 1);
  }
  RunRefresh(kCases[2], 12);
  ShowGallery();
  EmitRgb565Profile();
  Serial.printf("BENCHMARK_DONE %s\n", Variant());
}
#endif  // !ROO_TESTING
}  // namespace

void setup() {
  Serial.begin(115200);
#ifndef ROO_TESTING
  setCpuFrequencyMhz(240);
  pinMode(10, OUTPUT);
  digitalWrite(10, LOW);  // Manufacturer backlight is active low.
#endif
  display.init();
  ShowGallery();
#ifdef ROO_TESTING
  Serial.printf("SHADOW_GALLERY_READY %s (800x480 RGB565; levels 1..5)\n",
                Variant());
#else
  Serial.printf("BENCHMARK_READY %s\n", Variant());
#endif
}

void loop() {
#ifndef ROO_TESTING
  if (Serial.available() > 0 && Serial.read() == 'b') RunAll();
#endif
  delay(10);
}
