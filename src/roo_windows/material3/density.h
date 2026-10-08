#pragma once

#include <stdint.h>

namespace roo_windows::material3 {

/// Application-wide compactness for eligible Material 3 whitespace tokens.
/// Typography, icon assets, application spacing, and other component families
/// retain their geometry. Compact footprints are also the exact hit rectangles.
enum class Density : int8_t {
  kDefault = 0,
  kMinus1 = -1,
  kMinus2 = -2,
  kMinus3 = -3,
  kMinus4 = -4,
  kMinus5 = -5,
};

}  // namespace roo_windows::material3
