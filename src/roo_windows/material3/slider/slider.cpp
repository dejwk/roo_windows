#include "roo_windows/material3/slider/slider.h"

#include <algorithm>
#include <cmath>

#include "roo_windows/material3/slider/slider_internal.h"
#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/core/overlay_spec.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kTrackHeight = Scaled(16);
static constexpr int kTrackRadius = kTrackHeight / 2;
static constexpr int kTrackHandleGap = Scaled(6);
static constexpr int kHandleWidth = Scaled(4);
static constexpr int kHandleHeight = Scaled(44);
static constexpr float kHandleCornerRadius = 0.5f * (float)kHandleWidth;
static constexpr int kTouchSlopPixels = Scaled(20);
static constexpr int kInteractionRadius = kPointOverlayDiameter / 2;
static constexpr float kPressedThumbWidthRatio = 0.5f;

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
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

SliderRange NormalizeRangeOrDefault(SliderRange range) {
  if (internal::IsValidSliderRange(range.from, range.to, range.step)) {
    return range;
  }
  return SliderRange{};
}

float NormalizeValueForRange(float value, const SliderRange& range) {
  return internal::NormalizeSliderValueForRange(value, range.from, range.to,
                                                range.step);
}

float TrackShapeMinPrimary(float visible_min_primary) {
  return visible_min_primary <= 0.0f
             ? -0.5f
             : visible_min_primary - (float)kTrackRadius;
}

int16_t ThumbWidthForState(bool pressed) {
  if (!pressed) return kHandleWidth;
  int16_t width = (int16_t)roundf((float)kHandleWidth * kPressedThumbWidthRatio);
  return width < 1 ? 1 : width;
}

int16_t TrackGapForThumbWidth(int16_t thumb_width) {
  int16_t gap = kTrackHandleGap - (kHandleWidth - thumb_width) / 2;
  return gap < 0 ? 0 : gap;
}

internal::SliderVisualMetrics ResolveVisualMetrics(
    const internal::SliderAxisMetrics& axis, float thumb_center_primary,
    int16_t thumb_width, int16_t track_gap, int16_t thumb_cross_span) {
  int16_t track_cross_start = axis.centeredCrossStart(kTrackHeight);
  float track_min_cross = track_cross_start - 0.5f;
  float track_max_cross = track_cross_start + kTrackHeight - 0.5f;
  float thumb_min_primary =
      thumb_center_primary - 0.5f * (float)(thumb_width - 1) - 0.5f;
  float thumb_max_primary = thumb_min_primary + thumb_width;
  int16_t thumb_cross_start = axis.centeredCrossStart(thumb_cross_span);
  float thumb_min_cross = thumb_cross_start - 0.5f;
  float thumb_max_cross = thumb_min_cross + thumb_cross_span;
  float active_track_max_primary = thumb_min_primary - (float)track_gap;
  float inactive_track_min_primary = thumb_max_primary + (float)track_gap;
  return internal::SliderVisualMetrics{thumb_center_primary,
                                       track_cross_start,
                                       track_min_cross,
                                       track_max_cross,
                                       thumb_min_primary,
                                       thumb_max_primary,
                                       thumb_cross_start,
                                       thumb_min_cross,
                                       thumb_max_cross,
                                       active_track_max_primary,
                                       inactive_track_min_primary};
}

}  // namespace

Slider::Slider(const Environment& env, uint16_t pos)
    : Slider(env, SliderRange{},
             internal::SliderValueFromNormalizedPos(0.0f, 1.0f, pos)) {}

Slider::Slider(const Environment& env, SliderRange range, float value,
               SliderVariant variant)
    : BasicWidget(env),
      range_(NormalizeRangeOrDefault(range)),
      variant_(variant),
      value_(NormalizeValueForRange(value, range_)),
      is_dragging_(false) {}

bool Slider::onDown(XDim x, YDim y) {
  (void)x;
  (void)y;
  return isEnabled();
}

