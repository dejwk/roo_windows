#include "roo_windows/material3/slider/range_slider.h"

#include <algorithm>
#include <cmath>

#include "roo_display/shape/smooth.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/material3/slider/slider_internal.h"
#include "roo_windows/material3/slider/slider_paint_internal.h"
#include "roo_windows/material3/slider/slider_size_internal.h"
#include "roo_windows/material3/slider/value_indicator.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kTouchSlopPixels = Scaled(20);
static constexpr int8_t kNoActiveThumb = -1;
using Tokens = internal::SliderPaintTokens;
using StopSegment = internal::SliderPaintStopSegment;
using internal::DrawTrackPiece;
using internal::GetHandleTileBounds;
using internal::IndicatorDirtyRectFromSpan;
using internal::MakeSliderAxisMetrics;
using internal::PaintStopRuns;
using internal::ResolveTokens;
using internal::ShouldRenderStops;
using internal::ThumbWidthForState;
using internal::TrackGapForThumbWidth;
using internal::TrackSegmentClipBox;
using internal::TrackShapeMinPrimary;

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

// Finds the first legal snapped value on or above the requested bound.
float SmallestValidValueAtOrAbove(const SliderRange& range, float value) {
  if (!internal::IsDiscreteSliderRange(range.step)) {
    return internal::ClampSliderValue(value, range.from, range.to);
  }
  float steps = ceilf((value - range.from) / range.step -
                      internal::kSliderStepDivisibilityTolerance);
  return internal::ClampSliderValue(range.from + steps * range.step, range.from,
                                    range.to);
}

// Finds the last legal snapped value on or below the requested bound.
float LargestValidValueAtOrBelow(const SliderRange& range, float value) {
  if (!internal::IsDiscreteSliderRange(range.step)) {
    return internal::ClampSliderValue(value, range.from, range.to);
  }
  float steps = floorf((value - range.from) / range.step +
                       internal::kSliderStepDivisibilityTolerance);
  return internal::ClampSliderValue(range.from + steps * range.step, range.from,
                                    range.to);
}

// Normalizes two values into the slider domain, preserving the invariant that
// start <= end when no thumb-specific constraint needs to be honored.
void NormalizeOrderedValues(const SliderRange& range, float start_value,
                            float end_value, float& normalized_start,
                            float& normalized_end) {
  normalized_start = internal::NormalizeSliderValueForRange(
      start_value, range.from, range.to, range.step);
  normalized_end = internal::NormalizeSliderValueForRange(end_value, range.from,
                                                          range.to, range.step);
  if (normalized_start > normalized_end) {
    std::swap(normalized_start, normalized_end);
  }
}

