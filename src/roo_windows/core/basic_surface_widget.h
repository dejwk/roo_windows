#pragma once

#include "roo_windows/core/margins_mixin.h"
#include "roo_windows/core/padding_mixin.h"
#include "roo_windows/core/surface_widget.h"

namespace roo_windows {

// Compatibility base for surface widgets that want stored padding and
// margins. Its regular defaults match the former BasicSurfaceWidget behavior.
class BasicSurfaceWidget
    : public MarginsMixin<PaddingMixin<SurfaceWidget, PaddingSize::kRegular>,
                          MarginSize::kRegular> {
 public:
  using Base = MarginsMixin<PaddingMixin<SurfaceWidget, PaddingSize::kRegular>,
                            MarginSize::kRegular>;

  BasicSurfaceWidget(ApplicationContext& context) : Base(context) {}

  /// `BasicSurfaceWidget`s are considered clickable iff an
  /// interactive-change callback is registered. Subclasses that are
  /// intrinsically clickable override this.
  bool isClickable() const override { return hasInteractiveChangeHandler(); }
};

}  // namespace roo_windows