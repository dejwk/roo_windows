#include "check_13.h"

using namespace roo_display;

const Pictogram& check_13() {
  // Image file check 13x13, 4-bit Alpha,  RLE, 22 bytes.
  static const uint8_t check_data[] PROGMEM = {
      0x70, 0x27, 0x83, 0x8F, 0x20, 0x23, 0x89, 0x18, 0xF8, 0x02, 0xF8,
      0x08, 0xF8, 0x38, 0x38, 0xFC, 0xF8, 0x58, 0x18, 0xF8, 0x70, 0x25,
  };

  static Pictogram value(Box(2, 3, 10, 9), Box(0, 0, 12, 12), check_data,
                         Alpha4(color::Black));
  return value;
}
