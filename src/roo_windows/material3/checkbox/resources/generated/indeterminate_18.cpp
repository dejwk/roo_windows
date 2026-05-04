#include "indeterminate_18.h"

using namespace roo_display;

const Pictogram& indeterminate_18() {
  // Image file indeterminate 18x18, 4-bit Alpha,  RLE, 6 bytes.
  static const uint8_t indeterminate_data[] PROGMEM = {
      0x0B, 0xF9, 0x00, 0xBF, 0x90, 0xB0,
  };

  static Pictogram value(Box(4, 8, 13, 9), Box(0, 0, 17, 17),
                         indeterminate_data, Alpha4(color::Black));
  return value;
}
