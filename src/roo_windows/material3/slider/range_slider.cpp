#include "roo_windows/material3/slider/range_slider.h"

#include <algorithm>
#include <cmath>

#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/material3/slider/slider_internal.h"

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
static constexpr int8_t kNoActiveThumb = -1;

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

struct Tokens {
  Color active_track;
  Color inactive_track;
  Color handle;
};

Tokens ResolveTokens(const RangeSlider& widget) {
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

float ClampMinSeparation(const SliderRange& range, float min_separation) {
  float span = range.to - range.from;
  if (!(min_separation >= 0.0f)) return 0.0f;
  if (min_separation > span) return span;
  return min_separation;
}

float SmallestValidValueAtOrAbove(const SliderRange& range, float value) {
  if (!internal::IsDiscreteSliderRange(range.step)) {
    return internal::ClampSliderValue(value, range.from, range.to);
  }
  float steps = ceilf((value - range.from) / range.step -
                      internal::kSliderStepDivisibilityTolerance);
  return internal::ClampSliderValue(range.from + steps * range.step, range.from,
                                    range.to);
}

float LargestValidValueAtOrBelow(const SliderRange& range, float value) {
  if (!internal::IsDiscreteSliderRange(range.step)) {
    return internal::ClampSliderValue(value, range.from, range.to);
  }
  float steps = floorf((value - range.from) / range.step +
                       internal::kSliderStepDivisibilityTolerance);
  return internal::ClampSliderValue(range.from + steps * range.step, range.from,
                                    range.to);
}

void NormalizeOrderedValues(const SliderRange& range, float start_value,
                           float end_value, float& normalized_start,
                           float& normalized_end) {
  normalized_start = NormalizeValueForRange(start_value, range);
  normalized_end = NormalizeValueForRange(end_value, range);
  if (normalized_start > normalized_end) {
    std::swap(normalized_start, normalized_end);
  }
}

void NormalizeOrderedValuesWithSeparation(const SliderRange& range,
                                         float start_value, float end_value,
                                         float min_separation,
                                         int active_thumb,
                                         float& normalized_start,
                                         float& normalized_end) {
  float effective_min_separation =
      ClampMinSeparation(range, min_separation);

  if (active_thumb == 0) {
    normalized_end = NormalizeValueForRange(end_value, range);
    normalized_start = NormalizeValueForRange(start_value, range);
    if (normalized_start <=
        normalized_end - effective_min_separation) {
      return;
    }
    normalized_start = LargestValidValueAtOrBelow(
        range, normalized_end - effective_min_separation);
    return;
  }
  if (active_thumb == 1) {
    normalized_start = NormalizeValueForRange(start_value, range);
    normalized_end = NormalizeValueForRange(end_value, range);
    if (normalized_end >=
        normalized_start + effective_min_separation) {
      return;
    }
    normalized_end = SmallestValidValueAtOrAbove(
        range, normalized_start + effective_min_separation);
    return;
  }

  NormalizeOrderedValues(range, start_value, end_value, normalized_start,
                         normalized_end);
  if (normalized_end - normalized_start >= effective_min_separation) return;

  float adjusted_end = SmallestValidValueAtOrAbove(
      range, normalized_start + effective_min_separation);
  if (adjusted_end < range.to ||
      std::fabs(adjusted_end - range.to) <=
          internal::kSliderStepDivisibilityTolerance) {
    normalized_end = adjusted_end;
    return;
  }

  normalized_end = range.to;
  normalized_start = LargestValidValueAtOrBelow(
      range, normalized_end - effective_min_separation);
}

Rect UnionRects(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

int ResolveNearestThumb(const internal::SliderAxisMetrics& axis,
                       uint16_t start_pos, uint16_t end_pos,
                       int16_t primary_coord) {
  float start_center = axis.centerFromPos(start_pos);
  float end_center = axis.centerFromPos(end_pos);
  float start_distance = std::fabs((float)primary_coord - start_center);
  float end_distance = std::fabs((float)primary_coord - end_center);
  if (start_distance < end_distance) return 0;
  if (end_distance < start_distance) return 1;
  return primary_coord <= (start_center + end_center) * 0.5f ? 0 : 1;
}

int ResolveThumbForPress(const internal::SliderAxisMetrics& axis,
                        uint16_t start_pos, uint16_t end_pos,
                        int16_t primary_coord, bool& awaiting_direction) {
  bool start_hit = axis.hitsThumb(start_pos, primary_coord, kTouchSlopPixels);
  bool end_hit = axis.hitsThumb(end_pos, primary_coord, kTouchSlopPixels);
  awaiting_direction = false;
  if (start_hit && end_hit) {
    if (start_pos == end_pos) {
      awaiting_direction = true;
      return kNoActiveThumb;
    }
    return ResolveNearestThumb(axis, start_pos, end_pos, primary_coord);
  }
  if (start_hit) return 0;
  if (end_hit) return 1;
  return kNoActiveThumb;
}

Rect InvalidationRectForValueChange(const internal::SliderAxisMetrics& axis,
                                    uint16_t old_start_pos,
                                    uint16_t old_end_pos,
                                    uint16_t new_start_pos,
                                    uint16_t new_end_pos) {
  return UnionRects(
      axis.invalidationRectForPosChange(old_start_pos, new_start_pos),
      axis.invalidationRectForPosChange(old_end_pos, new_end_pos));
}

}  // namespace

RangeSlider::RangeSlider(const Environment& env, SliderRange range,
                         float start_value, float end_value)
    : BasicWidget(env),
      range_(NormalizeRangeOrDefault(range)),
      start_value_(range_.from),
      end_value_(range_.to),
      min_separation_(0.0f),
      active_thumb_(kNoActiveThumb),
      is_dragging_(false),
      awaiting_direction_(false) {
  NormalizeOrderedValuesWithSeparation(range_, start_value, end_value,
                                       min_separation_, kNoActiveThumb,
                                       start_value_, end_value_);
}

bool RangeSlider::onDown(XDim x, YDim y) {
  (void)x;
  (void)y;
  return isEnabled();
}

bool RangeSlider::onSingleTapUp(XDim x, YDim y) {
  if (!isEnabled()) return false;
  BasicWidget::onSingleTapUp(x, y);
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  active_thumb_ = ResolveNearestThumb(axis, start_pos, end_pos, x);
  awaiting_direction_ = false;
  onInteractionStart(active_thumb_);
  if (setActiveThumbPos(axis.posFromPrimaryCoord(x))) {
    triggerInteractiveChange();
  }
  onInteractionEnd(start_value_, end_value_);
  active_thumb_ = kNoActiveThumb;
  is_dragging_ = false;
  return true;
}

void RangeSlider::onShowPress(XDim x, YDim y) {
  if (!isEnabled()) return;
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  bool awaiting_direction = false;
  int thumb = ResolveThumbForPress(axis, start_pos, end_pos, x,
                                   awaiting_direction);
  if (awaiting_direction) {
    active_thumb_ = kNoActiveThumb;
    awaiting_direction_ = true;
    return;
  }
  if (thumb == kNoActiveThumb) return;

  active_thumb_ = thumb;
  awaiting_direction_ = false;
  if (!is_dragging_) {
    is_dragging_ = true;
    onInteractionStart(active_thumb_);
  }
  if (setActiveThumbPos(axis.posFromPrimaryCoord(x))) {
    triggerInteractiveChange();
  }
  Widget::onShowPress(x, y);
}

bool RangeSlider::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  (void)y;
  if (!isEnabled()) return false;
  if (!internal::ShouldCaptureHorizontalSliderScroll(
          is_dragging_ || awaiting_direction_, dx, dy)) {
    return false;
  }

  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  if (awaiting_direction_) {
    if (dx < 0) {
      active_thumb_ = 0;
    } else if (dx > 0) {
      active_thumb_ = 1;
    } else {
      return true;
    }
    awaiting_direction_ = false;
    is_dragging_ = true;
    onInteractionStart(active_thumb_);
  } else if (active_thumb_ == kNoActiveThumb) {
    uint16_t start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    active_thumb_ = ResolveNearestThumb(axis, start_pos, end_pos, x);
    is_dragging_ = true;
    onInteractionStart(active_thumb_);
  }

  bool changed = setActiveThumbPos(axis.posFromPrimaryCoord(x));
  if (changed) {
    setPressed(true);
    triggerInteractiveChange();
  }
  return true;
}

