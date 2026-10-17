#pragma once

#include <stdint.h>

namespace roo_windows {
/// Selects application-level presentation defaults, independently of the
/// design systems used by individual widgets. Material 3 is the default.
enum class DesignSystem : uint8_t { kMaterial2, kMaterial3 };
}  // namespace roo_windows
