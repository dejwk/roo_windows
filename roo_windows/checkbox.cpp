#include "checkbox.h"

#include "roo_display/image/image.h"
#include "roo_display/ui/tile.h"

using namespace roo_display;

namespace roo_windows {

// Image file checkbox_off_alpha_18 15x15, 4-bit Alpha,  RLE, 80 bytes.
static const uint8_t checkbox_off_alpha_18_data[] PROGMEM = {
    0x03, 0x07, 0x80, 0x90, 0x88, 0x16, 0x68, 0x80, 0x76, 0x83, 0x78, 0x68,
    0x28, 0x06, 0x18, 0x36, 0x86, 0x82, 0x80, 0x61, 0x83, 0x68, 0x68, 0x28,
    0x06, 0x18, 0x36, 0x86, 0x82, 0x80, 0x61, 0x83, 0x68, 0x68, 0x28, 0x06,
    0x18, 0x36, 0x86, 0x82, 0x80, 0x61, 0x83, 0x68, 0x68, 0x28, 0x06, 0x18,
    0x36, 0x86, 0x82, 0x80, 0x61, 0x83, 0x68, 0x68, 0x28, 0x06, 0x18, 0x36,
    0x86, 0x82, 0x80, 0x61, 0x83, 0x68, 0x68, 0x38, 0x06, 0x28, 0x16, 0x85,
    0x80, 0x92, 0x80, 0x20, 0x58, 0x09, 0x06, 0x03,
};

const RleImage4bppxPolarized<Alpha4, PrgMemResource>& checkbox_off_alpha_18() {
  static RleImage4bppxPolarized<Alpha4, PrgMemResource> value(
      15, 15, checkbox_off_alpha_18_data, Alpha4(color::Black));
  return value;
}

static const uint8_t checkbox_on_alpha_18_data[] PROGMEM = {
    0x08, 0xFE, 0x09, 0x0E, 0xFF, 0x0E, 0xFF, 0x0E, 0xFB, 0x0D, 0x0C,
    0xA0, 0xEF, 0xA8, 0x1D, 0x35, 0xA0, 0xEF, 0x98, 0x2D, 0x35, 0xEA,
    0x0E, 0xA0, 0xCC, 0x82, 0xD4, 0x5E, 0xB8, 0x3E, 0xFA, 0x2C, 0xA8,
    0x2D, 0x45, 0xEC, 0x89, 0x0E, 0xFE, 0x72, 0xCD, 0x45, 0xED, 0x0E,
    0xA8, 0x4E, 0x72, 0x34, 0xDE, 0x0E, 0xB8, 0x2E, 0x74, 0xDF, 0x0E,
    0xC0, 0x0E, 0xF9, 0x0E, 0xFF, 0x0E, 0xFF, 0x09, 0xFE, 0x09,
};

const RleImage4bppxPolarized<Alpha4, PrgMemResource>& checkbox_on_alpha_18() {
  static RleImage4bppxPolarized<Alpha4, PrgMemResource> value(
      15, 15, checkbox_on_alpha_18_data, Alpha4(color::Black));
  return value;
}

void Checkbox::defaultPaint(const Surface& s) {
  Color color = on_ ? theme().color.highlighterColor(s.bgcolor())
                    : theme().color.defaultColor(s.bgcolor());
  const RleImage4bppxPolarized<Alpha4, PrgMemResource>& ref =
      on_ ? checkbox_on_alpha_18() : checkbox_off_alpha_18();
  RleImage4bppxPolarized<Alpha4, PrgMemResource> img(
      ref.extents(), ref.resource(), Alpha4(color));
  roo_display::Tile tile(&img, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle(),
                         roo_display::color::Transparent, s.fill_mode());
  s.drawObject(tile);
}

}  // namespace roo_windows