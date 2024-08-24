#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class ProgressBar : public Widget {
 public:
  ProgressBar(const Environment& env) : Widget(env), progress_(-1), color_(0) {}

  void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::ExactHeight(Scaled(4)));
  }

  Margins getMargins() const override { return Margins(0); }
  Padding getPadding() const override { return Padding(0); }

  void setColor(roo_display::Color color) { color_ = color; }

  void setIndeterminate() { setProgress(-1); }

  void setProgress(int16_t progress) {
    if (progress > 10000) progress = 10000;
    if (progress < -1) progress = -1;
    if (progress_ == progress) return;
    progress_ = progress;
    setDirty();
  }

  bool isIndeterminate() const { return (progress_ < 0); }

 private:
  // negative = indeterminate. Otherwise, in [0-10000], in 1/100 of a percent.
  int16_t progress_;

  roo_display::Color color_;
};

}  // namespace roo_windows