#include "roo_windows/widgets/progress_bar.h"

#include <Arduino.h>

#include "roo_display/shape/basic.h"

using namespace roo_display;

namespace roo_windows {

void ProgressBar::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  Widget::paintWidgetContents(canvas, clipper);
  if (progress_ < 0) {
    // Indeterminate. Continuous animation.
    setDirty();
  }
}

void ProgressBar::paint(const Canvas& canvas) const {
  const Theme& th = theme();
  Color c = (color_ == color::Transparent ? th.color.primary : color_);
  if (progress_ >= 0) {
    // Determinate.
    int16_t xoffset_incomplete = (uint32_t)progress_ * width() / 10000;
    if (xoffset_incomplete > 0) {
      // There is some progress.
      canvas.fillRect(0, 0, xoffset_incomplete - 1, height() - 1, c);
    }
    if (xoffset_incomplete < width()) {
      // There is some left.
      c.set_a(0x80);
      canvas.fillRect(xoffset_incomplete, 0, width() - 1, height() - 1, c);
    }
  } else {
    int16_t offset_start =
        (uint32_t)(millis() % (1024 + 400) - 200) * width() / 1024;
    int16_t offset_end = offset_start + width() / 5;
    if (offset_start < 0) offset_start = 0;
    if (offset_start >= width()) offset_start = width() - 1;
    if (offset_end < 0) offset_end = 0;
    if (offset_end >= width()) offset_end = width() - 1;
    if (offset_start > 0) {
      c.set_a(0x80);
      canvas.fillRect(0, 0, offset_start - 1, height() - 1, c);
    }
    if (offset_start < width() - 1) {
      c.set_a(0xFF);
      canvas.fillRect(offset_start, 0, offset_end, height() - 1, c);
    }
    if (offset_end + 1 < width()) {
      c.set_a(0x80);
      canvas.fillRect(offset_end + 1, 0, width() - 1, height() - 1, c);
    }
  }
}

Dimensions ProgressBar::getSuggestedMinimumDimensions() const {
  return Dimensions(Scaled(20), Scaled(4));
}

}  // namespace roo_windows