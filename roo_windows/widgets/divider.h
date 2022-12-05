#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class HorizontalDivider : public Widget {
 public:
  using Widget::Widget;
  bool paint(const Canvas& canvas) override;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, 2);
  }

  Margins getMargins() const override { return Margins(1); }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::ExactHeight(2));
  }
};

class VerticalDivider : public Widget {
 public:
  using Widget::Widget;
  bool paint(const Canvas& canvas) override;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(2, 0);
  }

  Margins getMargins() const override { return Margins(1); }

  Padding getPadding() const override { return Padding(0); }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(2),
                         PreferredSize::MatchParentHeight());
  }
};

}  // namespace roo_windows
