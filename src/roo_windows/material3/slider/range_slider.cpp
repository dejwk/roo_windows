#include "roo_windows/material3/slider/range_slider.h"

#include <algorithm>
#include <cmath>

#include "roo_display/composition/streamable_stack.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/material3/slider/slider_internal.h"
#include "roo_windows/material3/slider/value_indicator.h"

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
static constexpr int8_t kNoActiveThumb = -1;
static constexpr float kPressedThumbWidthRatio = 0.5f;

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

// True iff the indicator should be drawn this frame. Range sliders show the
// bubble only above the currently-active thumb during interaction; when no
// thumb is active there is nothing to anchor on, even with kAlways.
bool ShowsValueIndicator(const RangeSlider& widget) {
  if (widget.activeThumbIndex() < 0) return false;
  switch (widget.style().value_indicator) {
    case SliderValueIndicatorBehavior::kHidden:
      return false;
    case SliderValueIndicatorBehavior::kShowOnInteraction:
    case SliderValueIndicatorBehavior::kWithinBounds:
      return widget.isPressed() || widget.isDragged();
    case SliderValueIndicatorBehavior::kAlways:
      return true;
  }
  return false;
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
  float effective_min_separation = ClampMinSeparation(range, min_separation);

  if (active_thumb == 0) {
    normalized_end = NormalizeValueForRange(end_value, range);
    normalized_start = NormalizeValueForRange(start_value, range);
    if (normalized_start <= normalized_end - effective_min_separation) {
      return;
    }
    normalized_start = LargestValidValueAtOrBelow(
        range, normalized_end - effective_min_separation);
    return;
  }
  if (active_thumb == 1) {
    normalized_start = NormalizeValueForRange(start_value, range);
    normalized_end = NormalizeValueForRange(end_value, range);
    if (normalized_end >= normalized_start + effective_min_separation) {
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

float TrackShapeMinPrimary(float visible_min_primary) {
  return visible_min_primary <= 0.0f
             ? -0.5f
             : visible_min_primary - (float)kTrackRadius;
}

int16_t ThumbWidthForState(bool pressed) {
  if (!pressed) return kHandleWidth;
  int16_t width =
      (int16_t)roundf((float)kHandleWidth * kPressedThumbWidthRatio);
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
  return internal::SliderVisualMetrics{
      thumb_center_primary,     track_cross_start,         track_min_cross,
      track_max_cross,          thumb_min_primary,         thumb_max_primary,
      thumb_cross_start,        thumb_min_cross,           thumb_max_cross,
      active_track_max_primary, inactive_track_min_primary};
}

}  // namespace

RangeSlider::RangeSlider(const Environment& env, SliderRange range,
                         float start_value, float end_value, SliderStyle style)
    : BasicWidget(env),
      range_(NormalizeRangeOrDefault(range)),
      style_(style),
      start_value_(range_.from),
      end_value_(range_.to),
      min_separation_(0.0f),
      active_thumb_(kNoActiveThumb),
      overlay_thumb_(kNoActiveThumb),
      is_dragging_(false),
      awaiting_direction_(false) {
  NormalizeOrderedValuesWithSeparation(range_, start_value, end_value,
                                       min_separation_, kNoActiveThumb,
                                       start_value_, end_value_);
  if (IndicatorEnabled(style_)) {
    setParentClipMode(ParentClipMode::kUnclipped);
  }
}

bool RangeSlider::onDown(XDim x, YDim y) {
  (void)x;
  (void)y;
  return isEnabled();
}

bool RangeSlider::onSingleTapUp(XDim x, YDim y) {
  if (!isEnabled()) return false;
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  active_thumb_ = ResolveNearestThumb(axis, start_pos, end_pos, x);
  overlay_thumb_ = active_thumb_;
  awaiting_direction_ = false;
  BasicWidget::onSingleTapUp(x, y);
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
  int thumb =
      ResolveThumbForPress(axis, start_pos, end_pos, x, awaiting_direction);
  if (awaiting_direction) {
    active_thumb_ = kNoActiveThumb;
    awaiting_direction_ = true;
    return;
  }
  if (thumb == kNoActiveThumb) return;

  active_thumb_ = thumb;
  overlay_thumb_ = thumb;
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
    overlay_thumb_ = active_thumb_;
    awaiting_direction_ = false;
    is_dragging_ = true;
    onInteractionStart(active_thumb_);
  } else if (active_thumb_ == kNoActiveThumb) {
    uint16_t start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    active_thumb_ = ResolveNearestThumb(axis, start_pos, end_pos, x);
    overlay_thumb_ = active_thumb_;
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
                 range_.step != range.step ||
                 min_separation_ != new_min_separation ||
                 start_value_ != new_start_value || end_value_ != new_end_value;
  if (!changed) return false;

  range_ = range;
  min_separation_ = new_min_separation;
  start_value_ = new_start_value;
  end_value_ = new_end_value;
  if (old_start_pos !=
          internal::SliderPosFromValue(range_.from, range_.to, start_value_) ||
      old_end_pos !=
          internal::SliderPosFromValue(range_.from, range_.to, end_value_)) {
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
  if (old_start_pos !=
          internal::SliderPosFromValue(range_.from, range_.to, start_value_) ||
      old_end_pos !=
          internal::SliderPosFromValue(range_.from, range_.to, end_value_)) {
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

bool RangeSlider::setStyle(SliderStyle style) {
  if (style_ == style) return false;
  Rect old_bounds = IndicatorEnabled(style_) ? maxParentBounds() : Rect();
  bool was_enabled = IndicatorEnabled(style_);
  style_ = style;
  bool is_enabled = IndicatorEnabled(style_);
  setParentClipMode(is_enabled ? ParentClipMode::kUnclipped
                               : ParentClipMode::kClipped);
  invalidateInterior();
  if (was_enabled || is_enabled) {
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
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

  Rect old_indicator_bounds =
      IndicatorEnabled(style_) ? maxParentBounds() : Rect();
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
    if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
      notifyParentInvalidatedRegion(
          Rect::Extent(old_indicator_bounds, maxParentBounds()));
    }
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
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  int16_t start_thumb_width =
      ThumbWidthForState(isPressed() && active_thumb_ == 0);
  int16_t end_thumb_width =
      ThumbWidthForState(isPressed() && active_thumb_ == 1);
  internal::SliderVisualMetrics start_layout = ResolveVisualMetrics(
      axis, axis.centerFromPos(start_pos), start_thumb_width,
      TrackGapForThumbWidth(start_thumb_width), kHandleHeight);
  internal::SliderVisualMetrics end_layout = ResolveVisualMetrics(
      axis, axis.centerFromPos(end_pos), end_thumb_width,
      TrackGapForThumbWidth(end_thumb_width), kHandleHeight);

  float active_track_min_primary = start_layout.inactive_track_min_primary;
  float active_track_max_primary = end_layout.active_track_max_primary;
  int16_t active_clip_min = (int16_t)ceilf(active_track_min_primary);
  int16_t active_clip_max = (int16_t)floorf(active_track_max_primary);
  if (active_clip_min < 0) active_clip_min = 0;
  if (active_clip_max >= width()) active_clip_max = width() - 1;

  roo_display::Box active_clip(
      active_clip_min, start_layout.track_cross_start, active_clip_max,
      start_layout.track_cross_start + kTrackHeight - 1);
  bool has_left_inactive_clip = true;
  int16_t left_inactive_max =
      (int16_t)floorf(start_layout.active_track_max_primary);
  roo_display::Box left_inactive_clip;
  if (left_inactive_max < 0) {
    has_left_inactive_clip = false;
  } else {
    if (left_inactive_max >= width()) left_inactive_max = width() - 1;
    left_inactive_clip =
        roo_display::Box(0, start_layout.track_cross_start, left_inactive_max,
                         start_layout.track_cross_start + kTrackHeight - 1);
  }
  bool has_right_inactive_clip = true;
  int16_t right_inactive_min =
      (int16_t)ceilf(end_layout.inactive_track_min_primary);
  roo_display::Box right_inactive_clip;
  if (right_inactive_min >= width()) {
    has_right_inactive_clip = false;
  } else {
    if (right_inactive_min < 0) right_inactive_min = 0;
    right_inactive_clip = roo_display::Box(
        right_inactive_min, start_layout.track_cross_start, width() - 1,
        start_layout.track_cross_start + kTrackHeight - 1);
  }

  auto inactive_track = SmoothFilledRoundRect(
      TrackShapeMinPrimary(0.0f), start_layout.track_min_cross, width() - 0.5f,
      start_layout.track_max_cross, kTrackRadius, tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      TrackShapeMinPrimary(active_track_min_primary),
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
  if (has_left_inactive_clip) {
    composite.addInput(&inactive_track, left_inactive_clip);
  }
  if (has_right_inactive_clip) {
    composite.addInput(&inactive_track, right_inactive_clip);
  }
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

roo_display::FpPoint RangeSlider::getPointOverlayFocus() const {
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  int thumb = active_thumb_;
  if (thumb == kNoActiveThumb) {
    thumb = overlay_thumb_;
  }
  float focus_primary = axis.centerFromPos(thumb == 1 ? end_pos : start_pos);
  return roo_display::FpPoint{focus_primary, 0.5f * (float)(height() - 1)};
}

ColorRole RangeSlider::effectiveContainerRole() const {
  return ColorRole::kPrimary;
}

roo::string_view RangeSlider::formatLabel(float value, char* scratch,
                                          size_t scratch_size) const {
  return ValueIndicatorBubble::FormatDefault(value, scratch, scratch_size);
}

Rect RangeSlider::getParentTransientPaintBounds() const {
  // Same logic as Slider::getParentTransientPaintBounds(): union the base
  // bounds with a conservative envelope big enough to cover the bubble at
  // any thumb position. The framework uses this to invalidate the bubble
  // area in the parent when isPressed/isDragged flips.
  Rect base = BasicWidget::getParentTransientPaintBounds();
  if (style_.value_indicator == SliderValueIndicatorBehavior::kHidden) {
    return base;
  }
  bool may_show =
      style_.value_indicator == SliderValueIndicatorBehavior::kAlways ||
      isPressed() || isDragged();
  if (!may_show || width() <= 0 || height() <= 0) return base;
  Rect bubble_local = ValueIndicatorBubble::ConservativeBounds(
      width(), height(), kHandleWidth, style_.value_indicator);
  if (bubble_local.empty()) return base;
  Rect bubble_parent = bubble_local.translate(offsetLeft(), offsetTop());
  return Rect::Extent(base, bubble_parent);
}

void RangeSlider::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  // Pre-paint the active thumb's value indicator bubble before the slider
  // contents. See Slider::paintWidgetContents() for the rationale on
  // ordering and the role of paint() vs decorate(). The canvas is already
  // clipped to maxBounds(), which (via getVisualBounds() ->
  // getTransientPaintBounds() -> our getParentTransientPaintBounds()
  // override) already covers the bubble's conservative envelope when the
  // indicator is showing.
  if (ShowsValueIndicator(*this) && width() > 0 && height() > 0) {
    // ShowsValueIndicator() guarantees activeThumbIndex() >= 0.
    int thumb = active_thumb_;
    float thumb_value = (thumb == 1) ? end_value_ : start_value_;

    char scratch[16];
    roo::string_view text = formatLabel(thumb_value, scratch, sizeof(scratch));

    const Theme& th = theme();
    Color bubble_color =
        isEnabled() ? th.color.inverseSurface : th.color.onSurface.withA(0x3D);
    Color text_color =
        isEnabled() ? th.color.inverseOnSurface : th.color.surface;

    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    uint16_t pos =
        internal::SliderPosFromValue(range_.from, range_.to, thumb_value);
    float thumb_center = axis.centerFromPos(pos);
    bool clamp =
        style_.value_indicator == SliderValueIndicatorBehavior::kWithinBounds;

    ValueIndicatorBubble bubble;
    if (bubble.layout(width(), thumb_center, text, clamp, bubble_color,
                      text_color)) {
      bubble.paint(canvas);
      bubble.decorate(canvas, clipper, OverlaySpec());
    }
  }
  BasicWidget::paintWidgetContents(canvas, clipper);
}

}  // namespace material3
}  // namespace roo_windows
