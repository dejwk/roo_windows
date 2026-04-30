#pragma once

#include "roo_windows/core/margins.h"
#include "roo_windows/core/padding.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Surface-owning counterpart of BasicWidget.
class BasicSurfaceWidget : public SurfaceWidget {
 public:
  BasicSurfaceWidget(const Environment& env)
      : SurfaceWidget(env), padding_(0), margins_(0) {}

  virtual Padding getDefaultPadding() const {
    return Padding(PaddingSize::kRegular);
  }

  virtual Margins getDefaultMargins() const {
    return Margins(MarginSize::kRegular);
  }

  void setPadding(PaddingSize h, PaddingSize v) {
    padding_ = ((int)h << 4) | (int)v;
  }

  void setPadding(PaddingSize size) { setPadding(size, size); }

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

  void setMargins(MarginSize h, MarginSize v) {
    margins_ = ((int)h << 4) | (int)v;
  }

  void setMargins(MarginSize size) { setMargins(size, size); }

  bool isClickable() const override {
    return getOnInteractiveChange() != nullptr;
  }

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