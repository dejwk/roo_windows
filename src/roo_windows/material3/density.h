#pragma once

#include <assert.h>
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

/// Selects an explicit component density or a live application fallback.
/// Unlike explicit kDefault, an inherited policy follows shared-theme changes.
class DensityOverride {
 public:
  /// Creates a policy that inherits the application density.
  constexpr DensityOverride() : density_(static_cast<Density>(1)) {}

  /// Pins component geometry to @p density, including explicit level zero.
  /// Invalid casts assert in debug and become explicit kDefault in release.
  static constexpr DensityOverride Explicit(Density density) {
    int8_t level = static_cast<int8_t>(density);
    if (level < -5 || level > 0) {
      assert(false && "invalid Material 3 density");
      density = Density::kDefault;
    }
    return DensityOverride(density);
  }

  /// Returns whether this policy follows the application setting.
  constexpr bool isInherited() const {
    return density_ == static_cast<Density>(1);
  }

  /// Resolves this policy against the live @p application_density.
  constexpr Density resolve(Density application_density) const {
    return isInherited() ? application_density : density_;
  }

  /// Compares policy identity, distinguishing inheritance from explicit zero.
  constexpr bool operator==(DensityOverride other) const {
    return density_ == other.density_;
  }

  /// Returns whether two policies differ.
  constexpr bool operator!=(DensityOverride other) const {
    return !(*this == other);
  }

 private:
  explicit constexpr DensityOverride(Density density) : density_(density) {}

  Density density_;
};

static_assert(sizeof(DensityOverride) == 1);

}  // namespace roo_windows::material3
