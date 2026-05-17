#include "roo_windows/material3/slider/slider.h"

#include <algorithm>
#include <cmath>

#include "roo_display/shape/smooth.h"
#include "roo_logging.h"
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
static constexpr int16_t kCenterGapHalfPixels = Scaled(2);
static constexpr int16_t kInsetIconEdgeOffsetPixels = Scaled(4);
static constexpr int16_t kInsetIconMinHandleCenterDistancePixels = Scaled(12);
static constexpr int16_t kInsetIconStopPaddingPixels = Scaled(4);
using Tokens = internal::SliderPaintTokens;
using StopSegment = internal::SliderPaintStopSegment;
using internal::DrawTrackPiece;
using internal::GetHandleTileBounds;
using internal::IndicatorDirtyRectFromSpan;
using internal::kStopMarkRadiusPixels;
using internal::MakeSliderAxisMetrics;
using internal::PaintStopRuns;
using internal::ResolveTokens;
using internal::SegmentContainsRange;
using internal::ShouldRenderStops;
using internal::ThumbWidthForState;
using internal::TrackGapForThumbWidth;
using internal::TrackSegmentClipBox;
using internal::TrackShapeMinPrimary;

// True iff the indicator should be drawn this frame given the current
// interaction state and behavior. kAlways shows the bubble at all times;
// the on-interaction policies show it only while pressed or dragged.
bool ShowsValueIndicator(const Slider& widget) {
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

// Splits the track into active/inactive runs. Centered sliders keep a small
// empty gap around logical zero so the two halves do not visually merge.
void BuildTrackSegments(const Slider& slider,
                        const internal::SliderAxisMetrics& axis,
                        const internal::SliderVisualMetrics& layout,
                        StopSegment* segments, int& segment_count) {
  segment_count = 0;
  if (slider.variant() != SliderVariant::kCentered) {
    if (layout.active_track_max_primary >= 0.0f) {
      segments[segment_count++] =
          StopSegment{0.0f, layout.active_track_max_primary, true};
    }
    if (layout.inactive_track_min_primary < axis.primarySpan()) {
      segments[segment_count++] =
          StopSegment{layout.inactive_track_min_primary,
                      (float)(axis.primarySpan() - 1), false};
    }
    return;
  }

  const SliderRange& range = slider.range();
  float center_anchor_primary =
      axis.centerFromValue(range, 0.5f * (range.from + range.to));
  bool thumb_on_or_right_of_center =
      axis.centerFromValue(range, slider.value()) >= center_anchor_primary;
  float center_left_edge = center_anchor_primary - (float)kCenterGapHalfPixels;
  float center_right_edge = center_anchor_primary + (float)kCenterGapHalfPixels;
  int16_t handle_left_edge = (int16_t)floorf(layout.active_track_max_primary);
  int16_t center_left_edge_i = (int16_t)floorf(center_left_edge);
  int16_t left_inactive_max =
      thumb_on_or_right_of_center
          ? std::min(handle_left_edge, center_left_edge_i)
          : handle_left_edge;
  if (left_inactive_max >= 0) {
    segments[segment_count++] = StopSegment{
        0.0f,
        (float)std::min<int16_t>(left_inactive_max, axis.primarySpan() - 1),
        false};
  }

  float active_track_min_primary = thumb_on_or_right_of_center
                                       ? center_right_edge
                                       : layout.inactive_track_min_primary;
  float active_track_max_primary = thumb_on_or_right_of_center
                                       ? layout.active_track_max_primary
                                       : center_left_edge;
  if (active_track_max_primary >= active_track_min_primary) {
    segments[segment_count++] =
        StopSegment{active_track_min_primary, active_track_max_primary, true};
  }

  int16_t handle_right_edge = (int16_t)ceilf(layout.inactive_track_min_primary);
  int16_t center_right_edge_i = (int16_t)ceilf(center_right_edge);
  int16_t right_inactive_min =
      thumb_on_or_right_of_center
          ? handle_right_edge
          : std::max(handle_right_edge, center_right_edge_i);
  if (right_inactive_min < axis.primarySpan()) {
    segments[segment_count++] =
        StopSegment{(float)std::max<int16_t>(0, right_inactive_min),
                    (float)(axis.primarySpan() - 1), false};
  }
}

// Finds the track segment that fully covers a prospective icon or stop range.
const StopSegment* FindSegmentContainingRange(const StopSegment* segments,
                                              int segment_count,
                                              float min_primary,
                                              float max_primary) {
  for (int i = 0; i < segment_count; ++i) {
    if (SegmentContainsRange(segments[i], min_primary, max_primary)) {
      return &segments[i];
    }
  }
  return nullptr;
}

// Chooses an inset-icon slot that stays near the requested edge, clears the
// thumb center, and remains fully inside a single track segment.
bool ResolveInsetIconBounds(const internal::SliderAxisMetrics& axis,
                            const internal::SliderVisualMetrics& layout,
                            const internal::SliderSizeMetrics& size_metrics,
                            const roo_display::Pictogram& icon, bool at_start,
                            const StopSegment* segments, int segment_count,
                            roo_display::Box& icon_bounds) {
  bool vertical = axis.isVertical();
  int16_t max_slot_span = size_metrics.icon_size > 0
                              ? size_metrics.icon_size
                              : size_metrics.track_height;
  if (max_slot_span <= 0) return false;

  roo_display::Box extents = icon.anchorExtents();
  int16_t icon_primary_span = vertical ? extents.height() : extents.width();
  int16_t icon_cross_span = vertical ? extents.width() : extents.height();
  if (icon_primary_span <= 0 || icon_cross_span <= 0 ||
      icon_primary_span > max_slot_span || icon_cross_span > max_slot_span ||
      icon_cross_span > size_metrics.track_height ||
      icon_primary_span > axis.primarySpan()) {
    return false;
  }

  int16_t cross_start = layout.track_cross_start +
                        (size_metrics.track_height - icon_cross_span) / 2;
  if (cross_start < 0 || cross_start + icon_cross_span > axis.crossSpan()) {
    return false;
  }

  auto try_candidate = [&](int16_t min_primary) -> bool {
    int16_t max_primary = min_primary + icon_primary_span - 1;
    if (min_primary < 0 || max_primary >= axis.primarySpan()) return false;
    int16_t max_left_boundary =
        (int16_t)floorf(layout.thumb_center_primary -
                        (float)kInsetIconMinHandleCenterDistancePixels);
    int16_t min_right_boundary =
        (int16_t)ceilf(layout.thumb_center_primary +
                       (float)kInsetIconMinHandleCenterDistancePixels);
    if (max_primary > max_left_boundary && min_primary < min_right_boundary) {
      return false;
    }
    if (FindSegmentContainingRange(segments, segment_count, (float)min_primary,
                                   (float)max_primary) == nullptr) {
      return false;
    }
    icon_bounds =
        axis.boxFromPrimaryCross(min_primary, cross_start, max_primary,
                                 cross_start + icon_cross_span - 1);
    return true;
  };

  int16_t edge_candidate =
      at_start
          ? kInsetIconEdgeOffsetPixels
          : axis.primarySpan() - kInsetIconEdgeOffsetPixels - icon_primary_span;
  if (try_candidate(edge_candidate)) return true;

  int16_t jump_candidate =
      at_start
          ? std::max<int16_t>(
                (int16_t)ceilf(layout.inactive_track_min_primary),
                (int16_t)ceilf(layout.thumb_center_primary +
                               (float)kInsetIconMinHandleCenterDistancePixels))
          : std::min<int16_t>(
                (int16_t)floorf(layout.active_track_max_primary) -
                    icon_primary_span + 1,
                (int16_t)floorf(
                    layout.thumb_center_primary -
                    (float)kInsetIconMinHandleCenterDistancePixels) -
                    icon_primary_span + 1);
  return try_candidate(jump_candidate);
}

// Grows the painted icon bounds along the track axis to reserve breathing room
// for nearby stops and dirty-region recomputation.
Rect ExpandInsetIconPrimaryRect(const Rect& icon_rect, bool vertical,
                                int16_t primary_span, int16_t primary_padding) {
  if (icon_rect.empty()) return icon_rect;
  if (vertical) {
    return Rect(
        icon_rect.xMin(), std::max<YDim>(0, icon_rect.yMin() - primary_padding),
        icon_rect.xMax(),
        std::min<YDim>(primary_span - 1, icon_rect.yMax() + primary_padding));
  }
  return Rect(
      std::max<XDim>(0, icon_rect.xMin() - primary_padding), icon_rect.yMin(),
      std::min<XDim>(primary_span - 1, icon_rect.xMax() + primary_padding),
      icon_rect.yMax());
}

}  // namespace

