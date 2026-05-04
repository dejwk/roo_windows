#include "indeterminate_13.h"

using namespace roo_display;

const Pictogram& indeterminate_13() {
  // Image file indeterminate 13x13, 4-bit Alpha,  RLE, 9 bytes.
  static const uint8_t indeterminate_data[] PROGMEM = {
      0x02, 0x80, 0x13, 0x02, 0xF0, 0x28, 0x01, 0x30, 0x20,
  };

  static Pictogram value(Box(3, 5, 9, 7), Box(0, 0, 12, 12), indeterminate_data,
                         Alpha4(color::Black));
  return value;
}
