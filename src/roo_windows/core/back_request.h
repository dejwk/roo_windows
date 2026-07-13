#pragma once

#include <cstdint>

namespace roo_windows {

/// Identifies the input that initiated a semantic back request.
enum class BackSource : uint8_t {
  kProgrammatic,
  kBackKey,
  kEscapeKey,
  kNavigationButton,
};

/// Reports whether a semantic back request was consumed.
enum class BackResult : uint8_t { kUnhandled, kHandled };

}  // namespace roo_windows
