#pragma once

#include <inttypes.h>
#include <pgmspace.h>

namespace roo_windows {

struct KeySpec {
  enum Function { TEXT, DEL, ENTER, SHIFT, SPACE, SWITCH_PAGE };

  Function function;
  uint32_t data;  // Function-dependent details.
  uint8_t width;  // In grid units.
};

struct KeyboardRowSpec {
  uint8_t start_offset;  // In grid units.
  uint8_t key_count;
  const KeySpec* keys;
};

struct KeyboardPageSpec {
  int8_t row_width;  // How many total grid units per row.
  int8_t row_count;  // How many rows of keys.
  const KeyboardRowSpec* rows;
};

static const PROGMEM KeySpec textKey(uint8_t w, uint32_t rune) {
  return KeySpec{.function = KeySpec::TEXT, .data = rune, .width = w};
}

static const PROGMEM KeySpec spaceKey(uint8_t w) {
  return KeySpec{.function = KeySpec::SPACE, .data = 0x20, .width = w};
}

static const PROGMEM KeySpec fnKey(uint8_t w, KeySpec::Function f) {
  return KeySpec{.function = f, .data = 0, .width = w};
}

}  // namespace roo_windows