bool Slider::onSingleTapUp(XDim x, YDim y) {
  if (!isEnabled()) return false;
  BasicWidget::onSingleTapUp(x, y);
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  uint16_t pos = axis.posFromPrimaryCoord(x);
  onInteractionStart();
  if (setPosInternal(pos, true)) {
    triggerInteractiveChange();
  }
  onInteractionEnd(value_);
  return true;
}

void Slider::onShowPress(XDim x, YDim y) {
  (void)y;
  if (!isEnabled()) return;
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  if (!axis.hitsThumb(getPos(), x, kTouchSlopPixels)) return;

  if (!is_dragging_) {
    is_dragging_ = true;
    onInteractionStart();
  }
  if (setPosInternal(axis.posFromPrimaryCoord(x), true)) {
    triggerInteractiveChange();
  }
  Widget::onShowPress(x, y);
}

bool Slider::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  (void)y;
  if (!isEnabled()) return false;
  if (!internal::ShouldCaptureHorizontalSliderScroll(is_dragging_, dx, dy)) {
    return false;
  }

  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  bool was_dragging = is_dragging_;
  if (setPosInternal(axis.posFromPrimaryCoord(x), true)) {
    if (!was_dragging) {
      onInteractionStart();
    }
    is_dragging_ = true;
    setPressed(true);
    triggerInteractiveChange();
  } else if (was_dragging) {
    is_dragging_ = true;
  }
  return true;
}

void Slider::onCancel() {
  if (is_dragging_) {
    onInteractionEnd(value_);
  }
  BasicWidget::onCancel();
  is_dragging_ = false;
}

bool Slider::setPos(uint16_t pos) {
  return setPosInternal(pos, false);
}

bool Slider::setPosInternal(uint16_t pos, bool from_user) {
  uint16_t old_pos = getPos();
  if (pos == old_pos) return false;
  value_ = NormalizeValueForRange(
      internal::SliderValueFromNormalizedPos(range_.from, range_.to, pos),
      range_);
  uint16_t new_pos = getPos();
  if (new_pos == old_pos) return false;
  onValueChange(value_, from_user);
  if (width() <= 0 || height() <= 0) {
    return true;
  }
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  invalidateInterior(axis.invalidationRectForPosChange(old_pos, new_pos));
  return true;
}

bool Slider::setRange(SliderRange range) {
  if (!internal::IsValidSliderRange(range.from, range.to, range.step)) {
    return false;
  }
  uint16_t old_pos = getPos();
  float new_value = NormalizeValueForRange(value_, range);
  bool changed = range_.from != range.from || range_.to != range.to ||
                 range_.step != range.step || value_ != new_value;
  if (!changed) return false;

  range_ = range;
  value_ = new_value;
  onValueChange(value_, false);

  uint16_t new_pos = getPos();
  if (width() > 0 && height() > 0 && old_pos != new_pos) {
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    invalidateInterior(axis.invalidationRectForPosChange(old_pos, new_pos));
  }
  return true;
}

bool Slider::setVariant(SliderVariant variant) {
  if (variant_ == variant) return false;
  variant_ = variant;
  invalidateInterior();
  return true;
}

bool Slider::setValue(float value) {
  float new_value = NormalizeValueForRange(value, range_);
  if (value_ == new_value) return false;

  uint16_t old_pos = getPos();
  value_ = new_value;
  onValueChange(value_, false);
  uint16_t new_pos = getPos();

  if (width() > 0 && height() > 0 && old_pos != new_pos) {
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    invalidateInterior(axis.invalidationRectForPosChange(old_pos, new_pos));
  }
  return true;
}

uint16_t Slider::getPos() const {
  return internal::SliderPosFromValue(range_.from, range_.to, value_);
}

