// Animate the authoritative fitted shadow parameters.
// Large rectangles sample the same 0..31 index (0..15.5 dp); small ones
// stay at 0.5 and 1 dp for low-elevation comparison. This rendering
// experiment uses internal shadow specs rather than changing the widget API.
#include <Arduino.h>

#include <cstdio>

#include "roo_display.h"
#ifdef ROO_TESTING
#include "roo_display/fake/reference_device.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#else
#include "roo_display/driver/esp32s3_dma_parallel_rgb565.h"
#endif
#include "curves.h"
#include "roo_display/font/font_adafruit_fixed_5x7.h"
#include "roo_display/shape/basic.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/decoration/decoration.h"

// The preview tile is laid out for native 100% zoom.
static_assert(ROO_WINDOWS_ZOOM == 100,
              "Run this 800x480 comparison at zoom 100");

namespace {
using namespace roo_display;
using namespace shadow_curves;
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

FontAdafruitFixed5x7 font;
uint32_t started_ms;
uint32_t last_frame_ms;
int previous_level = -1;
bool previous_triangle = false;
constexpr Color kBackground(0xFFF8F8F8);
// Temporary inspection switch; false restores the combined key + ambient view.
constexpr bool kAmbientOnly = false;

// Each tile resolves surface coverage, shadow, and background in one pass.
// Opaque output ensures shrinking shadows erase themselves without trails.
class ShadowTile : public Rasterizable {
 public:
  ShadowTile(int level, bool small = false)
      : bounds_(small ? Box(16, 12, 79, 51) : Box(48, 40, 191, 159)),
        extents_(small ? Box(0, 0, 95, 79) : Box(0, 0, 239, 239)),
        key_(MakeSpec(bounds_, level, false)),
        ambient_(MakeSpec(bounds_, level, true)) {}

  Box extents() const override { return extents_; }

  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  Color* result) const override {
    for (uint32_t i = 0; i < count; ++i) {
      uint8_t coverage = roo_windows::internal::RoundedFillCoverage(
          bounds_, key_.radii, 0, x[i], y[i]);
      int alpha = 0;
      if (coverage != 255) {
        alpha = kAmbientOnly ? roo_windows::internal::ReadLookupShadowLayer(
                                   ambient_, true, x[i], y[i])
                             : roo_windows::internal::ReadLookupShadow(
                                   key_, ambient_, x[i], y[i]);
      }
      int background = (248 * (255 - alpha) + 127) / 255;
      int value = (255 * coverage + background * (255 - coverage) + 127) / 255;
      result[i] = Color(value, value, value);
    }
  }

 private:
  Box bounds_;
  Box extents_;
  roo_windows::internal::LookupShadowSpec key_;
  roo_windows::internal::LookupShadowSpec ambient_;
};

// Only the status and large shadow tiles change; small references stay still.
void DrawFrame(int level, bool triangle) {
  DrawingContext dc(display);
  dc.setBackgroundColor(kBackground);
  dc.draw(ShadowTile(level), 280, 85);
  char status[96];
  snprintf(status, sizeof(status),
           "%s  |  index %2d / 31  |  elevation %d.%d dp",
           triangle ? "Triangle (10 s)" : "Sine (10 s)", level, level / 2,
           (level % 2) * 5);
  dc.draw(MakeTileOf(TextLabel(status, font, color::Black),
                     Box(20, 433, 779, 451), kCenter | kMiddle));
}
}  // namespace

void setup() {
#ifndef ROO_TESTING
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
#endif
  display.init();
  {
    DrawingContext dc(display);
    dc.setBackgroundColor(kBackground);
    dc.draw(FilledRect(0, 0, 799, 479, kBackground));
    dc.draw(
        TextLabel(kAmbientOnly
                      ? "Shadow curves - anchors +2 dp - RGB565 - ambient only"
                      : "Shadow curves - anchors +2 dp - RGB565",
                  font, color::Black),
        28, 40);
    dc.draw(MakeTileOf(
        TextLabel("Fitted shadows - free key offset", font, color::Black),
        Box(280, 55, 519, 80), kCenter | kMiddle));
    for (int level = 1; level <= 2; ++level) {
      int x = 292 + 120 * (level - 1);
      dc.draw(ShadowTile(level, true), x, 330);
      dc.draw(MakeTileOf(
          TextLabel(level == 1 ? "0.5 dp" : "1 dp", font, color::Black),
          Box(x, 410, x + 95, 430), kCenter | kMiddle));
    }
    dc.draw(TextLabel("Authoritative fitted parameters - key + ambient", font,
                      color::Black),
            28, 465);
  }
  started_ms = millis();
  last_frame_ms = started_ms;
  DrawFrame(0, false);
}

void loop() {
  // A direct rendering experiment has no widgets or widget animation tracks.
  // Sample time before drawing; repaint at most every 20 ms, skipping unchanged
  // rounded levels. Unsigned subtraction remains safe across millis() wrap.
  uint32_t now = millis();
  if (now - last_frame_ms < 20) {
    delay(1);
    return;
  }
  last_frame_ms = now;
  uint32_t elapsed = now - started_ms;
  int level = SampleLevel(elapsed);
  bool triangle = elapsed % 20000 >= 10000;
  if (level != previous_level || triangle != previous_triangle) {
    DrawFrame(level, triangle);
    previous_level = level;
    previous_triangle = triangle;
  }
}
