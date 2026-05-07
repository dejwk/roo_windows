#include "roo_windows/material3/slider/slider.h"

#include <algorithm>
#include <cmath>

#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/core/overlay_spec.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kTrackHeight = Scaled(16);
static constexpr int kTrackRadius = kTrackHeight / 2;
static constexpr int kTrackHandleGap = Scaled(2);
static constexpr int kHandleWidth = Scaled(4);
static constexpr int kHandleHeight = Scaled(44);
static constexpr float kHandleCornerRadius = 0.5f * (float)kHandleWidth;
static constexpr int kTouchSlopPixels = Scaled(20);
static constexpr int kInteractionRadius = kPointOverlayDiameter / 2;

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

int16_t RangeFromWidth(int16_t width) {
  int16_t range = width - kHandleWidth;
  return range < 1 ? 1 : range;
}

float CenterXFromPos(uint16_t pos, int16_t range) {
  return 0.5f * (float)(kHandleWidth - 1) +
         (float)(((uint32_t)pos * (uint32_t)range + 32768u) >> 16);
}

uint16_t PosFromX(XDim x, int16_t range) {
  int32_t pos = x + 1 - kHandleWidth / 2;
  if (pos < 0) pos = 0;
  if (pos >= range) pos = range - 1;
  return (((uint32_t)pos << 16) + (uint32_t)(range / 2)) / (uint32_t)range;
}

struct Tokens {
  Color active_track;
  Color inactive_track;
  Color handle;
};

Tokens ResolveTokens(const Slider& widget) {
  const Theme& theme = widget.theme();
  if (!widget.isEnabled()) {
    return Tokens{
        DisabledComposite(theme.color.onSurface, 0x61, theme),
        DisabledComposite(theme.color.onSurface, 0x1F, theme),
        DisabledComposite(theme.color.onSurface, 0x61, theme),
    };
  }

  return Tokens{
      theme.color.primary,
      theme.color.secondaryContainer,
      theme.color.primary,
  };
}

}  // namespace

bool Slider::onDown(XDim x, YDim y) {
  (void)x;
  (void)y;
  return isEnabled();
}

bool Slider::onSingleTapUp(XDim x, YDim y) {
  if (!isEnabled()) return false;
  BasicWidget::onSingleTapUp(x, y);
  int16_t range = RangeFromWidth(width());
  uint16_t pos = PosFromX(x, range);
  if (setPos(pos)) {
    triggerInteractiveChange();
  }
  return true;
}

void Slider::onShowPress(XDim x, YDim y) {
  (void)y;
  if (!isEnabled()) return;
  int16_t range = RangeFromWidth(width());
  uint16_t min_pos = PosFromX(x - kTouchSlopPixels, range);
  uint16_t max_pos = PosFromX(x + kTouchSlopPixels, range);
  if (getPos() < min_pos || getPos() > max_pos) return;

  is_dragging_ = true;
  if (setPos(PosFromX(x, range))) {
    triggerInteractiveChange();
  }
  Widget::onShowPress(x, y);
}

bool Slider::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  (void)y;
  if (!isEnabled()) return false;
  if (!is_dragging_ && (dy * dy > dx * dx) && dy * dy > 25) {
    return false;
  }

  int16_t range = RangeFromWidth(width());
  if (setPos(PosFromX(x, range))) {
    is_dragging_ = true;
    setPressed(true);
    triggerInteractiveChange();
  }
  return true;
}

void Slider::onCancel() {
  BasicWidget::onCancel();
  is_dragging_ = false;
}

bool Slider::setPos(uint16_t pos) {
  if (pos == pos_) return false;
  int16_t range = RangeFromWidth(width());
  float old_center = CenterXFromPos(pos_, range);
  pos_ = pos;
  if (width() <= 0 || height() <= 0) {
    return true;
  }
  float new_center = CenterXFromPos(pos_, range);
  int16_t min_x =
      (int16_t)std::min(old_center, new_center) - kInteractionRadius;
  int16_t max_x =
      (int16_t)std::max(old_center, new_center) + kInteractionRadius;
  invalidateInterior(Rect(min_x, 0, max_x, height() - 1));
  return true;
}

void Slider::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  int16_t range = RangeFromWidth(width());
  float center_x = CenterXFromPos(pos_, range);

  int16_t track_top_px = (height() - kTrackHeight) / 2;
  float track_min_y = track_top_px - 0.5f;
  float track_max_y = track_top_px + kTrackHeight - 0.5f;

  float handle_min_x = center_x - 0.5f * (float)(kHandleWidth - 1) - 0.5f;
  float handle_max_x = handle_min_x + kHandleWidth;
  int16_t handle_top_px = (height() - kHandleHeight) / 2;
  float handle_min_y = handle_top_px - 0.5f;
  float handle_max_y = handle_min_y + kHandleHeight;

  float active_track_max_x = handle_min_x - (float)kTrackHandleGap;
  float inactive_track_min_x = handle_max_x + (float)kTrackHandleGap;

  int16_t active_x_max = (int16_t)floorf(active_track_max_x);
  if (active_x_max < 0) active_x_max = -1;
  if (active_x_max >= width()) active_x_max = width() - 1;

  int16_t inactive_x_min = (int16_t)ceilf(inactive_track_min_x);
  if (inactive_x_min < 0) inactive_x_min = 0;
  if (inactive_x_min > width()) inactive_x_min = width();

  roo_display::Box active_clip(0, track_top_px, active_x_max,
                               track_top_px + kTrackHeight - 1);
  roo_display::Box inactive_clip(inactive_x_min, track_top_px, width() - 1,
                                 track_top_px + kTrackHeight - 1);

  auto inactive_track = SmoothFilledRoundRect(
      inactive_track_min_x - (float)kTrackRadius, track_min_y, width() - 0.5f,
      track_max_y, kTrackRadius, tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      -0.5f, track_min_y, active_track_max_x + (float)kTrackRadius, track_max_y,
      kTrackRadius, tokens.active_track);
  auto handle =
      SmoothFilledRoundRect(handle_min_x, handle_min_y, handle_max_x,
                            handle_max_y, kHandleCornerRadius, tokens.handle);

  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, width() - 1, height() - 1));
  if (!inactive_clip.empty()) {
    composite.addInput(&inactive_track, inactive_clip);
  }
  if (!active_clip.empty()) {
    composite.addInput(&active_track, active_clip);
  }
  composite.addInput(&handle);
  canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions Slider::getSuggestedMinimumDimensions() const {
  return Dimensions(kHandleHeight, kHandleHeight);
}

PreferredSize Slider::getPreferredSize() const {
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::ExactHeight(kHandleHeight));
}

roo_display::FpPoint Slider::getPointOverlayFocus() const {
  int16_t range = RangeFromWidth(width());
  return roo_display::FpPoint{CenterXFromPos(pos_, range),
                              0.5f * (float)(height() - 1)};
}

ColorRole Slider::effectiveContainerRole() const { return ColorRole::kPrimary; }

}  // namespace material3
}  // namespace roo_windows