void Slider::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  float center_anchor_primary = axis.centerFromPos(32768);
  bool thumb_on_or_right_of_center = axis.centerFromPos(getPos()) >= center_anchor_primary;
  int16_t thumb_width = ThumbWidthForState(isPressed());
  int16_t track_gap = TrackGapForThumbWidth(thumb_width);
  internal::SliderVisualMetrics layout = ResolveVisualMetrics(
      axis, axis.centerFromPos(getPos()), thumb_width, track_gap,
      kHandleHeight);

  float active_track_min_primary = -0.5f;
  float active_track_max_primary = layout.active_track_max_primary;
  roo_display::Box active_clip(0, layout.track_cross_start,
                 layout.activeClipMax(width()),
                 layout.track_cross_start + kTrackHeight - 1);
  bool has_left_inactive_clip = false;
  bool has_right_inactive_clip = true;
  roo_display::Box left_inactive_clip;
  roo_display::Box right_inactive_clip(
      (int16_t)ceilf(layout.inactive_track_min_primary),
      layout.track_cross_start, width() - 1,
      layout.track_cross_start + kTrackHeight - 1);

  if (variant_ == SliderVariant::kCentered) {
    active_track_min_primary = thumb_on_or_right_of_center
                                   ? center_anchor_primary
                                   : layout.inactive_track_min_primary;
    active_track_max_primary = thumb_on_or_right_of_center
                                   ? layout.active_track_max_primary
                                   : center_anchor_primary;

    int16_t active_clip_min = (int16_t)ceilf(active_track_min_primary);
    int16_t active_clip_max = (int16_t)floorf(active_track_max_primary);
    if (active_clip_min < 0) active_clip_min = 0;
    if (active_clip_max >= width()) active_clip_max = width() - 1;
    active_clip = roo_display::Box(active_clip_min, layout.track_cross_start,
                     active_clip_max,
                     layout.track_cross_start + kTrackHeight - 1);

    has_left_inactive_clip = true;
    int16_t left_inactive_max = thumb_on_or_right_of_center
                                    ? (int16_t)floorf(center_anchor_primary)
                                    : (int16_t)floorf(layout.active_track_max_primary);
    if (left_inactive_max < 0) {
      has_left_inactive_clip = false;
    } else {
      left_inactive_clip = roo_display::Box(
          0, layout.track_cross_start,
          left_inactive_max >= width() ? width() - 1 : left_inactive_max,
          layout.track_cross_start + kTrackHeight - 1);
    }

    int16_t right_inactive_min = thumb_on_or_right_of_center
                                     ? (int16_t)ceilf(layout.inactive_track_min_primary)
                                     : (int16_t)ceilf(center_anchor_primary);
    if (right_inactive_min >= width()) {
      has_right_inactive_clip = false;
    } else {
      if (right_inactive_min < 0) right_inactive_min = 0;
      right_inactive_clip = roo_display::Box(
          right_inactive_min, layout.track_cross_start, width() - 1,
          layout.track_cross_start + kTrackHeight - 1);
    }
  } else {
    int16_t right_inactive_min = (int16_t)ceilf(layout.inactive_track_min_primary);
    if (right_inactive_min >= width()) {
      has_right_inactive_clip = false;
    } else {
      if (right_inactive_min < 0) right_inactive_min = 0;
      right_inactive_clip = roo_display::Box(
          right_inactive_min, layout.track_cross_start, width() - 1,
          layout.track_cross_start + kTrackHeight - 1);
    }
  }

  auto inactive_track = SmoothFilledRoundRect(
      TrackShapeMinPrimary(0.0f), layout.track_min_cross,
      width() - 0.5f, layout.track_max_cross,
      kTrackRadius, tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
    TrackShapeMinPrimary(active_track_min_primary), layout.track_min_cross,
    active_track_max_primary + (float)kTrackRadius,
      layout.track_max_cross, kTrackRadius, tokens.active_track);
  auto handle =
      SmoothFilledRoundRect(layout.thumb_min_primary, layout.thumb_min_cross,
                            layout.thumb_max_primary, layout.thumb_max_cross,
                            kHandleCornerRadius, tokens.handle);

  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, width() - 1, height() - 1));
  if (has_left_inactive_clip) {
    composite.addInput(&inactive_track, left_inactive_clip);
  }
  if (has_right_inactive_clip) {
    composite.addInput(&inactive_track, right_inactive_clip);
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
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  return roo_display::FpPoint{axis.centerFromPos(getPos()),
                              0.5f * (float)(height() - 1)};
}

ColorRole Slider::effectiveContainerRole() const { return ColorRole::kPrimary; }

}  // namespace material3
}  // namespace roo_windows