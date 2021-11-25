#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class HorizontalDivider : public Widget {
 public:
  using Widget::Widget;
  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, 2);
  }

  Margins getDefaultMargins() const override { return Margins(1); }

  Padding getDefaultPadding() const override { return Padding(0); }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParent(), PreferredSize::Exact(2));
  }
};

class VerticalDivider : public Widget {
 public:
  using Widget::Widget;
  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(2, 0);
  }

  Margins getDefaultMargins() const override { return Margins(1); }

  Padding getDefaultPadding() const override { return Padding(0); }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::Exact(2), PreferredSize::MatchParent());
  }
};

}  // namespace roo_windows