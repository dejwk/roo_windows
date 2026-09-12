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
  ProgressBar(ApplicationContext& context)
      : Widget(context), progress_(-1), marquee_phase_ms_(0), color_(0) {
    context.presentations().observe(*this);
  }

  /// Paints the determinate fill bar, or the current marquee segment when
  /// indeterminate.
  void paint(PaintContext& ctx) const override;

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
  void setProgress(int16_t progress);

  bool isIndeterminate() const { return (progress_ < 0); }

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onPresentationChanged(const PresentationChange& change) override;
  void onLayout(bool changed, const Rect& rect) override;

  static constexpr AnimationTag kMarquee = 0;
  uint16_t appliedMarqueePhaseMs() const { return marquee_phase_ms_; }

 private:
  void startMarquee();
  void stopMarquee();
  bool hasDrawableBounds() const { return width() > 0 && height() > 0; }

  // negative = indeterminate. Otherwise, in [0-10000], in 1/100 of a percent.
  int16_t progress_;

  // Applied position within the 1424 ms marquee waveform. Timing is owned by
  // the application animation registry.
  uint16_t marquee_phase_ms_;

  roo_display::Color color_;
};

}  // namespace roo_windows
