#pragma once

#include <cstdint>

namespace roo_windows {

/// Describes whether a widget is connected to a live, visible window tree.
enum class PresentationState : uint8_t {
  kDetached,
  kHidden,
  kPresented,
};

}  // namespace roo_windows
