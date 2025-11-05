#include "roo_windows/widgets/slider.h"

#include "roo_display/composition/streamable_stack.h"
#include "roo_display/image/image.h"
#include "roo_display/shape/smooth.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/core/container.h"
#include "roo_windows/widgets/resources/circle.h"
#include "roo_windows/widgets/resources/circular_shadow.h"

using namespace roo_display;

namespace roo_windows {

namespace {
static constexpr int kRadius = Scaled(12);
static constexpr int kOverlayRadius = Scaled(22);

// How close must the click be to be recognized as clicking on the slider.
static constexpr int kTouchSlopPixels = 10;

int16_t xoffset_from_pos(uint16_t pos, int16_t range, Padding p) {
  return kRadius + (((int32_t)pos * range + 32768) >> 16) + p.left();
}

int16_t range_from_width(int16_t width, Padding p) {
  int16_t range = width - 2 * kRadius - p.left() - p.right();
  if (range < 1) return 1;
  return range;
}

uint16_t pos_from_x(XDim x, int16_t range, Padding p) {
  int16_t pos = x + 1 - kRadius - p.left();
  if (pos < 0) pos = 0;
  if (pos >= range) pos = range - 1;
  return ((pos << 16) + range / 2) / range;
}

}  // namespace

bool Slider::onDown(XDim x, YDim y) {
  // We want to allow single-taps to set slider values. Because of that,
  // we cannot reject onDown() events that happen far from the current
  // slider position.
  return true;
}

bool Slider::onSingleTapUp(XDim x, YDim y) {
  BasicWidget::onSingleTapUp(x, y);
  Padding p = getPadding();
  int16_t range = range_from_width(width(), p);
  if (setPos(pos_from_x(x, range, p))) {
    triggerInteractiveChange();
  }
  return true;
}

void Slider::onShowPress(XDim x, YDim y) {
  Padding p = getPadding();
  int16_t range = range_from_width(width(), p);
  uint16_t min_pos = pos_from_x(x - kTouchSlopPixels, range, p);
  uint16_t max_pos = pos_from_x(x + kTouchSlopPixels, range, p);
  bool is_within_bounds = (getPos() >= min_pos) && getPos() <= max_pos;
  if (is_within_bounds) {
    // Enough time was elapsed without vertical movement that we can commit
    // that this is a horizontal drag of the slider.
    is_dragging_ = true;
    Padding p = getPadding();
    int16_t range = range_from_width(width(), p);
    if (setPos(pos_from_x(x, range, p))) {
      triggerInteractiveChange();
    }
    Widget::onShowPress(x, y);
  }
}

bool Slider::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  if (!is_dragging_ && (dy * dy > dx * dx) && dy * dy > 25) {
    //
    return false;
  }
  // if (y < -20 || y > height() + 20) return false;
  Padding p = getPadding();
  int16_t range = range_from_width(width(), p);
  if (setPos(pos_from_x(x, range, p))) {
    // If the initial 'onDown' was outside the slider knob, we were not
    // committed to dragging yet (we could have still withdraw and allow the
    // gesture to be reinterpreted as vertical scroll in the parent component).
    // But now that we have definite movement in the horizontal direction, we
    // commit to dragging.
    is_dragging_ = true;
    setPressed(true);
    triggerInteractiveChange();
  }
  return true;
}

bool Slider::setPos(uint16_t pos) {
  if (pos == pos_) return false;
  Padding p = getPadding();
  int16_t range = range_from_width(width(), p);
  int16_t old_xoffset = xoffset_from_pos(pos_, range, p);
  pos_ = pos;
  int16_t new_xoffset = xoffset_from_pos(pos_, range, p);
  if (old_xoffset != new_xoffset) {
    invalidateInterior(Rect(std::min(old_xoffset, new_xoffset) - kOverlayRadius,
                            height() / 2 - kOverlayRadius,
                            std::max(old_xoffset, new_xoffset) + kOverlayRadius,
                            height() / 2 + kOverlayRadius - 1));
  }
  return true;
}

void Slider::paint(const Canvas& canvas) const {
  const Theme& th = theme();
  Color circleColor = th.color.highlighterColor(canvas.bgcolor());
  Padding p = getPadding();

  int16_t range = range_from_width(width(), p);
  int16_t xoffset = xoffset_from_pos(pos_, range, p);

#if ROO_WINDOWS_ZOOM >= 200
  auto circle = circle_48();
  auto shadow = circular_shadow_48();
#elif ROO_WINDOWS_ZOOM >= 150
  auto circle = circle_36();
  auto shadow = circular_shadow_36();
#elif ROO_WINDOWS_ZOOM >= 100
  auto circle = circle_24();
  auto shadow = circular_shadow_24();
#else
  auto circle = circle_18();
  auto shadow = circular_shadow_18();
#endif
  circle.color_mode().setColor(circleColor);

  auto sel = roo_display::SmoothThickLine(
      {kRadius + p.left() - 0.5f, kRadius - 0.5f},
      {xoffset - 0.5f, kRadius - 0.5f}, Scaled(6), circleColor);
  Color unselColor = circleColor.withA(th.state.disabled);
  auto unsel = roo_display::SmoothThickLine(
      {xoffset + 0.5f, kRadius - 0.5f},
      {parent_bounds().width() - 1 - kRadius - p.right() - 0.5f,
       kRadius - 0.5f},
      Scaled(4), unselColor);
  roo_display::StreamableStack composite(
      roo_display::Box(p.left(), 0, width() - p.right() - 1, 2 * kRadius - 1));
  composite.addInput(&sel);
  composite.addInput(&unsel);
  composite.addInput(&shadow, xoffset - kRadius, 0);
  composite.addInput(&circle, xoffset - kRadius, 0);
  canvas.drawTiled(composite, bounds(), kMiddle, isInvalidated());
}

Dimensions Slider::getSuggestedMinimumDimensions() const {
  return Dimensions(2 * kRadius, 2 * kRadius);
}

PreferredSize Slider::getPreferredSize() const {
  Padding p = getPadding();
  return PreferredSize(
      PreferredSize::MatchParentWidth(),
      PreferredSize::ExactHeight(2 * kRadius + p.top() + p.bottom()));
}

roo_display::FpPoint Slider::getPointOverlayFocus() const {
  float xoffset;
  Padding p = getPadding();

  int16_t range = range_from_width(width(), p);
  xoffset = (float)xoffset_from_pos(pos_, range, p);
  return roo_display::FpPoint{xoffset, (height() - 1) * 0.5f};
}

void Slider::onCancel() {
  BasicWidget::onCancel();
  is_dragging_ = false;
}

}  // namespace roo_windows