struct Slider::Metrics {
  internal::SliderSizeMetrics size_metrics;
  internal::SliderAxisMetrics axis;
  int16_t thumb_width;
  int16_t track_gap;
  internal::SliderVisualMetrics layout;
  StopSegment segments[3];
  int segment_count;
};

Slider::Metrics Slider::buildMetrics() const {
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  return buildMetrics(axis.centerFromValue(range_, value_), isPressed());
}

Slider::Metrics Slider::buildMetrics(float thumb_center, bool pressed) const {
  Metrics metrics{
      .size_metrics = internal::ResolveSliderSizeMetrics(style_.size),
      .axis = MakeSliderAxisMetrics(*this),
      .thumb_width = 0,
      .track_gap = 0,
      .layout = internal::SliderVisualMetrics(),
      .segments = {},
      .segment_count = 0,
  };
  metrics.thumb_width = ThumbWidthForState(pressed);
  metrics.track_gap = TrackGapForThumbWidth(metrics.thumb_width);
  metrics.layout = internal::ResolveSliderVisualMetrics(
      metrics.axis, thumb_center, metrics.thumb_width,
      metrics.size_metrics.track_height, metrics.track_gap,
      metrics.size_metrics.handle_height);
  BuildTrackSegments(*this, metrics.axis, metrics.layout, metrics.segments,
                     metrics.segment_count);
  return metrics;
}