// Normalizes two values while preserving the active thumb when possible. If the
// requested move would violate min separation, the moving thumb is clamped onto
// the nearest legal discrete or continuous value instead of swapping thumbs.
void NormalizeOrderedValuesWithSeparation(const SliderRange& range,
                                          float start_value, float end_value,
                                          float min_separation,
                                          int active_thumb,
                                          float& normalized_start,
                                          float& normalized_end) {
  float effective_min_separation = ClampMinSeparation(range, min_separation);

  if (active_thumb == 0) {
    normalized_end = internal::NormalizeSliderValueForRange(
        end_value, range.from, range.to, range.step);
    normalized_start = internal::NormalizeSliderValueForRange(
        start_value, range.from, range.to, range.step);
    if (normalized_start <= normalized_end - effective_min_separation) {
      return;
    }
    normalized_start = LargestValidValueAtOrBelow(
        range, normalized_end - effective_min_separation);
    return;
  }
  if (active_thumb == 1) {
    normalized_start = internal::NormalizeSliderValueForRange(
        start_value, range.from, range.to, range.step);
    normalized_end = internal::NormalizeSliderValueForRange(
        end_value, range.from, range.to, range.step);
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

// Simple helper for combining repaint envelopes produced by the two thumbs.
Rect UnionRects(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

// Chooses the thumb whose center is nearer to the interaction point.
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

// Resolves which thumb should react to a press. When both thumbs are stacked on
// the same position, defer the choice until scroll direction disambiguates it.
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

// Produces the minimal combined thumb repaint rect for any thumb positions that
// changed in this update.
Rect InvalidationRectForValueChange(const internal::SliderAxisMetrics& axis,
                                    uint16_t old_start_pos,
                                    uint16_t old_end_pos,
                                    uint16_t new_start_pos,
                                    uint16_t new_end_pos) {
  Rect start_rect =
      old_start_pos == new_start_pos
          ? Rect(0, 0, -1, -1)
          : axis.invalidationRectForPosChange(old_start_pos, new_start_pos);
  Rect end_rect =
      old_end_pos == new_end_pos
          ? Rect(0, 0, -1, -1)
          : axis.invalidationRectForPosChange(old_end_pos, new_end_pos);
  return UnionRects(start_rect, end_rect);
}

struct ThumbPaintMetrics {
  uint16_t pos;
  int16_t track_gap;
  internal::SliderVisualMetrics layout;
};

// Resolves the per-thumb paint metrics used to lay out the two independent
// handles against the shared track.
ThumbPaintMetrics ResolveThumbPaintMetrics(
    const SliderRange& range, float value,
    const internal::SliderAxisMetrics& axis,
    const internal::SliderSizeMetrics& size_metrics, bool pressed) {
  uint16_t pos = internal::SliderPosFromValue(range.from, range.to, value);
  int16_t thumb_width = ThumbWidthForState(size_metrics.handle_width, pressed);
  int16_t track_gap = TrackGapForThumbWidth(size_metrics, thumb_width);
  return ThumbPaintMetrics{
      pos,
      track_gap,
      internal::ResolveSliderVisualMetrics(
          axis, axis.centerFromPos(pos), thumb_width, size_metrics.track_height,
          track_gap, size_metrics.handle_height),
  };
}

// Splits the range slider track into the same inactive/active/inactive segment
// model used by both track painting and stop rendering.
void BuildTrackSegments(const Tokens& tokens,
                        const internal::SliderAxisMetrics& axis,
                        const ThumbPaintMetrics& start,
                        const ThumbPaintMetrics& end, StopSegment* segments,
                        int& segment_count) {
  segment_count = 0;

  int16_t left_inactive_max =
      (int16_t)floorf(start.layout.active_track_max_primary);
  if (left_inactive_max >= 0) {
    segments[segment_count++] = StopSegment{
        0.0f,
        (float)std::min<int16_t>(left_inactive_max, axis.primarySpan() - 1),
        tokens.inactive_track, tokens.inactive_stop};
  }

  float active_track_min_primary = start.layout.inactive_track_min_primary;
  float active_track_max_primary = end.layout.active_track_max_primary;
  if (active_track_max_primary >= active_track_min_primary) {
    segments[segment_count++] =
        StopSegment{active_track_min_primary, active_track_max_primary,
                    tokens.active_track, tokens.active_stop};
  }

  int16_t right_inactive_min =
      (int16_t)ceilf(end.layout.inactive_track_min_primary);
  if (right_inactive_min < axis.primarySpan()) {
    segments[segment_count++] =
        StopSegment{(float)std::max<int16_t>(0, right_inactive_min),
                    (float)(axis.primarySpan() - 1), tokens.inactive_track,
                    tokens.inactive_stop};
  }
}

}  // namespace

RangeSlider::RangeSlider(const Environment& env, SliderRange range,
                         float start_value, float end_value, SliderStyle style)
    : BasicWidget(env),
      range_(range),
      style_(style),
      start_value_(0.0f),
      end_value_(0.0f),
      min_separation_(0.0f),
      active_thumb_(kNoActiveThumb),
      overlay_thumb_(kNoActiveThumb),
      is_dragging_(false),
      awaiting_direction_(false),
      pending_content_dirty_span_(),
      pending_indicator_dirty_span_() {
  internal::CheckValidSliderRange(range_.from, range_.to, range_.step);
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
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  int16_t primary_coord = axis.primaryCoordFromPoint(x, y);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  active_thumb_ = ResolveNearestThumb(axis, start_pos, end_pos, primary_coord);
  overlay_thumb_ = active_thumb_;
  awaiting_direction_ = false;
  BasicWidget::onSingleTapUp(x, y);
  onInteractionStart(active_thumb_);
  if (setActiveThumbPos(axis.posFromPrimaryCoord(primary_coord))) {
    triggerInteractiveChange();
  }
  active_thumb_ = kNoActiveThumb;
  awaiting_direction_ = false;
  is_dragging_ = false;
  onInteractionEnd(start_value_, end_value_);
  return true;
}

void RangeSlider::onShowPress(XDim x, YDim y) {
  if (!isEnabled()) return;
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  bool awaiting_direction = false;
  int thumb = ResolveThumbForPress(axis, start_pos, end_pos,
                                   axis.primaryCoordFromPoint(x, y),
                                   awaiting_direction);
  if (awaiting_direction) {
    // Stacked thumbs need the first signed drag delta to decide whether the
    // user intends to move the lower or the upper bound.
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
  if (setActiveThumbPos(
          axis.posFromPrimaryCoord(axis.primaryCoordFromPoint(x, y)))) {
    triggerInteractiveChange();
  }
  Widget::onShowPress(x, y);
}

bool RangeSlider::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  if (!isEnabled()) return false;
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  if (!axis.shouldCaptureScroll(is_dragging_ || awaiting_direction_, dx, dy)) {
    return false;
  }

  if (awaiting_direction_) {
    // Once the thumbs are stacked, use drag direction rather than hit-testing
    // to decide which bound should peel away first.
    int16_t primary_delta = axis.primaryDelta(dx, dy);
    if (primary_delta < 0) {
      active_thumb_ = 0;
    } else if (primary_delta > 0) {
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
    active_thumb_ = ResolveNearestThumb(axis, start_pos, end_pos,
                                        axis.primaryCoordFromPoint(x, y));
    overlay_thumb_ = active_thumb_;
    is_dragging_ = true;
    onInteractionStart(active_thumb_);
  }

  bool changed = setActiveThumbPos(
      axis.posFromPrimaryCoord(axis.primaryCoordFromPoint(x, y)));
  if (changed) {
    setPressed(true);
    triggerInteractiveChange();
  }
  return true;
}

bool RangeSlider::onTouchUp(XDim x, YDim y) {
  (void)x;
  (void)y;
  if (active_thumb_ != kNoActiveThumb || is_dragging_ || awaiting_direction_) {
    bool had_active_thumb = active_thumb_ != kNoActiveThumb;
    if (isPressed()) {
      setPressed(false);
    }
    active_thumb_ = kNoActiveThumb;
    is_dragging_ = false;
    awaiting_direction_ = false;
    if (had_active_thumb) {
      onInteractionEnd(start_value_, end_value_);
    }
    return true;
  }
  return Widget::onTouchUp(x, y);
}

void RangeSlider::onCancel() {
  bool had_active_thumb = active_thumb_ != kNoActiveThumb;
  BasicWidget::onCancel();
  active_thumb_ = kNoActiveThumb;
  is_dragging_ = false;
  awaiting_direction_ = false;
  if (had_active_thumb) {
    onInteractionEnd(start_value_, end_value_);
  }
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
    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    invalidateValueChange(axis, old_start_pos, old_end_pos, new_start_pos,
                          new_end_pos, start_value_, end_value_);
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
    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    invalidateValueChange(axis, old_start_pos, old_end_pos, new_start_pos,
                          new_end_pos, start_value_, end_value_);
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

  if (width() > 0 && height() > 0) {
    uint16_t new_start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t new_end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    invalidateValueChange(axis, old_start_pos, old_end_pos, new_start_pos,
                          new_end_pos, start_value_, end_value_);
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

// Marks the minimal area that needs to be redrawn for a value change.
// Mirrors Slider::invalidatePosChange(): uses setDirty() rather than
// invalidateInterior() so the slider is not marked invalidated (paint()
// will only redraw the dirty slice), and notifies the parent only of the
// tight bubble envelope swept by the active thumb rather than the
// conservative full-width envelope reported by
// getParentTransientPaintBounds(). For range sliders the bubble only
// follows the active thumb, so only that thumb's old/new positions
// contribute to the bubble envelope.
void RangeSlider::invalidateValueChange(
    const internal::SliderAxisMetrics& axis, uint16_t old_start_pos,
    uint16_t old_end_pos, uint16_t new_start_pos, uint16_t new_end_pos,
    float new_start_value, float new_end_value) {
  Rect thumb_rect = InvalidationRectForValueChange(
      axis, old_start_pos, old_end_pos, new_start_pos, new_end_pos);
  Rect bubble_envelope(0, 0, -1, -1);
  if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
    uint16_t new_active_pos =
        (active_thumb_ == 1) ? new_end_pos : new_start_pos;
    float new_active_value =
        (active_thumb_ == 1) ? new_end_value : new_start_value;
    float c_new = axis.displayCenterFromPos(new_active_pos);
    char new_scratch[64];
    roo::string_view new_text =
        formatLabel(new_active_value, new_scratch, sizeof(new_scratch));
    int16_t new_bubble_width;
    int16_t new_bubble_height;
    ValueIndicatorBubble::MeasureBubbleSize(new_text, new_bubble_width,
                                            new_bubble_height);
    Rect new_indicator = ValueIndicatorBubble::EnvelopeForCenterRange(
        width(), height(), c_new, c_new, style_.value_indicator,
        style_.orientation, new_bubble_width, new_bubble_height);
    Rect old_indicator = IndicatorDirtyRectFromSpan(
        pending_indicator_dirty_span_, width(), height(), style_);
    bubble_envelope = Rect::Extent(old_indicator, new_indicator);
  }
  pending_content_dirty_span_.include(
      internal::DisplayMainSpanFromRect(thumb_rect, axis.isVertical()));
  pending_indicator_dirty_span_.include(
      internal::DisplayMainSpanFromRect(bubble_envelope, axis.isVertical()));
  setDirty(thumb_rect);
  if (!bubble_envelope.empty()) {
    notifyParentInvalidatedRegion(
        bubble_envelope.translate(offsetLeft(), offsetTop()));
  }
}

void RangeSlider::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetPressed) == 0) {
    Widget::notifyStateChanged(state_diff);
    return;
  }

  bool was_pressed = !isPressed();

  if (width() <= 0 || height() <= 0 || active_thumb_ == kNoActiveThumb) {
    setDirty();
    return;
  }

  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  float active_value = active_thumb_ == 1 ? end_value_ : start_value_;
  uint16_t pos =
      internal::SliderPosFromValue(range_.from, range_.to, active_value);
  Rect thumb_rect = axis.invalidationRectForPosChange(pos, pos);

  bool old_indicator_visible =
      style_.value_indicator == SliderValueIndicatorBehavior::kAlways ||
      was_pressed || isDragged();
  bool new_indicator_visible = ShowsValueIndicator(*this);

  Rect bubble_envelope(0, 0, -1, -1);
  if (old_indicator_visible != new_indicator_visible) {
    float center = axis.displayCenterFromPos(pos);
    bubble_envelope = ValueIndicatorBubble::EnvelopeForCenterRange(
        width(), height(), center, center, style_.value_indicator,
        style_.orientation);
  }

  pending_content_dirty_span_.include(
      internal::DisplayMainSpanFromRect(thumb_rect, axis.isVertical()));
  pending_indicator_dirty_span_.include(
      internal::DisplayMainSpanFromRect(bubble_envelope, axis.isVertical()));

  setDirty(bubble_envelope.empty() ? thumb_rect
                                   : Rect::Extent(thumb_rect, bubble_envelope));
  if (!bubble_envelope.empty()) {
    notifyParentInvalidatedRegion(
        bubble_envelope.translate(offsetLeft(), offsetTop()));
  }
}

void RangeSlider::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  ThumbPaintMetrics start =
      ResolveThumbPaintMetrics(range_, start_value_, axis, size_metrics,
                               isPressed() && active_thumb_ == 0);
  ThumbPaintMetrics end =
      ResolveThumbPaintMetrics(range_, end_value_, axis, size_metrics,
                               isPressed() && active_thumb_ == 1);
  StopSegment segments[3];
  int segment_count = 0;
  BuildTrackSegments(tokens, axis, start, end, segments, segment_count);

  auto track_bounds = axis.paintRectFromPrimaryCross(
      TrackShapeMinPrimary(0.0f, size_metrics.track_radius),
      start.layout.track_min_cross, axis.primarySpan() - 0.5f,
      start.layout.track_max_cross);
  auto start_handle_bounds = axis.paintRectFromPrimaryCross(
      start.layout.thumb_min_primary, start.layout.thumb_min_cross,
      start.layout.thumb_max_primary, start.layout.thumb_max_cross);
  auto end_handle_bounds = axis.paintRectFromPrimaryCross(
      end.layout.thumb_min_primary, end.layout.thumb_min_cross,
      end.layout.thumb_max_primary, end.layout.thumb_max_cross);

  auto inactive_track = SmoothFilledRoundRect(
      track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
      track_bounds.y_max, size_metrics.track_radius, tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
      track_bounds.y_max, size_metrics.track_radius, tokens.active_track);
  auto start_handle = SmoothFilledRoundRect(
      start_handle_bounds.x_min, start_handle_bounds.y_min,
      start_handle_bounds.x_max, start_handle_bounds.y_max,
      size_metrics.handleCornerRadius(), tokens.handle);
  auto end_handle =
      SmoothFilledRoundRect(end_handle_bounds.x_min, end_handle_bounds.y_min,
                            end_handle_bounds.x_max, end_handle_bounds.y_max,
                            size_metrics.handleCornerRadius(), tokens.handle);
  Rect widget_bounds = bounds();
  Rect start_handle_tile_bounds =
      GetHandleTileBounds(widget_bounds, start_handle.extents(),
                          start.track_gap, axis.isVertical());
  Rect end_handle_tile_bounds = GetHandleTileBounds(
      widget_bounds, end_handle.extents(), end.track_gap, axis.isVertical());

  // Paint the same segment model used by paintStops() so the track and stop
  // passes agree on where each active or inactive run begins and ends.
  for (int i = 0; i < segment_count; ++i) {
    roo_display::Box track_clip =
        TrackSegmentClipBox(axis, segments[i], start.layout.track_cross_start,
                            size_metrics.track_height);
    if (track_clip.empty()) continue;
    const auto& track_piece = segments[i].track_color == tokens.active_track
                                  ? active_track
                                  : inactive_track;
    DrawTrackPiece(canvas, track_piece, widget_bounds, track_clip,
                   axis.isVertical());
  }
  canvas.drawTiled(start_handle, start_handle_tile_bounds, kNoAlign);
  canvas.drawTiled(end_handle, end_handle_tile_bounds, kNoAlign);
}

