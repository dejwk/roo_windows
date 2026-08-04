#pragma once

#include <stdint.h>

namespace roo_windows {
namespace material3 {

// Expressive Material 3 size tier shared by the standard-button, icon-button,
// split-button, and button-group families. Each component resolves this tier
// through its own geometry tokens; equal tiers do not imply equal dimensions.
enum class ButtonSize : uint8_t {
  kExtraSmall,
  kSmall,
  kMedium,
  kLarge,
  kExtraLarge,
};

// Resting Material 3 corner family shared by button families that expose the
// round/square selector. Each component retains its own radius token table.
enum class ButtonShape : uint8_t {
  kRound,
  kSquare,
};

}  // namespace material3
}  // namespace roo_windows