Rect Slider::insetIconRect(const Metrics& metrics) const {
  InsetIcon inset_icon = getInsetIcon();
  if (inset_icon.icon == nullptr || metrics.segment_count == 0) {
    return Rect(0, 0, -1, -1);
  }

  roo_display::Box inset_bounds;
  bool inset_at_start = inset_icon.anchor == SliderInsetIconAnchor::kStart;
  if (variant_ != SliderVariant::kStandard ||
      !metrics.size_metrics.supports_inset_icon ||
      metrics.size_metrics.icon_size <= 0 ||
      !ResolveInsetIconBounds(metrics.axis, metrics.layout,
                              metrics.size_metrics, *inset_icon.icon,
                              inset_at_start, metrics.segments,
                              metrics.segment_count, inset_bounds)) {
    return Rect(0, 0, -1, -1);
  }
  return Rect(inset_bounds);
}

Rect Slider::insetIconReservedRect(const Metrics& metrics) const {
  return ExpandInsetIconPrimaryRect(
      insetIconRect(metrics), metrics.axis.isVertical(),
      metrics.axis.primarySpan(), kInsetIconStopPaddingPixels);
}

Rect Slider::insetIconDirtyRect(const Metrics& metrics) const {
  return ExpandInsetIconPrimaryRect(
      insetIconRect(metrics), metrics.axis.isVertical(),
      metrics.axis.primarySpan(),
      kInsetIconStopPaddingPixels + kStopMarkRadiusPixels);
}