void RangeSlider::paintStops(const Canvas& canvas, Clipper& clipper) const {
  if (!ShouldRenderStops(*this) || width() <= 0 || height() <= 0) return;

  Tokens tokens = ResolveTokens(*this);
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  ThumbPaintMetrics start =
      ResolveThumbPaintMetrics(range_, start_value_, axis, size_metrics,
                               isPressed() && active_thumb_ == 0);
  ThumbPaintMetrics end =
      ResolveThumbPaintMetrics(range_, end_value_, axis, size_metrics,
                               isPressed() && active_thumb_ == 1);

  StopSegment segments[3];
  int segment_count = 0;
  BuildTrackSegments(tokens, axis, start, end, segments, segment_count);

  if (segment_count == 0) return;

  PaintStopRuns(canvas, clipper, axis, segments, segment_count, range_,
                nullptr);
}

Dimensions RangeSlider::getSuggestedMinimumDimensions() const {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  return Dimensions(size_metrics.handle_height, size_metrics.handle_height);
}

PreferredSize RangeSlider::getPreferredSize() const {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  if (style_.orientation == SliderOrientation::kVertical) {
    return PreferredSize(PreferredSize::ExactWidth(size_metrics.handle_height),
                         PreferredSize::MatchParentHeight());
  }
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::ExactHeight(size_metrics.handle_height));
}

