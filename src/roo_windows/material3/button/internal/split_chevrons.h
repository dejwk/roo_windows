#pragma once

#include "roo_display/composition/rasterizable_stack.h"
#include "roo_display/shape/smooth.h"

namespace roo_windows::material3::internal {

/// Reusable two-rectangle difference, rotated about a fixed pixel center.
/// Storage is allocated once; geometry updates retain both stack inputs.
class SplitChevron {
 public:
  /// Creates the two reusable stack inputs for subsequent geometry updates.
  SplitChevron();

  SplitChevron(const SplitChevron&) = delete;
  SplitChevron& operator=(const SplitChevron&) = delete;

  /// Sets the enclosing bounds, nominal slot size, and down-to-up fraction.
  void setGeometry(roo_display::Box bounds, int slot, float fraction,
                   roo_display::Color color);

  /// Returns the composed chevron with the enclosing bounds as its anchor.
  const roo_display::RasterizableStack& image() const { return image_; }

 private:
  roo_display::SmoothShape outer_;
  roo_display::SmoothShape cutout_;
  roo_display::RasterizableStack image_;
};

}  // namespace roo_windows::material3::internal