void Slider::paintInsetIcons(const Canvas& canvas, Clipper& clipper,
                             const Metrics& metrics,
                             const Tokens& tokens) const {
  Rect inset_rect = insetIconRect(metrics);
  if (inset_rect.empty()) return;

  InsetIcon inset_icon = getInsetIcon();

  auto paint_icon = [&](const roo_display::Pictogram& icon,
                        const roo_display::Box& icon_bounds,
                        const StopSegment& segment) {
    Canvas icon_canvas = canvas;
    icon_canvas.set_bgcolor(TrackColorForSegment(tokens, segment));
    icon_canvas.clip(icon_bounds.translate(canvas.dx(), canvas.dy()));
    if (icon_canvas.clip_box().empty()) return;
    roo_display::Pictogram tinted_icon(icon);
    tinted_icon.color_mode().setColor(StopColorForSegment(tokens, segment));
    icon_canvas.drawTiled(tinted_icon, Rect(icon_bounds), kCenter | kMiddle,
                          false);
    roo_display::Box device_box = roo_display::Box::Intersect(
        icon_bounds.translate(canvas.dx(), canvas.dy()), canvas.clip_box());
    if (!device_box.empty()) {
      clipper.addExclusion(device_box);
    }
  };

  float min_primary =
      metrics.axis.isVertical()
          ? (float)(metrics.axis.primarySpan() - 1 - inset_rect.yMax())
          : (float)inset_rect.xMin();
  float max_primary =
      metrics.axis.isVertical()
          ? (float)(metrics.axis.primarySpan() - 1 - inset_rect.yMin())
          : (float)inset_rect.xMax();
  const StopSegment* inset_segment = FindSegmentContainingRange(
      metrics.segments, metrics.segment_count, min_primary, max_primary);
  if (inset_segment != nullptr) {
    paint_icon(*inset_icon.icon, inset_rect.asBox(), *inset_segment);
  }
}

Slider::Slider(const Environment& env, SliderRange range, float value,
               SliderVariant variant, SliderStyle style)
    : BasicWidget(env),
      range_(range),
      variant_(variant),
      style_(style),
      value_(0.0f),
      is_dragging_(false),
      pending_content_dirty_span_(),
      pending_indicator_dirty_span_() {
  internal::CheckValidSliderRange(range_.from, range_.to, range_.step);
  value_ = internal::NormalizeSliderValueForRange(value, range_.from, range_.to,
                                                  range_.step);
  if (IndicatorEnabled(style_)) {
    setParentClipMode(ParentClipMode::kUnclipped);
  }
}

bool Slider::onDown(XDim x, YDim y) {
  (void)x;
  (void)y;
  return isEnabled();
}

bool Slider::onSingleTapUp(XDim x, YDim y) {
  if (!isEnabled()) return false;
  BasicWidget::onSingleTapUp(x, y);
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  float value =
      axis.valueFromPrimaryCoord(range_, axis.primaryCoordFromPoint(x, y));
  onInteractionStart();
  if (setValueInternal(value, true)) {
    triggerInteractiveChange();
  }
  onInteractionEnd(value_);
  return true;
}

void Slider::onShowPress(XDim x, YDim y) {
  if (!isEnabled()) return;
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  int16_t primary_coord = axis.primaryCoordFromPoint(x, y);
  if (!axis.hitsThumbAtValue(range_, value_, primary_coord, kTouchSlopPixels)) {
    return;
  }

  if (!is_dragging_) {
    is_dragging_ = true;
    onInteractionStart();
  }
  if (setValueInternal(axis.valueFromPrimaryCoord(range_, primary_coord),
                       true)) {
    triggerInteractiveChange();
  }
  Widget::onShowPress(x, y);
}