void RangeSlider::onCancel() {
  if (active_thumb_ != kNoActiveThumb) {
    onInteractionEnd(start_value_, end_value_);
  }
  BasicWidget::onCancel();
  active_thumb_ = kNoActiveThumb;
  is_dragging_ = false;
  awaiting_direction_ = false;
}

bool RangeSlider::setRange(SliderRange range) {
  if (!internal::IsValidSliderRange(range.from, range.to, range.step)) {
    return false;
  }

  uint16_t old_start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t old_end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);

  float new_start_value;
  float new_end_value;
  float new_min_separation = ClampMinSeparation(range, min_separation_);
  NormalizeOrderedValuesWithSeparation(range, start_value_, end_value_,
                                       new_min_separation, kNoActiveThumb,
                                       new_start_value, new_end_value);

  bool changed = range_.from != range.from || range_.to != range.to ||
                 range_.step != range.step || min_separation_ != new_min_separation ||
                 start_value_ != new_start_value || end_value_ != new_end_value;
  if (!changed) return false;

  range_ = range;
  min_separation_ = new_min_separation;
  start_value_ = new_start_value;
  end_value_ = new_end_value;
  if (old_start_pos != internal::SliderPosFromValue(range_.from, range_.to,
                                                    start_value_) ||
      old_end_pos != internal::SliderPosFromValue(range_.from, range_.to,
                                                  end_value_)) {
    onValueChange(start_value_, end_value_, kNoActiveThumb, false);
  }

  if (width() > 0 && height() > 0) {
    uint16_t new_start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t new_end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    Rect invalidation = InvalidationRectForValueChange(
        axis, old_start_pos, old_end_pos, new_start_pos, new_end_pos);
    invalidateInterior(invalidation);
  }
  return true;
}