roo_display::FpPoint RangeSlider::getPointOverlayFocus() const {
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  uint16_t start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);
  int thumb = active_thumb_;
  if (thumb == kNoActiveThumb) {
    thumb = overlay_thumb_;
  }
  if (style_.orientation == SliderOrientation::kVertical) {
    return roo_display::FpPoint{
        axis.centeredCross(),
        axis.displayCenterFromPos(thumb == 1 ? end_pos : start_pos)};
  }
  return roo_display::FpPoint{
      axis.displayCenterFromPos(thumb == 1 ? end_pos : start_pos),
      axis.centeredCross()};
}

ColorRole RangeSlider::effectiveContainerRole() const {
  return ColorRole::kPrimary;
}

Rect RangeSlider::getSloppyTouchParentBounds() const {
  Rect bounds = Widget::getSloppyTouchParentBounds();
  if (style_.orientation == SliderOrientation::kVertical) {
    return Rect(bounds.xMin(), bounds.yMin() - kTouchSlopPixels, bounds.xMax(),
                bounds.yMax() + kTouchSlopPixels);
  }
  return Rect(bounds.xMin() - kTouchSlopPixels, bounds.yMin(),
              bounds.xMax() + kTouchSlopPixels, bounds.yMax());
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
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  Rect bubble_local = ValueIndicatorBubble::ConservativeBounds(
      width(), height(), size_metrics.handle_width, style_.value_indicator,
      style_.orientation);
  if (bubble_local.empty()) return base;
  Rect bubble_parent = bubble_local.translate(offsetLeft(), offsetTop());
  return Rect::Extent(base, bubble_parent);
}