bool Slider::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  if (!isEnabled()) return false;
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  if (!axis.shouldCaptureScroll(is_dragging_, dx, dy)) {
    return false;
  }

  bool was_dragging = is_dragging_;
  if (setValueInternal(
          axis.valueFromPrimaryCoord(range_, axis.primaryCoordFromPoint(x, y)),
          true)) {
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

bool Slider::onTouchUp(XDim x, YDim y) {
  if (is_dragging_) {
    if (isPressed()) {
      setPressed(false);
    }
    onInteractionEnd(value_);
    is_dragging_ = false;
    return true;
  }
  return Widget::onTouchUp(x, y);
}

void Slider::onCancel() {
  if (is_dragging_) {
    onInteractionEnd(value_);
  }
  BasicWidget::onCancel();
  is_dragging_ = false;
}

bool Slider::setValueInternal(float value, bool from_user) {
  float new_value = internal::NormalizeSliderValueForRange(
      value, range_.from, range_.to, range_.step);
  if (value_ == new_value) return false;
  float old_value = value_;
  value_ = new_value;
  onValueChange(value_, from_user);
  if (width() <= 0 || height() <= 0) return true;
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  float old_center = axis.centerFromValue(range_, old_value);
  float new_center = axis.centerFromValue(range_, new_value);
  if (old_center != new_center) {
    invalidateValueChange(axis, old_center, new_center, new_value);
  }
  return true;
}

bool Slider::setRange(SliderRange range) {
  if (!internal::IsValidSliderRange(range.from, range.to, range.step)) {
    return false;
  }
  float new_value = internal::NormalizeSliderValueForRange(
      value_, range.from, range.to, range.step);
  bool changed = range_.from != range.from || range_.to != range.to ||
                 range_.step != range.step || value_ != new_value;
  if (!changed) return false;

  SliderRange old_range = range_;
  float old_value = value_;
  range_ = range;
  value_ = new_value;
  onValueChange(value_, false);

  if (width() > 0 && height() > 0) {
    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    float old_center = axis.centerFromValue(old_range, old_value);
    float new_center = axis.centerFromValue(range_, new_value);
    if (old_center != new_center) {
      invalidateValueChange(axis, old_center, new_center, new_value);
    }
  }
  return true;
}

bool Slider::setVariant(SliderVariant variant) {
  if (variant_ == variant) return false;
  variant_ = variant;
  invalidateInterior();
  return true;
}

bool Slider::setStyle(SliderStyle style) {
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

bool Slider::setValue(float value) { return setValueInternal(value, false); }

// Marks the minimal area that needs to be redrawn for a thumb move from
// `old_center` to `new_center`. Uses setDirty() (not invalidateInterior()) so
// the slider itself is not marked invalidated: paint() can rely on
// isInvalidated() being false here and let the canvas clip restrict
// drawing to the dirty slice. When the value indicator is visible, the
// dirty rect is complemented by the bubble strip swept by the two thumb
// positions (rather than the conservative full-envelope reported by
// getParentTransientPaintBounds()), and the parent is told to invalidate only
// that strip so siblings beneath repaint just the area the old bubble vacated.
void Slider::invalidateValueChange(const internal::SliderAxisMetrics& axis,
                                   float old_center, float new_center,
                                   float new_value) {
  Rect thumb_rect =
      axis.invalidationRectForCenterChange(old_center, new_center);
  Rect icon_envelope(0, 0, -1, -1);
  Rect old_icon = insetIconDirtyRect(buildMetrics(old_center, isPressed()));
  Rect new_icon = insetIconDirtyRect(buildMetrics(new_center, isPressed()));
  if (!old_icon.empty() || !new_icon.empty()) {
    icon_envelope = Rect::Extent(old_icon, new_icon);
  }
  Rect content_envelope = icon_envelope.empty()
                              ? thumb_rect
                              : Rect::Extent(thumb_rect, icon_envelope);
  Rect bubble_envelope(0, 0, -1, -1);
  if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
    float c_new = axis.displayPrimary(new_center);
    char new_scratch[64];
    roo::string_view new_text =
        formatLabel(new_value, new_scratch, sizeof(new_scratch));
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
      internal::DisplayMainSpanFromRect(content_envelope, axis.isVertical()));
  pending_indicator_dirty_span_.include(
      internal::DisplayMainSpanFromRect(bubble_envelope, axis.isVertical()));
  setDirty(content_envelope);
  if (!bubble_envelope.empty()) {
    notifyParentInvalidatedRegion(
        bubble_envelope.translate(offsetLeft(), offsetTop()));
  }
}

void Slider::notifyStateChanged(uint16_t state_diff) {
  if ((state_diff & kWidgetPressed) == 0) {
    Widget::notifyStateChanged(state_diff);
    return;
  }

  bool was_pressed = !isPressed();

  if (width() <= 0 || height() <= 0) {
    setDirty();
    return;
  }

  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  float center = axis.centerFromValue(range_, value_);
  Rect thumb_rect = axis.invalidationRectForCenterChange(center, center);

  bool old_indicator_visible =
      style_.value_indicator == SliderValueIndicatorBehavior::kAlways ||
      was_pressed || isDragged();
  bool new_indicator_visible = ShowsValueIndicator(*this);

  Rect bubble_envelope(0, 0, -1, -1);
  if (old_indicator_visible != new_indicator_visible) {
    float display_center = axis.displayPrimary(center);
    bubble_envelope = ValueIndicatorBubble::EnvelopeForCenterRange(
        width(), height(), display_center, display_center,
        style_.value_indicator, style_.orientation);
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

void Slider::paint(const Canvas& canvas) const {
  (void)canvas;
  LOG(DFATAL) << "Slider::paint() should not be called; render via "
              << "paintWidgetContents()/paintTrackAndThumb() instead";
}

void Slider::paintTrackAndThumb(const Canvas& canvas, const Metrics& metrics,
                                const Tokens& tokens) const {
  float center_anchor_primary =
      metrics.axis.centerFromValue(range_, 0.5f * (range_.from + range_.to));

  auto track_bounds = metrics.axis.paintRectFromPrimaryCross(
      TrackShapeMinPrimary(0.0f, metrics.size_metrics.track_radius),
      metrics.layout.track_min_cross, metrics.axis.primarySpan() - 0.5f,
      metrics.layout.track_max_cross);
  auto handle_bounds = metrics.axis.paintRectFromPrimaryCross(
      metrics.layout.thumb_min_primary, metrics.layout.thumb_min_cross,
      metrics.layout.thumb_max_primary, metrics.layout.thumb_max_cross);

  auto inactive_track = SmoothFilledRoundRect(
      track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
      track_bounds.y_max, metrics.size_metrics.track_radius,
      tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
      track_bounds.y_max, metrics.size_metrics.track_radius,
      tokens.active_track);
  auto handle = SmoothFilledRoundRect(
      handle_bounds.x_min, handle_bounds.y_min, handle_bounds.x_max,
      handle_bounds.y_max, internal::kHandleCornerRadius, tokens.handle);
  Rect widget_bounds = bounds();
  Rect handle_tile_bounds =
      GetHandleTileBounds(widget_bounds, handle.extents(), metrics.track_gap,
                          metrics.axis.isVertical());

  // Paint the same segment model used by stop rendering so track and stop
  // passes agree on every active/inactive boundary.
  for (int i = 0; i < metrics.segment_count; ++i) {
    roo_display::Box track_clip = TrackSegmentClipBox(
        metrics.axis, metrics.segments[i], metrics.layout.track_cross_start,
        metrics.size_metrics.track_height);
    if (track_clip.empty()) continue;
    const auto& track_piece =
        metrics.segments[i].active ? active_track : inactive_track;
    DrawTrackPiece(canvas, track_piece, widget_bounds, track_clip,
                   metrics.axis.isVertical());
  }
  if (variant_ == SliderVariant::kCentered) {
    // The center gap is painted back to the widget background after the track
    // pieces so the left and right halves remain visibly separated at zero.
    int16_t gap_min =
        (int16_t)floorf(center_anchor_primary - (float)kCenterGapHalfPixels) +
        1;
    int16_t gap_max =
        (int16_t)ceilf(center_anchor_primary + (float)kCenterGapHalfPixels) - 1;
    if (gap_min < 0) gap_min = 0;
    if (gap_max >= metrics.axis.primarySpan()) {
      gap_max = metrics.axis.primarySpan() - 1;
    }
    if (gap_min <= gap_max) {
      Rect gap_rect = Rect(metrics.axis.boxFromPrimaryCross(
          gap_min, 0, gap_max, metrics.axis.crossSpan() - 1));
      canvas.fillRect(gap_rect, canvas.bgcolor());
    }
  }
  canvas.drawTiled(handle, handle_tile_bounds, kNoAlign);
}

void Slider::paintStops(const Canvas& canvas, Clipper& clipper,
                        const Metrics& metrics, const Tokens& tokens) const {
  if (!ShouldRenderStops(*this) || width() <= 0 || height() <= 0 ||
      metrics.segment_count == 0) {
    return;
  }

  Rect reserved_icon_rect = insetIconReservedRect(metrics);
  internal::StopSuppressionFn suppress_stop;
  if (!reserved_icon_rect.empty()) {
    suppress_stop = [&](const roo_display::Box& stop_extents) {
      return reserved_icon_rect.intersects(stop_extents);
    };
  }
  PaintStopRuns(canvas, clipper, tokens, metrics.axis, metrics.segments,
                metrics.segment_count, range_, suppress_stop);
}

Dimensions Slider::getSuggestedMinimumDimensions() const {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  return Dimensions(size_metrics.handle_height, size_metrics.handle_height);
}

PreferredSize Slider::getPreferredSize() const {
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  if (style_.orientation == SliderOrientation::kVertical) {
    return PreferredSize(PreferredSize::ExactWidth(size_metrics.handle_height),
                         PreferredSize::MatchParentHeight());
  }
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::ExactHeight(size_metrics.handle_height));
}

roo_display::FpPoint Slider::getPointOverlayFocus() const {
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  float display_center = axis.displayCenterFromValue(range_, value_);
  if (style_.orientation == SliderOrientation::kVertical) {
    return roo_display::FpPoint{axis.centeredCross(), display_center};
  }
  return roo_display::FpPoint{display_center, axis.centeredCross()};
}

ColorRole Slider::effectiveContainerRole() const { return ColorRole::kPrimary; }

Rect Slider::getSloppyTouchParentBounds() const {
  Rect bounds = Widget::getSloppyTouchParentBounds();
  if (style_.orientation == SliderOrientation::kVertical) {
    return Rect(bounds.xMin(), bounds.yMin() - kTouchSlopPixels, bounds.xMax(),
                bounds.yMax() + kTouchSlopPixels);
  }
  return Rect(bounds.xMin() - kTouchSlopPixels, bounds.yMin(),
              bounds.xMax() + kTouchSlopPixels, bounds.yMax());
}

roo::string_view Slider::formatLabel(float value, char* scratch,
                                     size_t scratch_size) const {
  return ValueIndicatorBubble::FormatDefault(value, scratch, scratch_size);
}

void SliderWithInsetIcon::setIcon(const roo_display::Pictogram* icon,
                                  SliderInsetIconAnchor anchor) {
  if (icon_.icon == icon && icon_.anchor == anchor) return;
  icon_ = {icon, anchor};
  invalidateInterior();
}

Rect Slider::getParentTransientPaintBounds() const {
  // Start with whatever the base widget reports (currently just bounds; in
  // the future this may include other transient overflow). Then, if the
  // value indicator may be shown this frame, union with a conservative
  // envelope big enough to cover the bubble at any thumb position. This is
  // what tells the framework to invalidate the bubble area in the parent
  // when isPressed/isDragged flips.
  Rect base = BasicWidget::getParentTransientPaintBounds();
  if (style_.value_indicator == SliderValueIndicatorBehavior::kHidden) {
    return base;
  }
  bool may_show =
      style_.value_indicator == SliderValueIndicatorBehavior::kAlways ||
      isPressed() || isDragged();
  if (!may_show || width() <= 0 || height() <= 0) return base;
  Rect bubble_local = ValueIndicatorBubble::ConservativeBounds(
      width(), height(), internal::kHandleWidth, style_.value_indicator,
      style_.orientation);
  if (bubble_local.empty()) return base;
  Rect bubble_parent = bubble_local.translate(offsetLeft(), offsetTop());
  return Rect::Extent(base, bubble_parent);
}

void Slider::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  // The canvas we receive is clipped to maxBounds(), which (via our
  // getParentTransientPaintBounds() override) covers the full bubble
  // conservative envelope when the indicator is showing. If the only
  // reason we're being repainted is a value/style state change that
  // dirtied a tight rectangle (set by invalidateValueChange()), narrow the
  // canvas clip to that rectangle so paint() does not redraw the entire
  // envelope. isInvalidated() being true means the framework asked us to
  // fully repaint (e.g. style change, visibility toggle), in which case
  // we keep the full clip.
  internal::DirtySpan pending_content_span = pending_content_dirty_span_;
  internal::DirtySpan pending_indicator_span = pending_indicator_dirty_span_;
  pending_content_dirty_span_ = internal::DirtySpan();

  // The area of the content that needs to be repainted.
  Rect pending_content = internal::ContentRectFromDisplayMainSpan(
      pending_content_span, width(), height(),
      style_.orientation == SliderOrientation::kVertical);
  // The area of the value indicator that needs to be repainted.
  Rect pending_indicator = IndicatorDirtyRectFromSpan(
      pending_indicator_span, width(), height(), style_);

  internal::DirtySpan current_indicator_span = pending_indicator_span;

  bool invalidated = isInvalidated();

  if (ShowsValueIndicator(*this) &&
      (invalidated || !pending_indicator.empty())) {
    Canvas indicator_canvas = canvas;
    if (!invalidated) {
      indicator_canvas.clipToExtents(pending_indicator);
    }
    if (!indicator_canvas.clip_box().empty() && width() > 0 && height() > 0) {
      // Pre-paint the value indicator bubble BEFORE the slider's track and
      // thumb. This way, even if the bubble's geometry overlapped the
      // slider's rectangular bounds (it does not today, but might if kGap or
      // kPaddingV were tuned aggressively), the slider would still appear
      // behind the bubble: subsequent slider writes inside the bubble's
      // inscribed inner rectangle are blocked by the clipper exclusion
      // installed by decorate(), and writes in the bubble's rounded-corner
      // strips have the bubble's decoration overlay alpha-composited on top.
      //
      // The canvas received here has been shifted to slider-local coordinates
      // and clipped to maxBounds(). Because Widget::getVisualBounds() (the
      // default for maxBounds()) unions in getTransientPaintBounds(), and our
      // getParentTransientPaintBounds() override already accounts for the
      // bubble's conservative envelope when the indicator is showing, the
      // canvas clip already covers the bubble area.
      char scratch[64];
      roo::string_view text = formatLabel(value_, scratch, sizeof(scratch));
      internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
      Rect indicator_bounds = PaintValueIndicator(
          theme(), isEnabled(), indicator_canvas, clipper, width(), height(),
          axis.displayCenterFromValue(range_, value_), style_, text);
      if (!indicator_bounds.empty()) {
        current_indicator_span = internal::DisplayMainSpanFromRect(
            indicator_bounds,
            style_.orientation == SliderOrientation::kVertical);
      }
    }
  }

  Canvas content_canvas = prepareContentsCanvas(canvas);
  if (!invalidated && !pending_content.empty()) {
    content_canvas.clipToExtents(pending_content);
  }
  if (!content_canvas.clip_box().empty()) {
    clipper.setBounds(content_canvas.clip_box());
    Metrics metrics = buildMetrics();
    Tokens tokens = ResolveTokens(*this);
    paintInsetIcons(content_canvas, clipper, metrics, tokens);
    paintStops(content_canvas, clipper, metrics, tokens);
    paintTrackAndThumb(content_canvas, metrics, tokens);
  }
  pending_indicator_dirty_span_ = ShowsValueIndicator(*this)
                                      ? current_indicator_span
                                      : internal::DirtySpan();
  markClean();
}

}  // namespace material3
}  // namespace roo_windows
