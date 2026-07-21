#pragma once

#include <stdint.h>

namespace roo_windows {

/// Selects the physical ordering used for logical leading/trailing layout.
enum class LayoutDirection : uint8_t {
  kLeftToRight,
  kRightToLeft,
};

}  // namespace roo_windows
