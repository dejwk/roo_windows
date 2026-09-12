#include "roo_windows/widgets/progress_bar.h"

#include "roo_display/shape/basic.h"

using namespace roo_display;

namespace roo_windows {

namespace {

constexpr int32_t kMarqueeTravelMs = 1024;
constexpr int32_t kMarqueeLeadInMs = 200;
constexpr int32_t kMarqueePeriodMs = kMarqueeTravelMs + 2 * kMarqueeLeadInMs;

}  // namespace

void ProgressBar::setProgress(int16_t progress) {
  if (progress > 10000) progress = 10000;
  if (progress < -1) progress = -1;
  if (progress_ == progress) return;
  progress_ = progress;
  if (isIndeterminate()) {
    startMarquee();
  } else {
    stopMarquee();
  }
  invalidateInterior();
}

void ProgressBar::startMarquee() {
  context().animations().cancel(*this, kMarquee);
  marquee_phase_ms_ = 0;
  if (!isIndeterminate() || !hasDrawableBounds() ||
      presentationState() != PresentationState::kPresented) {
    return;
  }
  if (context().animations().start(*this, kMarquee,
                                   AnimationSpec::CustomTime()) !=
      AnimationStatus::kOk) {
    marquee_phase_ms_ = 0;
  }
}

void ProgressBar::stopMarquee() {
  context().animations().cancel(*this, kMarquee);
  marquee_phase_ms_ = 0;
}

void ProgressBar::onAnimationFrame(AnimationTag tag,
                                   const AnimationSample& sample) {
  if (tag != kMarquee) {
    Widget::onAnimationFrame(tag, sample);
    return;
  }
  uint16_t phase =
      static_cast<uint16_t>(sample.elapsed.inMillis() % kMarqueePeriodMs);
  if (phase == marquee_phase_ms_) return;
  marquee_phase_ms_ = phase;
  invalidateInterior();
}

void ProgressBar::onPresentationChanged(const PresentationChange& change) {
  if (change.state == PresentationState::kPresented &&
      !change.detached_since_delivery) {
    startMarquee();
  } else {
    stopMarquee();
  }
}

void ProgressBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  if (rect.width() <= 0 || rect.height() <= 0) {
    stopMarquee();
  } else if (isIndeterminate() &&
             presentationState() == PresentationState::kPresented &&
             !context().animations().contains(*this, kMarquee)) {
    startMarquee();
  }
}

void ProgressBar::paint(PaintContext& ctx) const {
  const Theme& th = theme();
  Color c = (color_ == color::Transparent
                 ? th.framework.color.resolve(FrameworkColorRole::kEmphasis)
                 : color_);
  if (progress_ >= 0) {
    // Determinate.
    int16_t xoffset_incomplete = (uint32_t)progress_ * width() / 10000;
    if (xoffset_incomplete > 0) {
      // There is some progress.
      ctx.fillRect(0, 0, xoffset_incomplete - 1, height() - 1, c);
    }
    if (xoffset_incomplete < width()) {
      // There is some left.
      c.set_a(0x80);
      ctx.fillRect(xoffset_incomplete, 0, width() - 1, height() - 1, c);
    }
  } else {
    int16_t offset_start = static_cast<int16_t>(
        static_cast<uint32_t>(static_cast<int32_t>(marquee_phase_ms_) -
                              kMarqueeLeadInMs) *
        width() / kMarqueeTravelMs);
    int16_t offset_end = offset_start + width() / 5;
    if (offset_start < 0) offset_start = 0;
    if (offset_start >= width()) offset_start = width() - 1;
    if (offset_end < 0) offset_end = 0;
    if (offset_end >= width()) offset_end = width() - 1;
    if (offset_start > 0) {
      c.set_a(0x80);
      ctx.fillRect(0, 0, offset_start - 1, height() - 1, c);
    }
    if (offset_start < width() - 1) {
      c.set_a(0xFF);
      ctx.fillRect(offset_start, 0, offset_end, height() - 1, c);
    }
    if (offset_end + 1 < width()) {
      c.set_a(0x80);
      ctx.fillRect(offset_end + 1, 0, width() - 1, height() - 1, c);
    }
  }
}

Dimensions ProgressBar::getSuggestedMinimumDimensions() const {
  return Dimensions(Scaled(20), Scaled(4));
}

}  // namespace roo_windows
