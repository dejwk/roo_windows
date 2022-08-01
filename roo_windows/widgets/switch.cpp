#include "switch.h"

#include <Arduino.h>

#include "roo_display/image/image.h"
#include "roo_display/internal/raw_streamable_overlay.h"
#include "roo_display/ui/tile.h"

using namespace roo_display;

namespace roo_windows {

namespace {
const RleImage4bppxBiased<Alpha4, PrgMemResource>& slider_20();
const RleImage4bppxBiased<Alpha4, PrgMemResource>& circle_20();
const RleImage4bppxBiased<Alpha4, PrgMemResource>& shadow_20();

static constexpr int kSwitchAnimationMs = 120;
}  // namespace

bool Switch::onSingleTapUp(int16_t x, int16_t y) {
  toggle();
  anim_ = millis() & 0x7FFF;
  return Widget::onSingleTapUp(x, y);
}

int16_t Switch::time_animating_ms() const {
  return (millis() & 0x7FFF) - (anim_ & 0x7FFF);
}

bool Switch::paint(const Surface& s) {
  const Theme& th = theme();
  Color circleColor =
      isOn() ? th.color.highlighterColor(s.bgcolor()) : th.color.surface;
  Color sliderColor = isOn() ? th.color.highlighterColor(s.bgcolor())
                             : th.color.defaultColor(s.bgcolor());
  RleImage4bppxBiased<Alpha4, PrgMemResource> slider = slider_20();
  RleImage4bppxBiased<Alpha4, PrgMemResource> circle = circle_20();
  RleImage4bppxBiased<Alpha4, PrgMemResource> shadow = shadow_20();
  slider.color_mode().setColor(sliderColor);
  circle.color_mode().setColor(circleColor);
  // leaving shadow as black.
  int16_t xoffset;
  bool finished = true;
  if (isAnimating()) {
    int16_t ms = time_animating_ms();
    if (ms < 0 || ms > kSwitchAnimationMs) {
      anim_ = 0x8000;
      xoffset = isOn() ? 20 : 0;
    } else {
      markDirty();
      int16_t progress = ms * 20 / kSwitchAnimationMs;
      xoffset = isOn() ? progress : 20 - progress;
      finished = false;
    }
  } else {
    xoffset = isOn() ? 20 : 0;
  }
  auto composite = MakeDrawableRawStreamable(Overlay(
      slider, 5, 4, Overlay(shadow, xoffset, 0, circle, xoffset + 1, 1), 0, 0));
  roo_display::Tile toggle(&composite, Box(0, 0, 42, 24), roo_display::kMiddle,
                           roo_display::color::Transparent, s.fill_mode());
  if (isInvalidated()) {
    roo_display::Tile tile(&toggle, bounds(),
                           roo_display::kCenter | roo_display::kMiddle,
                           roo_display::color::Transparent, s.fill_mode());
    s.drawObject(tile);
  } else {
    s.drawObject(toggle,
                 roo_display::kCenter.GetOffset(bounds(), toggle.extents()),
                 roo_display::kMiddle.GetOffset(bounds(), toggle.extents()));
  }
  return finished;
}

Dimensions Switch::getSuggestedMinimumDimensions() const {
  return Dimensions(42, 24);
}

namespace {

// Image file slider_20 34x14, 4-bit Alpha,  RLE, 62 bytes.
static const uint8_t slider_20_data[] PROGMEM = {
    0x40, 0x20, 0x48, 0x0A, 0x25, 0x04, 0x02, 0x60, 0x18, 0x0B, 0x05,
    0x01, 0x30, 0x18, 0x0B, 0x25, 0x01, 0x28, 0x0B, 0x45, 0x10, 0x28,
    0x0B, 0x45, 0x02, 0x04, 0x80, 0xB4, 0x50, 0x48, 0x09, 0x80, 0x50,
    0x48, 0x0B, 0x45, 0x04, 0x02, 0x80, 0xB4, 0x50, 0x21, 0x80, 0xB4,
    0x52, 0x01, 0x80, 0xB2, 0x50, 0x13, 0x01, 0x80, 0xB0, 0x50, 0x16,
    0x02, 0x04, 0x80, 0xA2, 0x50, 0x40, 0x24,
};
const RleImage4bppxBiased<Alpha4, PrgMemResource>& slider_20() {
  static RleImage4bppxBiased<Alpha4, PrgMemResource> value(
      34, 14, slider_20_data, Alpha4(color::Black));
  return value;
}

// Image file circle_20 20x20, 4-bit Alpha,  RLE, 86 bytes.
static const uint8_t circle_20_data[] PROGMEM = {
    0x68, 0x65, 0x9C, 0xDD, 0xC9, 0x47, 0x30, 0x40, 0xDF, 0x90, 0xC0,
    0x37, 0x08, 0xFC, 0x0E, 0x07, 0x50, 0x9F, 0xF0, 0x63, 0x05, 0xFF,
    0x90, 0xE0, 0x32, 0x0D, 0xFF, 0xA8, 0x1B, 0x05, 0xFF, 0xC0, 0x30,
    0xAF, 0xFC, 0x08, 0x0D, 0xFF, 0xC0, 0xB0, 0xEF, 0xFC, 0x0D, 0x0E,
    0xFF, 0xC0, 0x0D, 0xFF, 0xC0, 0xB0, 0xAF, 0xFC, 0x08, 0x05, 0xFF,
    0xC8, 0x13, 0x0D, 0xFF, 0xA0, 0xB2, 0x05, 0xFF, 0x90, 0xE0, 0x33,
    0x09, 0xFF, 0x06, 0x50, 0x8F, 0xC0, 0xE0, 0x77, 0x04, 0x0D, 0xF9,
    0x0C, 0x03, 0x73, 0x86, 0x59, 0xCD, 0xDC, 0x94, 0x60,
};

const RleImage4bppxBiased<Alpha4, PrgMemResource>& circle_20() {
  static RleImage4bppxBiased<Alpha4, PrgMemResource> value(
      20, 20, circle_20_data, Alpha4(color::Black));
  return value;
}

// Image file shadow_20 23x23, 4-bit Alpha,  RLE, 209 bytes.
static const uint8_t shadow_20_data[] PROGMEM = {
    0x72, 0x80, 0x01, 0x77, 0x28, 0x90, 0x12, 0x34, 0x44, 0x43, 0x21, 0x75,
    0x89, 0x32, 0x45, 0x78, 0x88, 0x87, 0x64, 0x31, 0x71, 0x89, 0x61, 0x35,
    0x79, 0xAB, 0xBC, 0xBB, 0xA8, 0x64, 0x16, 0x84, 0x13, 0x58, 0xAC, 0x80,
    0x2D, 0x84, 0xCB, 0x97, 0x41, 0x58, 0x42, 0x58, 0xBC, 0xD8, 0x03, 0xE8,
    0x4D, 0xCA, 0x73, 0x13, 0x86, 0x14, 0x8B, 0xCD, 0xEE, 0xD8, 0x5E, 0xED,
    0xC9, 0x62, 0x38, 0x42, 0x6A, 0xCD, 0xEF, 0x98, 0x94, 0xEE, 0xDB, 0x84,
    0x10, 0x14, 0x8B, 0xDE, 0xFB, 0x89, 0x3E, 0xDC, 0x96, 0x20, 0x15, 0x9C,
    0xDE, 0xFC, 0x89, 0x2E, 0xDA, 0x73, 0x02, 0x5A, 0xCE, 0xEF, 0xC8, 0x91,
    0xED, 0xB8, 0x30, 0x26, 0xAC, 0xEF, 0xD8, 0x91, 0xED, 0xB8, 0x41, 0x26,
    0xAC, 0xEF, 0xD8, 0x92, 0xED, 0xB8, 0x41, 0x25, 0x9C, 0xEE, 0xFC, 0x89,
    0x2E, 0xDB, 0x73, 0x01, 0x48, 0xBD, 0xEF, 0xB8, 0x4E, 0xEC, 0xA6, 0x22,
    0x84, 0x37, 0xAD, 0xEE, 0xFA, 0x84, 0xED, 0xC9, 0x51, 0x28, 0x52, 0x59,
    0xBD, 0xEE, 0xF8, 0x5E, 0xDC, 0xA7, 0x31, 0x28, 0x41, 0x36, 0xAC, 0xD0,
    0xFE, 0xB8, 0x6E, 0xED, 0xCB, 0x85, 0x24, 0x8A, 0x01, 0x47, 0xAB, 0xDD,
    0xEE, 0xEE, 0xDD, 0xCB, 0x85, 0x26, 0x89, 0x72, 0x47, 0x9B, 0xCC, 0xDD,
    0xCC, 0xBA, 0x85, 0x31, 0x78, 0x95, 0x13, 0x57, 0x9A, 0xAA, 0xA9, 0x86,
    0x42, 0x17, 0x28, 0x92, 0x12, 0x35, 0x66, 0x66, 0x54, 0x31, 0x76, 0x00,
    0x18, 0x01, 0x20, 0x17, 0x10,
};

const RleImage4bppxBiased<Alpha4, PrgMemResource>& shadow_20() {
  static RleImage4bppxBiased<Alpha4, PrgMemResource> value(
      23, 23, shadow_20_data, Alpha4(color::Black));
  return value;
}

}  // namespace

}  // namespace roo_windows