#pragma once

#include <cstdint>
#include <type_traits>
#include <utility>

#include "roo_windows/core/padding.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

/// Adds configurable, theme-token based padding to a Widget subclass.
///
/// This is a base-wrapping mixin: use it as `PaddingMixin<WidgetBase>`, where
/// `WidgetBase` is a Widget subclass. This keeps a single Widget inheritance
/// chain, so getPadding() remains the Widget virtual override.
template <typename WidgetBase,
          PaddingSize DefaultPaddingSize = PaddingSize::kNone>
class PaddingMixin : public WidgetBase {
  static_assert(std::is_base_of<Widget, WidgetBase>::value,
                "PaddingMixin base must derive from Widget");

 public:
  template <typename... Args>
  explicit PaddingMixin(Args&&... args)
      : WidgetBase(std::forward<Args>(args)...) {}

  /// Returns the padding used for axes configured as kDefault. Subclasses may
  /// override this to provide asymmetric or runtime-dependent defaults.
  virtual Padding getDefaultPadding() const {
    return Padding(DefaultPaddingSize);
  }

  /// Updates horizontal and vertical padding independently.
  void setPadding(PaddingSize horizontal, PaddingSize vertical) {
    const uint8_t padding = (static_cast<uint8_t>(horizontal) << 4) |
                            static_cast<uint8_t>(vertical);
    if (padding_ == padding) return;
    padding_ = padding;
    this->invalidateInterior();
    this->requestLayout();
  }

  /// Sets both horizontal and vertical padding to the same token.
  void setPadding(PaddingSize size) { setPadding(size, size); }

  Padding getPadding() const override {
    const PaddingSize horizontal = static_cast<PaddingSize>(padding_ >> 4);
    const PaddingSize vertical = static_cast<PaddingSize>(padding_ & 0x0f);
    Padding defaults;
    if (horizontal == PaddingSize::kDefault ||
        vertical == PaddingSize::kDefault) {
      defaults = getDefaultPadding();
    }
    return Padding(
        horizontal == PaddingSize::kDefault
            ? (defaults.left() + defaults.right()) / 2
            : Padding::DimensionForSize(horizontal),
        vertical == PaddingSize::kDefault
            ? (defaults.top() + defaults.bottom()) / 2
            : Padding::DimensionForSize(vertical));
  }

 private:
  // kDefault / kDefault. Each nibble stores one PaddingSize token.
  uint8_t padding_ = 0;
};

}  // namespace roo_windows
