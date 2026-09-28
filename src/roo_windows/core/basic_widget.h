#pragma once

#include "roo_windows/core/margins_mixin.h"
#include "roo_windows/core/padding_mixin.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Compatibility base for widgets that want both stored padding and margins.
// New specialized widgets can inherit PaddingMixin or MarginsMixin directly
// when they need only one of those capabilities.
class BasicWidget : public MarginsMixin<PaddingMixin<Widget>> {
 public:
  using Base = MarginsMixin<PaddingMixin<Widget>>;

  BasicWidget(ApplicationContext& context) : Base(context) {}

  /// `BasicWidget`s are considered clickable iff an interactive-change
  /// callback is registered. Subclasses that are intrinsically clickable
  /// override this.
  bool isClickable() const override { return hasInteractiveChangeHandler(); }
};

}  // namespace roo_windows
