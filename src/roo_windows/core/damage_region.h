#pragma once

#include <cstdint>
#include <vector>

#include "roo_display/core/box.h"

namespace roo_windows::internal {

/// Retains a bounded, conservative union of device-coordinate damage boxes.
/// No operation allocates except appendComplement() through its caller's
/// vector.
class DamageRegion {
 public:
  static constexpr uint8_t kCapacity = 4;

  /// Discards all pending damage.
  void clear() { count_ = 0; }

  /// Adds clipped device damage, merging outward when capacity is exceeded.
  void add(roo_display::Box clipped_damage);

  /// Reports whether the union is empty.
  bool empty() const { return count_ == 0; }

  /// Reports the number of retained boxes.
  uint8_t size() const { return count_; }

  /// Returns one retained box in deterministic insertion/merge order.
  const roo_display::Box& at(uint8_t index) const { return rects_[index]; }

  /// Returns the union's bounding box, or an empty box.
  roo_display::Box bounds() const;

  /// Bounds the damage inside @p clip without constructing gap fragments.
  roo_display::Box intersectionBounds(const roo_display::Box& clip) const;

  /// Proves that every pixel of @p candidate lies in the retained union.
  bool covers(const roo_display::Box& candidate) const;

  /// Appends disjoint boxes covering precisely @p clip outside the union.
  /// Produces no more than 45 boxes at the fixed four-member capacity.
  void appendComplement(const roo_display::Box& clip,
                        std::vector<roo_display::Box>& output) const;

 private:
  roo_display::Box rects_[kCapacity];
  uint8_t count_ = 0;
};

}  // namespace roo_windows::internal
