#include "indeterminate_27.h"

using namespace roo_display;

const Pictogram& indeterminate_27() {
  // Image file indeterminate 27x27, 4-bit Alpha,  RLE, 9 bytes.
  static const uint8_t indeterminate_data[] PROGMEM = {
      0x08, 0xFE, 0x08, 0x0E, 0xFE, 0x0E, 0x08, 0xFE, 0x08,
  };

  static Pictogram value(Box(6, 12, 20, 14), Box(0, 0, 26, 26),
                         indeterminate_data, Alpha4(color::Black));
  return value;
}