bool RangeSlider::setMinSeparation(float value) {
  if (!std::isfinite(value) || value < 0.0f) return false;
  float effective_value = ClampMinSeparation(range_, value);
  float new_start_value;
  float new_end_value;
  NormalizeOrderedValuesWithSeparation(range_, start_value_, end_value_,
                                       effective_value, kNoActiveThumb,
                                       new_start_value, new_end_value);
  bool changed = min_separation_ != effective_value ||
                 start_value_ != new_start_value || end_value_ != new_end_value;
  if (!changed) return false;

  uint16_t old_start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t old_end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  min_separation_ = effective_value;
  start_value_ = new_start_value;
  end_value_ = new_end_value;
  if (old_start_pos != internal::SliderPosFromValue(range_.from, range_.to,
                                                    start_value_) ||
      old_end_pos != internal::SliderPosFromValue(range_.from, range_.to,
                                                  end_value_)) {
    onValueChange(start_value_, end_value_, kNoActiveThumb, false);
  }

  if (width() > 0 && height() > 0) {
    uint16_t new_start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t new_end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    invalidateInterior(InvalidationRectForValueChange(
        axis, old_start_pos, old_end_pos, new_start_pos, new_end_pos));
  }
  return true;
}

bool RangeSlider::setValues(float start_value, float end_value) {
  return setValuesInternal(start_value, end_value, false, kNoActiveThumb);
}

