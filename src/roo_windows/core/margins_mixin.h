#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "roo_windows/core/margins.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

/// Adds configurable, theme-token based margins to a Widget subclass.
///
/// This is a base-wrapping mixin: use it as `MarginsMixin<WidgetBase>`, where
/// `WidgetBase` is a Widget subclass. It can be composed with PaddingMixin.
template <typename WidgetBase,
          MarginSize DefaultMarginsSize = MarginSize::kNone>
class MarginsMixin : public WidgetBase {
  static_assert(std::is_base_of<Widget, WidgetBase>::value,
                "MarginsMixin base must derive from Widget");

 public:
  template <typename... Args>
  explicit MarginsMixin(Args&&... args)
      : WidgetBase(std::forward<Args>(args)...) {}

  /// Returns the margins used for axes configured as kDefault. Subclasses may
  /// override this to provide asymmetric or runtime-dependent defaults.
  virtual Margins getDefaultMargins() const {
    return Margins(DefaultMarginsSize);
  }

  /// Updates horizontal and vertical margins independently.
  void setMargins(MarginSize horizontal, MarginSize vertical) {
    const uint8_t margins = (static_cast<uint8_t>(horizontal) << 4) |
                            static_cast<uint8_t>(vertical);
    if (margins_ == margins) return;
    margins_ = margins;
    this->requestLayout();
  }

  /// Sets both horizontal and vertical margins to the same token.
  void setMargins(MarginSize size) { setMargins(size, size); }

  Margins getMargins() const override {
    const MarginSize horizontal = static_cast<MarginSize>(margins_ >> 4);
    const MarginSize vertical = static_cast<MarginSize>(margins_ & 0x0f);
    Margins defaults;
    if (horizontal == MarginSize::kDefault ||
        vertical == MarginSize::kDefault) {
      defaults = getDefaultMargins();
    }
    return Margins(
        horizontal == MarginSize::kDefault
            ? (defaults.left() + defaults.right()) / 2
            : Margins::DimensionForSize(horizontal),
        vertical == MarginSize::kDefault
            ? (defaults.top() + defaults.bottom()) / 2
            : Margins::DimensionForSize(vertical));
  }

 private:
  // kDefault / kDefault. Each nibble stores one MarginSize token.
  uint8_t margins_ = 0;
};

}  // namespace roo_windows
