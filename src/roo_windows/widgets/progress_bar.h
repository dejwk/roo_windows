#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

/// Horizontal progress indicator supporting determinate and indeterminate
/// modes.
///
/// Progress is stored as hundredths of a percent in `[0, 10000]`; negative
/// values mean indeterminate, which is rendered as an animated marquee.
class ProgressBar : public Widget {
 public:
  ProgressBar(const Environment& env) : Widget(env), progress_(-1), color_(0) {}

  /// Drives the indeterminate animation timing on each paint pass.
  void paintWidgetContents(PaintContext& ctx) override;

  /// Paints the determinate fill bar, or the current marquee segment when
  /// indeterminate.
  void paint(const Canvas& canvas) const override;

  /// Reports a small fixed footprint (full-parent width, 4 dp tall).
  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::ExactHeight(Scaled(4)));
  }

  Margins getMargins() const override { return Margins(0); }
  Padding getPadding() const override { return Padding(0); }

  /// Overrides the fill color used for both determinate and indeterminate
  /// bars. Transparent defers to the theme primary color.
  void setColor(roo_display::Color color) { color_ = color; }

  /// Switches the bar to indeterminate (marquee) mode.
  void setIndeterminate() { setProgress(-1); }

  /// Sets a determinate progress in hundredths of a percent (`[0, 10000]`).
  /// Negative values switch to indeterminate. No-op if unchanged.
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