void RangeSlider::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  // See Slider::paintWidgetContents() for the rationale on ordering and
  // on narrowing the canvas clip when the repaint is driven by a
  // value/style state change (rather than a forced full invalidation).
  internal::DirtySpan pending_content_span = pending_content_dirty_span_;
  internal::DirtySpan pending_indicator_span = pending_indicator_dirty_span_;
  pending_content_dirty_span_ = internal::DirtySpan();

  Rect pending_content = internal::ContentRectFromDisplayMainSpan(
      pending_content_span, width(), height(),
      style_.orientation == SliderOrientation::kVertical);
  Rect pending_indicator = IndicatorDirtyRectFromSpan(
      pending_indicator_span, width(), height(), style_);
  internal::DirtySpan current_indicator_span = pending_indicator_span;
  // Pre-paint the active thumb's value indicator bubble before the slider
  // contents. See Slider::paintWidgetContents() for the rationale on
  // ordering and the role of paint() vs decorate(). The canvas is already
  // clipped to maxBounds(), which (via getVisualBounds() ->
  // getTransientPaintBounds() -> our getParentTransientPaintBounds()
  // override) already covers the bubble's conservative envelope when the
  // indicator is showing.
  auto paint_indicator = [&](const Canvas& indicator_canvas) {
    if (!ShowsValueIndicator(*this) || width() <= 0 || height() <= 0) return;
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

    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    uint16_t pos =
        internal::SliderPosFromValue(range_.from, range_.to, thumb_value);
    float thumb_center = axis.displayCenterFromPos(pos);
    bool clamp =
        style_.value_indicator == SliderValueIndicatorBehavior::kWithinBounds;

    ValueIndicatorBubble bubble;
    if (bubble.layout(width(), height(), thumb_center, style_.orientation, text,
                      clamp, bubble_color, text_color)) {
      current_indicator_span = internal::DisplayMainSpanFromRect(
          bubble.bounds(), style_.orientation == SliderOrientation::kVertical);
      bubble.paint(indicator_canvas);
      bubble.decorate(indicator_canvas, clipper, OverlaySpec());
    }
  };

  if (!isInvalidated()) {
    if (!pending_indicator.empty()) {
      Canvas indicator_canvas = canvas;
      indicator_canvas.clipToExtents(pending_indicator);
      if (!indicator_canvas.clip_box().empty()) {
        paint_indicator(indicator_canvas);
      }
    }

    Canvas content_canvas = prepareContentsCanvas(canvas);
    if (!pending_content.empty()) {
      content_canvas.clipToExtents(pending_content);
    }
    if (!content_canvas.clip_box().empty()) {
      clipper.setBounds(content_canvas.clip_box());
      paintStops(content_canvas, clipper);
      paint(content_canvas);
    }
    pending_indicator_dirty_span_ = ShowsValueIndicator(*this)
                                        ? current_indicator_span
                                        : internal::DirtySpan();
    markClean();
    return;
  }

  Canvas my_canvas = canvas;
  paint_indicator(my_canvas);
  Canvas content_canvas = prepareContentsCanvas(my_canvas);
  if (!content_canvas.clip_box().empty()) {
    clipper.setBounds(content_canvas.clip_box());
    paintStops(content_canvas, clipper);
    paint(content_canvas);
  }
  pending_indicator_dirty_span_ = ShowsValueIndicator(*this)
                                      ? current_indicator_span
                                      : internal::DirtySpan();
  markClean();
}

}  // namespace material3
}  // namespace roo_windows
