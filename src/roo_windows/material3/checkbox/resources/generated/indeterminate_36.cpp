#include "indeterminate_36.h"

using namespace roo_display;

const Pictogram& indeterminate_36() {
  // Image file indeterminate 36x36, 4-bit Alpha,  RLE, 17 bytes.
  static const uint8_t indeterminate_data[] PROGMEM = {
      0x04, 0x0D, 0xFF, 0xA8, 0x1D, 0x4D, 0xFF, 0xC0, 0x0D,
      0xFF, 0xC8, 0x1D, 0x4D, 0xFF, 0xA0, 0xD0, 0x40,
  };

  static Pictogram value(Box(8, 16, 27, 19), Box(0, 0, 35, 35),
                         indeterminate_data, Alpha4(color::Black));
  return value;
}
