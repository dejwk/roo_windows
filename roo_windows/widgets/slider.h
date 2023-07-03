#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Slider : public BasicWidget {
 public:
  Slider(const Environment& env, uint16_t pos = 0)
      : BasicWidget(env), pos_(pos) {}

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  bool isClickable() const override { return true; }

  bool onDown(XDim x, YDim y) override;

  bool supportsScrolling() const override { return true; }

  bool usesHighlighterColor() const override { return true; }

  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  PreferredSize getPreferredSize() const override;

  OverlayType getOverlayType() const override {
    return OVERLAY_POINT;
  }

  roo_display::FpPoint getPointOverlayFocus() const override;

 private:
  uint16_t pos_;
};

}  // namespace roo_windows