bool RangeSlider::setValuesInternal(float start_value, float end_value,
                                    bool from_user, int active_thumb) {
  float new_start_value;
  float new_end_value;
  NormalizeOrderedValuesWithSeparation(range_, start_value, end_value,
                                       min_separation_, active_thumb,
                                       new_start_value, new_end_value);
  if (start_value_ == new_start_value && end_value_ == new_end_value) {
    return false;
  }

  uint16_t old_start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t old_end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);

  start_value_ = new_start_value;
  end_value_ = new_end_value;
  onValueChange(start_value_, end_value_, active_thumb,
                from_user && active_thumb != kNoActiveThumb);

  if (width() > 0 && height() > 0) {
    uint16_t new_start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t new_end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    Rect invalidation = InvalidationRectForValueChange(
        axis, old_start_pos, old_end_pos, new_start_pos, new_end_pos);
    invalidateInterior(invalidation);
  }
  return true;
}

bool RangeSlider::setActiveThumbPos(uint16_t pos) {
  float value =
      internal::SliderValueFromNormalizedPos(range_.from, range_.to, pos);
  if (active_thumb_ == 0) {
    return setValuesInternal(value, end_value_, true, 0);
  }
  if (active_thumb_ == 1) {
    return setValuesInternal(start_value_, value, true, 1);
  }
  return false;
}

void RangeSlider::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos = internal::SliderPosFromValue(range_.from, range_.to,
                                                  end_value_);
  internal::SliderVisualMetrics start_layout =
      internal::ResolveHorizontalSliderVisualMetrics(
          axis, start_pos, kTrackHeight, kTrackHandleGap, kHandleHeight);
  internal::SliderVisualMetrics end_layout =
      internal::ResolveHorizontalSliderVisualMetrics(
          axis, end_pos, kTrackHeight, kTrackHandleGap, kHandleHeight);

  float active_track_min_primary = start_layout.inactive_track_min_primary;
  float active_track_max_primary = end_layout.active_track_max_primary;
  int16_t active_clip_min = (int16_t)ceilf(active_track_min_primary);
  int16_t active_clip_max = (int16_t)floorf(active_track_max_primary);
  if (active_clip_min < 0) active_clip_min = 0;
  if (active_clip_max >= width()) active_clip_max = width() - 1;

  roo_display::Box active_clip(active_clip_min, start_layout.track_cross_start,
                               active_clip_max,
                               start_layout.track_cross_start +
                                   kTrackHeight - 1);

  auto inactive_track = SmoothFilledRoundRect(
      -(float)kTrackRadius - 0.5f, start_layout.track_min_cross,
      width() - 0.5f, start_layout.track_max_cross, kTrackRadius,
      tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      active_track_min_primary - (float)kTrackRadius,
      start_layout.track_min_cross,
      active_track_max_primary + (float)kTrackRadius,
      start_layout.track_max_cross, kTrackRadius, tokens.active_track);
  auto start_handle = SmoothFilledRoundRect(
      start_layout.thumb_min_primary, start_layout.thumb_min_cross,
      start_layout.thumb_max_primary, start_layout.thumb_max_cross,
      kHandleCornerRadius, tokens.handle);
  auto end_handle = SmoothFilledRoundRect(
      end_layout.thumb_min_primary, end_layout.thumb_min_cross,
      end_layout.thumb_max_primary, end_layout.thumb_max_cross,
      kHandleCornerRadius, tokens.handle);

  roo_display::StreamableStack composite(
      roo_display::Box(0, 0, width() - 1, height() - 1));
  composite.addInput(&inactive_track);
  if (!active_clip.empty()) {
    composite.addInput(&active_track, active_clip);
  }
  composite.addInput(&start_handle);
  composite.addInput(&end_handle);
  canvas.drawTiled(composite, bounds(), kCenter | kMiddle, isInvalidated());
}

Dimensions RangeSlider::getSuggestedMinimumDimensions() const {
  return Dimensions(kHandleHeight, kHandleHeight);
}

PreferredSize RangeSlider::getPreferredSize() const {
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::ExactHeight(kHandleHeight));
}

ColorRole RangeSlider::effectiveContainerRole() const {
  return ColorRole::kPrimary;
}

}  // namespace material3
}  // namespace roo_windows