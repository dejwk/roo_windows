#include "roo_windows/material3/slider/range_slider.h"

#include <algorithm>
#include <cmath>

#include "roo_display/shape/smooth.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/material3/slider/slider_internal.h"
#include "roo_windows/material3/slider/slider_size_internal.h"
#include "roo_windows/material3/slider/value_indicator.h"

using namespace roo_display;

namespace roo_windows {
namespace material3 {

namespace {

static constexpr int kTouchSlopPixels = Scaled(20);
static constexpr int8_t kNoActiveThumb = -1;
static constexpr float kPressedThumbWidthRatio = 0.5f;
static constexpr int16_t kStopMarkRadiusPixels = Scaled(2);
static constexpr int16_t kStopMarkSpanPixels = Scaled(4);

Color DisabledComposite(Color fg, uint8_t alpha, const Theme& theme) {
  return AlphaBlend(theme.color.surface, fg.withA(alpha));
}

struct Tokens {
  Color active_track;
  Color inactive_track;
  Color handle;
  Color active_stop;
  Color inactive_stop;
};

Tokens ResolveTokens(const RangeSlider& widget) {
  const Theme& theme = widget.theme();
  if (!widget.isEnabled()) {
    return Tokens{
        DisabledComposite(theme.color.onSurface, 0x61, theme),
        DisabledComposite(theme.color.onSurface, 0x1F, theme),
        DisabledComposite(theme.color.onSurface, 0x61, theme),
        theme.color.surface,
        theme.color.surface,
    };
  }

  return Tokens{
      theme.color.primary,
      theme.color.secondaryContainer,
      theme.color.primary,
      theme.color.onPrimary,
      theme.color.onSecondaryContainer,
  };
}

bool ShouldRenderStops(const RangeSlider& widget) {
  if (!internal::IsDiscreteSliderRange(widget.range().step)) return false;
  return widget.style().tick_mode != SliderTickMode::kHidden;
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
  normalized_start = internal::NormalizeSliderValueForRange(
      start_value, range.from, range.to, range.step);
  normalized_end = internal::NormalizeSliderValueForRange(end_value, range.from,
                                                          range.to, range.step);
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

float TrackShapeMinPrimary(float visible_min_primary, int16_t track_radius) {
  return visible_min_primary <= 0.0f
             ? -0.5f
             : visible_min_primary - (float)track_radius;
}

int16_t ThumbWidthForState(int16_t nominal_width, bool pressed) {
  if (!pressed) return nominal_width;
  int16_t width =
      (int16_t)roundf((float)nominal_width * kPressedThumbWidthRatio);
  return width < 1 ? 1 : width;
}

int16_t TrackGapForThumbWidth(const internal::SliderSizeMetrics& size_metrics,
                              int16_t thumb_width) {
  int16_t gap = size_metrics.track_handle_gap -
                (size_metrics.handle_width - thumb_width) / 2;
  return gap < 0 ? 0 : gap;
}

Rect GetTrackTileBounds(const Rect& widget_bounds, const roo_display::Box& clip,
                        bool vertical) {
  if (vertical) {
    return Rect(widget_bounds.xMin(), clip.yMin(), widget_bounds.xMax(),
                clip.yMax());
  }
  return Rect(clip.xMin(), widget_bounds.yMin(), clip.xMax(),
              widget_bounds.yMax());
}

Rect GetHandleTileBounds(const Rect& widget_bounds,
                         const roo_display::Box& handle_bounds,
                         int16_t track_gap, bool vertical) {
  Rect expanded =
      vertical ? Rect(widget_bounds.xMin(), handle_bounds.yMin() - track_gap,
                      widget_bounds.xMax(), handle_bounds.yMax() + track_gap)
               : Rect(handle_bounds.xMin() - track_gap, widget_bounds.yMin(),
                      handle_bounds.xMax() + track_gap, widget_bounds.yMax());
  return Rect::Intersect(expanded, widget_bounds);
}

void DrawTrackPiece(const Canvas& canvas, const roo_display::Drawable& piece,
                    const Rect& widget_bounds,
                    const roo_display::Box& clip_bounds, bool vertical) {
  if (clip_bounds.empty()) return;
  Rect tile_bounds = GetTrackTileBounds(widget_bounds, clip_bounds, vertical);
  canvas.drawTiled(piece, tile_bounds, kNoAlign);
}

internal::SliderAxisMetrics MakeSliderAxisMetrics(const RangeSlider& slider) {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(slider.style().size);
  return internal::SliderAxisMetrics(
      slider.width(), slider.height(), size_metrics.handle_width,
      size_metrics.track_handle_gap,
      slider.style().orientation == SliderOrientation::kVertical);
}

Rect IndicatorDirtyRectFromSpan(const internal::DirtySpan& span, int16_t width,
                                int16_t height, SliderStyle style) {
  if (span.empty() || width <= 0 || height <= 0) return Rect(0, 0, -1, -1);
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style.size);
  Rect conservative = ValueIndicatorBubble::ConservativeBounds(
      width, height, size_metrics.handle_width, style.value_indicator,
      style.orientation);
  if (conservative.empty()) return Rect(0, 0, -1, -1);
  if (style.orientation == SliderOrientation::kVertical) {
    return Rect(conservative.xMin(), span.min_coord, conservative.xMax(),
                span.max_coord);
  }
  return Rect(span.min_coord, conservative.yMin(), span.max_coord,
              conservative.yMax());
}

struct StopSegment {
  float min_primary;
  float max_primary;
  Color track_color;
  Color stop_color;
};

struct StopRun {
  bool has_marks = false;
  int16_t min_primary = 0;
  int16_t max_primary = -1;
};

struct StopSpan {
  int16_t min_primary;
  int16_t max_primary;
};

struct ThumbPaintMetrics {
  uint16_t pos;
  int16_t track_gap;
  internal::SliderVisualMetrics layout;
};

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

roo_display::Box StopSegmentClipBox(const internal::SliderAxisMetrics& axis,
                                    const StopSegment& segment) {
  int16_t min_primary = (int16_t)ceilf(segment.min_primary);
  int16_t max_primary = (int16_t)floorf(segment.max_primary);
  if (min_primary < 0) min_primary = 0;
  if (max_primary >= axis.primarySpan()) max_primary = axis.primarySpan() - 1;
  if (max_primary < min_primary) return roo_display::Box(0, 0, -1, -1);
  int16_t cross_start = (axis.crossSpan() - kStopMarkSpanPixels) / 2;
  return axis.boxFromPrimaryCross(min_primary, cross_start, max_primary,
                                  cross_start + kStopMarkSpanPixels - 1);
}

inline bool SegmentContains(const StopSegment& segment, float primary) {
  return primary >= segment.min_primary - 0.01f &&
         primary <= segment.max_primary + 0.01f;
}

roo_display::FpPoint StopMarkCenter(const internal::SliderAxisMetrics& axis,
                                    float primary_center) {
  float cross_center = 0.5f * (float)axis.crossSpan() - 0.5f;
  if (axis.isVertical()) {
    return roo_display::FpPoint{cross_center,
                                axis.displayPrimary(primary_center)};
  }
  return roo_display::FpPoint{primary_center, cross_center};
}

void IncludeStopExtentsInRun(const internal::SliderAxisMetrics& axis,
                             const roo_display::Box& stop_extents,
                             StopRun& run) {
  StopSpan span =
      axis.isVertical()
          ? StopSpan{(int16_t)(axis.primarySpan() - 1 - stop_extents.yMax()),
                     (int16_t)(axis.primarySpan() - 1 - stop_extents.yMin())}
          : StopSpan{stop_extents.xMin(), stop_extents.xMax()};
  if (!run.has_marks) {
    run.has_marks = true;
    run.min_primary = span.min_primary;
    run.max_primary = span.max_primary;
    return;
  }
  if (span.min_primary < run.min_primary) run.min_primary = span.min_primary;
  if (span.max_primary > run.max_primary) run.max_primary = span.max_primary;
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

  int16_t active_clip_min =
      (int16_t)ceilf(start.layout.inactive_track_min_primary);
  int16_t active_clip_max =
      (int16_t)floorf(end.layout.active_track_max_primary);
  if (active_clip_min < 0) active_clip_min = 0;
  if (active_clip_max >= axis.primarySpan()) {
    active_clip_max = axis.primarySpan() - 1;
  }

  roo_display::Box active_clip = axis.boxFromPrimaryCross(
      active_clip_min, start.layout.track_cross_start, active_clip_max,
      start.layout.track_cross_start + size_metrics.track_height - 1);
  bool has_left_inactive_clip = true;
  int16_t left_inactive_max =
      (int16_t)floorf(start.layout.active_track_max_primary);
  roo_display::Box left_inactive_clip;
  if (left_inactive_max < 0) {
    has_left_inactive_clip = false;
  } else {
    if (left_inactive_max >= axis.primarySpan()) {
      left_inactive_max = axis.primarySpan() - 1;
    }
    left_inactive_clip = axis.boxFromPrimaryCross(
        0, start.layout.track_cross_start, left_inactive_max,
        start.layout.track_cross_start + size_metrics.track_height - 1);
  }
  bool has_right_inactive_clip = true;
  int16_t right_inactive_min =
      (int16_t)ceilf(end.layout.inactive_track_min_primary);
  roo_display::Box right_inactive_clip;
  if (right_inactive_min >= axis.primarySpan()) {
    has_right_inactive_clip = false;
  } else {
    if (right_inactive_min < 0) right_inactive_min = 0;
    right_inactive_clip = axis.boxFromPrimaryCross(
        right_inactive_min, start.layout.track_cross_start,
        axis.primarySpan() - 1,
        start.layout.track_cross_start + size_metrics.track_height - 1);
  }

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

  if (has_left_inactive_clip) {
    DrawTrackPiece(canvas, inactive_track, widget_bounds, left_inactive_clip,
                   axis.isVertical());
  }
  if (has_right_inactive_clip) {
    DrawTrackPiece(canvas, inactive_track, widget_bounds, right_inactive_clip,
                   axis.isVertical());
  }
  if (!active_clip.empty()) {
    DrawTrackPiece(canvas, active_track, widget_bounds, active_clip,
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

  if (segment_count == 0) return;

  StopRun runs[3];
  int32_t stop_count =
      (int32_t)lroundf((range_.to - range_.from) / range_.step);
  for (int32_t i = 0; i <= stop_count; ++i) {
    float value = (i == stop_count) ? range_.to : range_.from + i * range_.step;
    uint16_t pos = internal::SliderPosFromValue(range_.from, range_.to, value);
    if (pos == start.pos || pos == end.pos) continue;
    float primary_center = axis.centerFromPos(pos);
    for (int segment_index = 0; segment_index < segment_count;
         ++segment_index) {
      if (!SegmentContains(segments[segment_index], primary_center)) continue;
      auto stop = SmoothFilledCircle(StopMarkCenter(axis, primary_center),
                                     kStopMarkRadiusPixels,
                                     segments[segment_index].stop_color);
      IncludeStopExtentsInRun(axis, stop.extents(), runs[segment_index]);
      break;
    }
  }

  for (int segment_index = 0; segment_index < segment_count; ++segment_index) {
    if (!runs[segment_index].has_marks) continue;
    roo_display::Box segment_clip_box =
        StopSegmentClipBox(axis, segments[segment_index]);
    if (segment_clip_box.empty()) continue;
    int16_t cross_start = (axis.crossSpan() - kStopMarkSpanPixels) / 2;
    roo_display::Box run_box = axis.boxFromPrimaryCross(
        runs[segment_index].min_primary, cross_start,
        runs[segment_index].max_primary, cross_start + kStopMarkSpanPixels - 1);
    run_box = roo_display::Box::Intersect(run_box, segment_clip_box);
    if (run_box.empty()) continue;

    Canvas stop_canvas = canvas;
    stop_canvas.set_bgcolor(segments[segment_index].track_color);
    stop_canvas.clip(segment_clip_box.translate(canvas.dx(), canvas.dy()));
    bool has_previous_stop = false;
    StopSpan previous_span{0, -1};
    for (int32_t i = 0; i <= stop_count; ++i) {
      float value =
          (i == stop_count) ? range_.to : range_.from + i * range_.step;
      uint16_t pos =
          internal::SliderPosFromValue(range_.from, range_.to, value);
      if (pos == start.pos || pos == end.pos) continue;
      float primary_center = axis.centerFromPos(pos);
      if (!SegmentContains(segments[segment_index], primary_center)) continue;
      auto stop = SmoothFilledCircle(StopMarkCenter(axis, primary_center),
                                     kStopMarkRadiusPixels,
                                     segments[segment_index].stop_color);
      StopSpan current_span =
          axis.isVertical()
              ? StopSpan{(int16_t)(axis.primarySpan() - 1 -
                                   stop.extents().yMax()),
                         (int16_t)(axis.primarySpan() - 1 -
                                   stop.extents().yMin())}
              : StopSpan{stop.extents().xMin(), stop.extents().xMax()};
      if (has_previous_stop) {
        int16_t gap_min_primary = previous_span.max_primary + 1;
        int16_t gap_max_primary = current_span.min_primary - 1;
        if (gap_max_primary >= gap_min_primary) {
          stop_canvas.fillRect(
              axis.boxFromPrimaryCross(gap_min_primary, cross_start,
                                       gap_max_primary,
                                       cross_start + kStopMarkSpanPixels - 1),
              segments[segment_index].track_color);
        }
      }
      stop_canvas.drawObject(stop);
      previous_span = current_span;
      has_previous_stop = true;
    }

    roo_display::Box run_device_box = roo_display::Box::Intersect(
        run_box.translate(canvas.dx(), canvas.dy()), canvas.clip_box());
    if (!run_device_box.empty()) {
      clipper.addExclusion(run_device_box);
    }
  }
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
