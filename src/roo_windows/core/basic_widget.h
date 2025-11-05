#pragma once

#include "roo_windows/core/margins.h"
#include "roo_windows/core/padding.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Base class for widgets that wish to support some basic anc cheap padding and
// margin customization. Custom implementation spares 2 bytes per instance to
// keep the padding and margin information.
class BasicWidget : public Widget {
 public:
  BasicWidget(const Environment& env) : Widget(env), padding_(0), margins_(0) {}

  BasicWidget(const BasicWidget& w)
      : Widget(w), padding_(w.padding_), margins_(w.margins_) {}

  virtual Padding getDefaultPadding() const {
    return Padding(PaddingSize::REGULAR);
  }

  virtual Margins getDefaultMargins() const {
    return Margins(MarginSize::REGULAR);
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
    if (hs == PaddingSize::DEFAULT_SIZE || vs == PaddingSize::DEFAULT_SIZE) {
      def = getDefaultPadding();
    }
    h = (hs == PaddingSize::DEFAULT_SIZE) ? (def.left() + def.right()) / 2
                                          : Padding::DimensionForSize(hs);
    v = (vs == PaddingSize::DEFAULT_SIZE) ? (def.top() + def.bottom()) / 2
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
    if (hs == MarginSize::DEFAULT_SIZE || vs == MarginSize::DEFAULT_SIZE) {
      def = getDefaultMargins();
    }
    h = (hs == MarginSize::DEFAULT_SIZE) ? (def.left() + def.right()) / 2
                                         : Margins::DimensionForSize(hs);
    v = (vs == MarginSize::DEFAULT_SIZE) ? (def.top() + def.bottom()) / 2
                                         : Margins::DimensionForSize(vs);
    return Margins(h, v);
  }

 private:
  uint8_t padding_;
  uint8_t margins_;
};

}  // namespace roo_windows
