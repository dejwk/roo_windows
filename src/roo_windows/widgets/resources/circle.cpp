#include "roo_windows/widgets/resources/circle.h"

#include "roo_display/color/named.h"

namespace roo_windows {

using namespace roo_display;

// Image file circle_18 18x18, 4-bit Alpha,  RLE, 63 bytes.
static const uint8_t circle_18_data[] PROGMEM = {
    0x68, 0x23, 0x55, 0x37, 0x30, 0x80, 0xEC, 0x0E, 0x08, 0x60, 0x10,
    0xCF, 0x90, 0xC0, 0x14, 0x0C, 0xFB, 0x0C, 0x30, 0x8F, 0xD0, 0x82,
    0x0E, 0xFD, 0x81, 0xE0, 0x3F, 0xF0, 0x30, 0x5F, 0xF0, 0x05, 0xFF,
    0x05, 0x03, 0xFF, 0x81, 0x30, 0xEF, 0xD0, 0xE2, 0x08, 0xFD, 0x08,
    0x30, 0xCF, 0xB0, 0xC4, 0x01, 0x0C, 0xF9, 0x0C, 0x01, 0x60, 0x80,
    0xEC, 0x0E, 0x08, 0x73, 0x82, 0x35, 0x53, 0x60,
};

const Pictogram& circle_18() {
  static Pictogram value(Box(1, 1, 16, 16), Box(0, 0, 17, 17), circle_18_data,
                         Alpha4(color::Black));
  return value;
}

// Image file circle_24 24x24, 4-bit Alpha,  RLE, 82 bytes.
static const uint8_t circle_24_data[] PROGMEM = {
    0x68, 0x63, 0x8B, 0xDD, 0xB8, 0x37, 0x30, 0x30, 0xCF, 0x90, 0xC0, 0x37,
    0x06, 0xFD, 0x06, 0x50, 0x6F, 0xF0, 0x63, 0x03, 0xFF, 0xA0, 0x32, 0x0C,
    0xFF, 0xA8, 0x1C, 0x03, 0xFF, 0xC0, 0x30, 0x8F, 0xFC, 0x08, 0x0B, 0xFF,
    0xC0, 0xB0, 0xDF, 0xFC, 0x00, 0xDF, 0xFC, 0x0D, 0x0B, 0xFF, 0xC0, 0xB0,
    0x8F, 0xFC, 0x08, 0x03, 0xFF, 0xC8, 0x13, 0x0C, 0xFF, 0xA0, 0xC2, 0x03,
    0xFF, 0xA0, 0x33, 0x06, 0xFF, 0x06, 0x50, 0x6F, 0xD0, 0x67, 0x03, 0x0C,
    0xF9, 0x0C, 0x03, 0x73, 0x86, 0x38, 0xBD, 0xDB, 0x83, 0x60,
};

const Pictogram& circle_24() {
  static Pictogram value(Box(2, 2, 21, 21), Box(0, 0, 23, 23), circle_24_data,
                         Alpha4(color::Black));
  return value;
}

// Image file circle_36 36x36, 4-bit Alpha,  RLE, 152 bytes.
static const uint8_t circle_36_data[] PROGMEM = {
    0x73, 0x89, 0x01, 0x69, 0xBC, 0xCB, 0x96, 0x17, 0x74, 0x04, 0x0B, 0xFB,
    0x0B, 0x04, 0x77, 0x02, 0x0B, 0xFF, 0x0B, 0x02, 0x74, 0x05, 0x0E, 0xFF,
    0xA0, 0xE0, 0x57, 0x20, 0x7F, 0xFE, 0x07, 0x70, 0x5F, 0xFF, 0x90, 0x55,
    0x02, 0x0E, 0xFF, 0xF9, 0x0E, 0x02, 0x40, 0xBF, 0xFF, 0xB0, 0xB3, 0x04,
    0xFF, 0xFD, 0x04, 0x20, 0xBF, 0xFF, 0xD8, 0x1B, 0x01, 0xFF, 0xFF, 0x01,
    0x06, 0xFF, 0xFF, 0x06, 0x09, 0xFF, 0xFF, 0x09, 0x0B, 0xFF, 0xFF, 0x0B,
    0x0C, 0xFF, 0xFF, 0x00, 0xCF, 0xFF, 0xF0, 0xC0, 0xBF, 0xFF, 0xF0, 0xB0,
    0x9F, 0xFF, 0xF0, 0x90, 0x6F, 0xFF, 0xF0, 0x60, 0x1F, 0xFF, 0xF8, 0x11,
    0x0B, 0xFF, 0xFD, 0x0B, 0x20, 0x4F, 0xFF, 0xD0, 0x43, 0x0B, 0xFF, 0xFB,
    0x0B, 0x40, 0x20, 0xEF, 0xFF, 0x90, 0xE0, 0x25, 0x05, 0xFF, 0xF9, 0x05,
    0x70, 0x7F, 0xFE, 0x07, 0x72, 0x05, 0x0E, 0xFF, 0xA0, 0xE0, 0x57, 0x40,
    0x20, 0xBF, 0xF0, 0xB0, 0x27, 0x70, 0x40, 0xBF, 0xB0, 0xB0, 0x47, 0x74,
    0x89, 0x01, 0x69, 0xBC, 0xCB, 0x96, 0x17, 0x30,
};

const Pictogram& circle_36() {
  static Pictogram value(Box(3, 3, 32, 32), Box(0, 0, 35, 35), circle_36_data,
                         Alpha4(color::Black));
  return value;
}

// Image file circle_48 48x48, 4-bit Alpha,  RLE, 227 bytes.
static const uint8_t circle_48_data[] PROGMEM = {
    0x77, 0x18, 0x90, 0x36, 0x8A, 0xBB, 0xA8, 0x63, 0x77, 0x76, 0x81, 0x3A,
    0xEF, 0xB8, 0x1E, 0xA3, 0x77, 0x71, 0x04, 0x0C, 0xFF, 0xA0, 0xC0, 0x47,
    0x74, 0x02, 0x0B, 0xFF, 0xE0, 0xB0, 0x27, 0x71, 0x04, 0x0E, 0xFF, 0xF9,
    0x0E, 0x04, 0x76, 0x06, 0xFF, 0xFD, 0x06, 0x74, 0x06, 0xFF, 0xFF, 0x06,
    0x72, 0x04, 0xFF, 0xFF, 0xA0, 0x47, 0x02, 0x0E, 0xFF, 0xFF, 0xA0, 0xE0,
    0x26, 0x0B, 0xFF, 0xFF, 0xC0, 0xB5, 0x04, 0xFF, 0xFF, 0xE0, 0x44, 0x0C,
    0xFF, 0xFF, 0xE0, 0xC3, 0x03, 0x80, 0xC0, 0xF0, 0x32, 0x0A, 0x80, 0xC0,
    0xF0, 0xA2, 0x0E, 0x80, 0xC0, 0xF8, 0x1E, 0x03, 0x80, 0xC2, 0xF0, 0x30,
    0x68, 0x0C, 0x2F, 0x06, 0x08, 0x80, 0xC2, 0xF0, 0x80, 0xA8, 0x0C, 0x2F,
    0x0A, 0x0B, 0x80, 0xC2, 0xF0, 0x0B, 0x80, 0xC2, 0xF0, 0xB0, 0xA8, 0x0C,
    0x2F, 0x0A, 0x08, 0x80, 0xC2, 0xF0, 0x80, 0x68, 0x0C, 0x2F, 0x06, 0x03,
    0x80, 0xC2, 0xF8, 0x13, 0x0E, 0x80, 0xC0, 0xF0, 0xE2, 0x0A, 0x80, 0xC0,
    0xF0, 0xA2, 0x03, 0x80, 0xC0, 0xF0, 0x33, 0x0C, 0xFF, 0xFF, 0xE0, 0xC4,
    0x04, 0xFF, 0xFF, 0xE0, 0x45, 0x0B, 0xFF, 0xFF, 0xC0, 0xB6, 0x02, 0x0E,
    0xFF, 0xFF, 0xA0, 0xE0, 0x27, 0x04, 0xFF, 0xFF, 0xA0, 0x47, 0x20, 0x6F,
    0xFF, 0xF0, 0x67, 0x40, 0x6F, 0xFF, 0xD0, 0x67, 0x60, 0x40, 0xEF, 0xFF,
    0x90, 0xE0, 0x47, 0x71, 0x02, 0x0B, 0xFF, 0xE0, 0xB0, 0x27, 0x74, 0x04,
    0x0C, 0xFF, 0xA0, 0xC0, 0x47, 0x77, 0x18, 0x13, 0xAE, 0xFB, 0x81, 0xEA,
    0x37, 0x77, 0x68, 0x90, 0x36, 0x8A, 0xBB, 0xA8, 0x63, 0x77, 0x10,
};

const Pictogram& circle_48() {
  static Pictogram value(Box(4, 4, 43, 43), Box(0, 0, 47, 47), circle_48_data,
                         Alpha4(color::Black));
  return value;
}

}  // namespace roo_windows