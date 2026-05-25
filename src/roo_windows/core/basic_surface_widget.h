#pragma once

#include "roo_windows/core/margins.h"
#include "roo_windows/core/padding.h"
#include "roo_windows/core/surface_widget.h"

namespace roo_windows {

// Surface-owning counterpart of BasicWidget.
class BasicSurfaceWidget : public SurfaceWidget {
 public:
  BasicSurfaceWidget(ApplicationContext& context)
      : SurfaceWidget(context), padding_(0), margins_(0) {}

  /// Returns the surface widget's default padding token.
  virtual Padding getDefaultPadding() const {
    return Padding(PaddingSize::kRegular);
  }

  /// Returns the surface widget's default margins token.
  virtual Margins getDefaultMargins() const {
    return Margins(MarginSize::kRegular);
  }

  /// Updates horizontal and vertical padding (independently). Triggers
  /// repaint and relayout when the value actually changes.
  void setPadding(PaddingSize h, PaddingSize v) {
    uint8_t padding = ((int)h << 4) | (int)v;
    if (padding_ == padding) return;
    padding_ = padding;
    invalidateInterior();
    requestLayout();
  }

  /// Sets both horizontal and vertical padding to the same token.
  void setPadding(PaddingSize size) { setPadding(size, size); }

  /// Resolves the configured padding tokens to pixel padding, falling back
  /// to the per-subclass default when a token is `kDefault`.
  Padding getPadding() const override {
    PaddingSize hs = (PaddingSize)(padding_ >> 4);
    PaddingSize vs = (PaddingSize)(padding_ & 15);
    Padding def;
    int16_t h, v;
    if (hs == PaddingSize::kDefault || vs == PaddingSize::kDefault) {
      def = getDefaultPadding();
    }
    h = (hs == PaddingSize::kDefault) ? (def.left() + def.right()) / 2
                                      : Padding::DimensionForSize(hs);
    v = (vs == PaddingSize::kDefault) ? (def.top() + def.bottom()) / 2
                                      : Padding::DimensionForSize(vs);
    return Padding(h, v);
  }

  /// Updates horizontal and vertical margins (independently).
  void setMargins(MarginSize h, MarginSize v) {
    margins_ = ((int)h << 4) | (int)v;
  }

  /// Sets both horizontal and vertical margins to the same token.
  void setMargins(MarginSize size) { setMargins(size, size); }

  /// `BasicSurfaceWidget`s are considered clickable iff an
  /// interactive-change callback is registered. Subclasses that are
  /// intrinsically clickable override this.
  bool isClickable() const override {
    return getOnInteractiveChange() != nullptr;
  }

  /// Resolves the configured margin tokens to pixel margins, falling back
  /// to the per-subclass default when a token is `kDefault`.
  Margins getMargins() const override {
    MarginSize hs = (MarginSize)(margins_ >> 4);
    MarginSize vs = (MarginSize)(margins_ & 15);
    Margins def;
    int16_t h, v;
    if (hs == MarginSize::kDefault || vs == MarginSize::kDefault) {
      def = getDefaultMargins();
    }
    h = (hs == MarginSize::kDefault) ? (def.left() + def.right()) / 2
                                     : Margins::DimensionForSize(hs);
    v = (vs == MarginSize::kDefault) ? (def.top() + def.bottom()) / 2
                                     : Margins::DimensionForSize(vs);
    return Margins(h, v);
  }

 private:
  uint8_t padding_;
  uint8_t margins_;
};

}  // namespace roo_windows