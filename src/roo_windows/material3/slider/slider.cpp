#include "roo_windows/material3/slider/slider.h"

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
static constexpr float kPressedThumbWidthRatio = 0.5f;
static constexpr int16_t kCenterGapHalfPixels = Scaled(2);
static constexpr int16_t kTrackIconEdgeOffsetPixels = Scaled(4);
static constexpr int16_t kTrackIconMinHandleCenterDistancePixels = Scaled(12);
static constexpr int16_t kTrackIconStopPaddingPixels = Scaled(4);
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

Tokens ResolveTokens(const Slider& widget) {
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

bool ShouldRenderStops(const Slider& widget) {
  if (!internal::IsDiscreteSliderRange(widget.range().step)) return false;
  return widget.style().tick_mode != SliderTickMode::kHidden;
}

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

int16_t IconPrimarySpan(const roo_display::Pictogram& icon, bool vertical) {
  roo_display::Box extents = icon.anchorExtents();
  return vertical ? extents.height() : extents.width();
}

int16_t IconCrossSpan(const roo_display::Pictogram& icon, bool vertical) {
  roo_display::Box extents = icon.anchorExtents();
  return vertical ? extents.width() : extents.height();
}

bool IconFitsWithinSlot(const roo_display::Pictogram* icon, int16_t slot_size,
                        bool vertical) {
  if (icon == nullptr || slot_size <= 0) return false;
  roo_display::Box extents = icon->anchorExtents();
  int16_t primary_span = vertical ? extents.height() : extents.width();
  int16_t cross_span = vertical ? extents.width() : extents.height();
  return primary_span <= slot_size && cross_span <= slot_size;
}

bool CanPaintInsetIcon(const Slider& slider,
                       const internal::SliderSizeMetrics& size_metrics) {
  return slider.variant() == SliderVariant::kStandard &&
         size_metrics.supports_inset_icon && size_metrics.icon_size > 0;
}

void DrawTrackPiece(const Canvas& canvas, const roo_display::Drawable& piece,
                    const Rect& widget_bounds,
                    const roo_display::Box& clip_bounds, bool vertical) {
  if (clip_bounds.empty()) return;
  Rect tile_bounds = GetTrackTileBounds(widget_bounds, clip_bounds, vertical);
  canvas.drawTiled(piece, tile_bounds, kNoAlign);
}

internal::SliderAxisMetrics MakeSliderAxisMetrics(const Slider& slider) {
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
  bool active;
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

inline bool SegmentContainsRange(const StopSegment& segment, float min_primary,
                                 float max_primary) {
  return min_primary >= segment.min_primary - 0.01f &&
         max_primary <= segment.max_primary + 0.01f;
}

void BuildTrackSegments(const Slider& slider, const Tokens& tokens,
                        const internal::SliderAxisMetrics& axis,
                        const internal::SliderVisualMetrics& layout,
                        StopSegment* segments, int& segment_count) {
  segment_count = 0;
  float center_anchor_primary = axis.centerFromPos(32768);
  bool thumb_on_or_right_of_center =
      axis.centerFromPos(slider.getPos()) >= center_anchor_primary;
  float center_left_edge = center_anchor_primary - (float)kCenterGapHalfPixels;
  float center_right_edge = center_anchor_primary + (float)kCenterGapHalfPixels;

  if (slider.variant() == SliderVariant::kCentered) {
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
          tokens.inactive_track, tokens.inactive_stop, false};
    }

    float active_track_min_primary = thumb_on_or_right_of_center
                                         ? center_right_edge
                                         : layout.inactive_track_min_primary;
    float active_track_max_primary = thumb_on_or_right_of_center
                                         ? layout.active_track_max_primary
                                         : center_left_edge;
    if (active_track_max_primary >= active_track_min_primary) {
      segments[segment_count++] =
          StopSegment{active_track_min_primary, active_track_max_primary,
                      tokens.active_track, tokens.active_stop, true};
    }

    int16_t handle_right_edge =
        (int16_t)ceilf(layout.inactive_track_min_primary);
    int16_t center_right_edge_i = (int16_t)ceilf(center_right_edge);
    int16_t right_inactive_min =
        thumb_on_or_right_of_center
            ? handle_right_edge
            : std::max(handle_right_edge, center_right_edge_i);
    if (right_inactive_min < axis.primarySpan()) {
      segments[segment_count++] =
          StopSegment{(float)std::max<int16_t>(0, right_inactive_min),
                      (float)(axis.primarySpan() - 1), tokens.inactive_track,
                      tokens.inactive_stop, false};
    }
    return;
  }

  if (layout.active_track_max_primary >= 0.0f) {
    segments[segment_count++] =
        StopSegment{0.0f, layout.active_track_max_primary, tokens.active_track,
                    tokens.active_stop, true};
  }
  if (layout.inactive_track_min_primary < axis.primarySpan()) {
    segments[segment_count++] = StopSegment{
        layout.inactive_track_min_primary, (float)(axis.primarySpan() - 1),
        tokens.inactive_track, tokens.inactive_stop, false};
  }
}

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

bool ResolveTrackIconBounds(const internal::SliderAxisMetrics& axis,
                            const internal::SliderVisualMetrics& layout,
                            const internal::SliderSizeMetrics& size_metrics,
                            const roo_display::Pictogram& icon, bool at_start,
                            const StopSegment* segments, int segment_count,
                            roo_display::Box& icon_bounds,
                            const StopSegment*& icon_segment) {
  int16_t max_slot_span = size_metrics.icon_size > 0
                              ? size_metrics.icon_size
                              : size_metrics.track_height;
  if (!IconFitsWithinSlot(&icon, max_slot_span, axis.isVertical())) {
    return false;
  }

  int16_t icon_primary_span = IconPrimarySpan(icon, axis.isVertical());
  int16_t icon_cross_span = IconCrossSpan(icon, axis.isVertical());
  if (icon_primary_span <= 0 || icon_cross_span <= 0 ||
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
                        (float)kTrackIconMinHandleCenterDistancePixels);
    int16_t min_right_boundary =
        (int16_t)ceilf(layout.thumb_center_primary +
                       (float)kTrackIconMinHandleCenterDistancePixels);
    if (max_primary > max_left_boundary && min_primary < min_right_boundary) {
      return false;
    }
    const StopSegment* containing = FindSegmentContainingRange(
        segments, segment_count, (float)min_primary, (float)max_primary);
    if (containing == nullptr) return false;
    icon_bounds =
        axis.boxFromPrimaryCross(min_primary, cross_start, max_primary,
                                 cross_start + icon_cross_span - 1);
    icon_segment = containing;
    return true;
  };

  int16_t edge_candidate =
      at_start
          ? kTrackIconEdgeOffsetPixels
          : axis.primarySpan() - kTrackIconEdgeOffsetPixels - icon_primary_span;
  if (try_candidate(edge_candidate)) return true;

  int16_t jump_candidate =
      at_start
          ? std::max<int16_t>(
                (int16_t)ceilf(layout.inactive_track_min_primary),
                (int16_t)ceilf(layout.thumb_center_primary +
                               (float)kTrackIconMinHandleCenterDistancePixels))
          : std::min<int16_t>(
                (int16_t)floorf(layout.active_track_max_primary) -
                    icon_primary_span + 1,
                (int16_t)floorf(
                    layout.thumb_center_primary -
                    (float)kTrackIconMinHandleCenterDistancePixels) -
                    icon_primary_span + 1);
  return try_candidate(jump_candidate);
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

Rect ExpandTrackIconPrimaryRect(const Rect& icon_rect, bool vertical,
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

struct Slider::PaintContext {
  Tokens tokens;
  internal::SliderSizeMetrics size_metrics;
  internal::SliderAxisMetrics axis;
  int16_t thumb_width;
  int16_t track_gap;
  internal::SliderVisualMetrics layout;
  StopSegment segments[3];
  int segment_count;
};

Slider::PaintContext Slider::buildPaintContext() const {
  return buildPaintContext(getPos(), isPressed());
}

Slider::PaintContext Slider::buildPaintContext(uint16_t pos,
                                               bool pressed) const {
  PaintContext context{
      .tokens = ResolveTokens(*this),
      .size_metrics = internal::ResolveSliderSizeMetrics(style_.size),
      .axis = MakeSliderAxisMetrics(*this),
      .thumb_width = 0,
      .track_gap = 0,
      .layout = internal::SliderVisualMetrics(),
      .segments = {},
      .segment_count = 0,
  };
  context.thumb_width =
      ThumbWidthForState(context.size_metrics.handle_width, pressed);
  context.track_gap =
      TrackGapForThumbWidth(context.size_metrics, context.thumb_width);
  context.layout = internal::ResolveSliderVisualMetrics(
      context.axis, context.axis.centerFromPos(pos), context.thumb_width,
      context.size_metrics.track_height, context.track_gap,
      context.size_metrics.handle_height);
  BuildTrackSegments(*this, context.tokens, context.axis, context.layout,
                     context.segments, context.segment_count);
  return context;
}

Rect Slider::trackIconRect(const PaintContext& context) const {
  InsetIcon inset_icon = getInsetIcon();
  if (inset_icon.icon == nullptr || context.segment_count == 0) {
    return Rect(0, 0, -1, -1);
  }

  roo_display::Box inset_bounds;
  const StopSegment* inset_segment = nullptr;
  bool inset_at_start = inset_icon.anchor == SliderTrackIconAnchor::kStart;
  if (!CanPaintInsetIcon(*this, context.size_metrics) ||
      !IconFitsWithinSlot(inset_icon.icon, context.size_metrics.icon_size,
                          context.axis.isVertical()) ||
      !ResolveTrackIconBounds(
          context.axis, context.layout, context.size_metrics, *inset_icon.icon,
          inset_at_start, context.segments, context.segment_count, inset_bounds,
          inset_segment)) {
    return Rect(0, 0, -1, -1);
  }
  return Rect(inset_bounds);
}

Rect Slider::trackIconReservedRect(const PaintContext& context) const {
  return ExpandTrackIconPrimaryRect(
      trackIconRect(context), context.axis.isVertical(),
      context.axis.primarySpan(), kTrackIconStopPaddingPixels);
}

Rect Slider::trackIconDirtyRect(const PaintContext& context) const {
  return ExpandTrackIconPrimaryRect(
      trackIconReservedRect(context), context.axis.isVertical(),
      context.axis.primarySpan(), kStopMarkRadiusPixels);
}

void Slider::paintTrackIcons(const Canvas& canvas, Clipper& clipper,
                             const PaintContext& context) const {
  InsetIcon inset_icon = getInsetIcon();
  if (inset_icon.icon == nullptr || context.segment_count == 0) {
    return;
  }

  auto paint_icon = [&](const roo_display::Pictogram& icon,
                        const roo_display::Box& icon_bounds,
                        const StopSegment& segment) {
    Canvas icon_canvas = canvas;
    icon_canvas.set_bgcolor(segment.track_color);
    icon_canvas.clip(icon_bounds.translate(canvas.dx(), canvas.dy()));
    if (icon_canvas.clip_box().empty()) return;
    roo_display::Pictogram tinted_icon(icon);
    tinted_icon.color_mode().setColor(segment.stop_color);
    icon_canvas.drawTiled(tinted_icon, Rect(icon_bounds), kCenter | kMiddle,
                          false);
    roo_display::Box device_box = roo_display::Box::Intersect(
        icon_bounds.translate(canvas.dx(), canvas.dy()), canvas.clip_box());
    if (!device_box.empty()) {
      clipper.addExclusion(device_box);
    }
  };

  Rect inset_rect = trackIconRect(context);
  if (!inset_rect.empty()) {
    float min_primary =
        context.axis.isVertical()
            ? (float)(context.axis.primarySpan() - 1 - inset_rect.yMax())
            : (float)inset_rect.xMin();
    float max_primary =
        context.axis.isVertical()
            ? (float)(context.axis.primarySpan() - 1 - inset_rect.yMin())
            : (float)inset_rect.xMax();
    const StopSegment* inset_segment = FindSegmentContainingRange(
        context.segments, context.segment_count, min_primary, max_primary);
    if (inset_segment != nullptr) {
      paint_icon(*inset_icon.icon, inset_rect.asBox(), *inset_segment);
    }
  }
}

Slider::Slider(const Environment& env, uint16_t pos)
    : Slider(env, SliderRange{},
             internal::SliderValueFromNormalizedPos(0.0f, 1.0f, pos)) {}

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
  uint16_t pos = axis.posFromPrimaryCoord(axis.primaryCoordFromPoint(x, y));
  onInteractionStart();
  if (setPosInternal(pos, true)) {
    triggerInteractiveChange();
  }
  onInteractionEnd(value_);
  return true;
}

void Slider::onShowPress(XDim x, YDim y) {
  if (!isEnabled()) return;
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  int16_t primary_coord = axis.primaryCoordFromPoint(x, y);
  if (!axis.hitsThumb(getPos(), primary_coord, kTouchSlopPixels)) return;

  if (!is_dragging_) {
    is_dragging_ = true;
    onInteractionStart();
  }
  if (setPosInternal(axis.posFromPrimaryCoord(primary_coord), true)) {
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
  if (setPosInternal(axis.posFromPrimaryCoord(axis.primaryCoordFromPoint(x, y)),
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

bool Slider::setPos(uint16_t pos) { return setPosInternal(pos, false); }

bool Slider::setPosInternal(uint16_t pos, bool from_user) {
  uint16_t old_pos = getPos();
  if (pos == old_pos) return false;
  value_ = internal::NormalizeSliderValueForRange(
      internal::SliderValueFromNormalizedPos(range_.from, range_.to, pos),
      range_.from, range_.to, range_.step);
  uint16_t new_pos = getPos();
  if (new_pos == old_pos) return false;
  onValueChange(value_, from_user);
  if (width() <= 0 || height() <= 0) {
    return true;
  }
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  invalidatePosChange(axis, old_pos, new_pos, value_);
  return true;
}

bool Slider::setRange(SliderRange range) {
  if (!internal::IsValidSliderRange(range.from, range.to, range.step)) {
    return false;
  }
  uint16_t old_pos = getPos();
  float new_value = internal::NormalizeSliderValueForRange(
      value_, range.from, range.to, range.step);
  bool changed = range_.from != range.from || range_.to != range.to ||
                 range_.step != range.step || value_ != new_value;
  if (!changed) return false;

  range_ = range;
  value_ = new_value;
  onValueChange(value_, false);

  uint16_t new_pos = getPos();
  if (width() > 0 && height() > 0 && old_pos != new_pos) {
    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    invalidatePosChange(axis, old_pos, new_pos, value_);
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

bool Slider::setValue(float value) {
  float new_value = internal::NormalizeSliderValueForRange(
      value, range_.from, range_.to, range_.step);
  if (value_ == new_value) return false;

  uint16_t old_pos = getPos();
  value_ = new_value;
  onValueChange(value_, false);
  uint16_t new_pos = getPos();

  if (width() > 0 && height() > 0 && old_pos != new_pos) {
    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    invalidatePosChange(axis, old_pos, new_pos, value_);
  }
  return true;
}

// Marks the minimal area that needs to be redrawn for a thumb move from
// `old_pos` to `new_pos`. Uses setDirty() (not invalidateInterior()) so
// the slider itself is not marked invalidated: paint() can rely on
// isInvalidated() being false here and let the canvas clip restrict
// drawing to the dirty slice. When the value indicator is visible, the
// dirty rect is complemented by the bubble strip swept by the two thumb
// positions (rather than the conservative full-envelope reported by
// getParentTransientPaintBounds()), and the parent is told to invalidate only
// that strip so siblings beneath repaint just the area the old bubble vacated.
void Slider::invalidatePosChange(const internal::SliderAxisMetrics& axis,
                                 uint16_t old_pos, uint16_t new_pos,
                                 float new_value) {
  Rect thumb_rect = axis.invalidationRectForPosChange(old_pos, new_pos);
  Rect icon_envelope(0, 0, -1, -1);
  Rect old_icon = trackIconDirtyRect(buildPaintContext(old_pos, isPressed()));
  Rect new_icon = trackIconDirtyRect(buildPaintContext(new_pos, isPressed()));
  if (!old_icon.empty() || !new_icon.empty()) {
    icon_envelope = Rect::Extent(old_icon, new_icon);
  }
  Rect content_envelope = icon_envelope.empty()
                              ? thumb_rect
                              : Rect::Extent(thumb_rect, icon_envelope);
  Rect bubble_envelope(0, 0, -1, -1);
  if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
    float c_new = axis.displayCenterFromPos(new_pos);
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
  uint16_t pos = getPos();
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

uint16_t Slider::getPos() const {
  return internal::SliderPosFromValue(range_.from, range_.to, value_);
}

void Slider::paint(const Canvas& canvas) const {
  paintTrackAndThumb(canvas, buildPaintContext());
}

void Slider::paintTrackAndThumb(const Canvas& canvas,
                                const PaintContext& context) const {
  float center_anchor_primary = context.axis.centerFromPos(32768);
  bool thumb_on_or_right_of_center =
      context.axis.centerFromPos(getPos()) >= center_anchor_primary;
  float center_left_edge = center_anchor_primary - (float)kCenterGapHalfPixels;
  float center_right_edge = center_anchor_primary + (float)kCenterGapHalfPixels;

  float active_track_min_primary = -0.5f;
  float active_track_max_primary = context.layout.active_track_max_primary;
  roo_display::Box active_clip = context.axis.boxFromPrimaryCross(
      0, context.layout.track_cross_start,
      context.layout.activeClipMax(context.axis.primarySpan()),
      context.layout.track_cross_start + context.size_metrics.track_height - 1);
  bool has_left_inactive_clip = false;
  bool has_right_inactive_clip = true;
  roo_display::Box left_inactive_clip;
  roo_display::Box right_inactive_clip = context.axis.boxFromPrimaryCross(
      (int16_t)ceilf(context.layout.inactive_track_min_primary),
      context.layout.track_cross_start, context.axis.primarySpan() - 1,
      context.layout.track_cross_start + context.size_metrics.track_height - 1);

  if (variant_ == SliderVariant::kCentered) {
    active_track_min_primary = thumb_on_or_right_of_center
                                   ? center_right_edge
                                   : context.layout.inactive_track_min_primary;
    active_track_max_primary = thumb_on_or_right_of_center
                                   ? context.layout.active_track_max_primary
                                   : center_left_edge;

    int16_t active_clip_min = (int16_t)ceilf(active_track_min_primary);
    int16_t active_clip_max = (int16_t)floorf(active_track_max_primary);
    if (active_clip_min < 0) active_clip_min = 0;
    if (active_clip_max >= context.axis.primarySpan()) {
      active_clip_max = context.axis.primarySpan() - 1;
    }
    if (active_clip_max >= active_clip_min) {
      active_clip = context.axis.boxFromPrimaryCross(
          active_clip_min, context.layout.track_cross_start, active_clip_max,
          context.layout.track_cross_start + context.size_metrics.track_height -
              1);
    } else {
      active_clip = roo_display::Box(0, 0, -1, -1);
    }

    has_left_inactive_clip = true;
    int16_t handle_left_edge =
        (int16_t)floorf(context.layout.active_track_max_primary);
    int16_t center_left_edge_i = (int16_t)floorf(center_left_edge);
    int16_t left_inactive_max =
        thumb_on_or_right_of_center
            ? std::min(handle_left_edge, center_left_edge_i)
            : handle_left_edge;
    if (left_inactive_max < 0) {
      has_left_inactive_clip = false;
    } else {
      left_inactive_clip = context.axis.boxFromPrimaryCross(
          0, context.layout.track_cross_start,
          left_inactive_max >= context.axis.primarySpan()
              ? context.axis.primarySpan() - 1
              : left_inactive_max,
          context.layout.track_cross_start + context.size_metrics.track_height -
              1);
    }

    int16_t handle_right_edge =
        (int16_t)ceilf(context.layout.inactive_track_min_primary);
    int16_t center_right_edge_i = (int16_t)ceilf(center_right_edge);
    int16_t right_inactive_min =
        thumb_on_or_right_of_center
            ? handle_right_edge
            : std::max(handle_right_edge, center_right_edge_i);
    if (right_inactive_min >= context.axis.primarySpan()) {
      has_right_inactive_clip = false;
    } else {
      if (right_inactive_min < 0) right_inactive_min = 0;
      right_inactive_clip = context.axis.boxFromPrimaryCross(
          right_inactive_min, context.layout.track_cross_start,
          context.axis.primarySpan() - 1,
          context.layout.track_cross_start + context.size_metrics.track_height -
              1);
    }
  } else {
    int16_t right_inactive_min =
        (int16_t)ceilf(context.layout.inactive_track_min_primary);
    if (right_inactive_min >= context.axis.primarySpan()) {
      has_right_inactive_clip = false;
    } else {
      if (right_inactive_min < 0) right_inactive_min = 0;
      right_inactive_clip = context.axis.boxFromPrimaryCross(
          right_inactive_min, context.layout.track_cross_start,
          context.axis.primarySpan() - 1,
          context.layout.track_cross_start + context.size_metrics.track_height -
              1);
    }
  }

  auto track_bounds = context.axis.paintRectFromPrimaryCross(
      TrackShapeMinPrimary(0.0f, context.size_metrics.track_radius),
      context.layout.track_min_cross, context.axis.primarySpan() - 0.5f,
      context.layout.track_max_cross);
  auto handle_bounds = context.axis.paintRectFromPrimaryCross(
      context.layout.thumb_min_primary, context.layout.thumb_min_cross,
      context.layout.thumb_max_primary, context.layout.thumb_max_cross);

  auto inactive_track = SmoothFilledRoundRect(
      track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
      track_bounds.y_max, context.size_metrics.track_radius,
      context.tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      track_bounds.x_min, track_bounds.y_min, track_bounds.x_max,
      track_bounds.y_max, context.size_metrics.track_radius,
      context.tokens.active_track);
  auto handle = SmoothFilledRoundRect(handle_bounds.x_min, handle_bounds.y_min,
                                      handle_bounds.x_max, handle_bounds.y_max,
                                      context.size_metrics.handleCornerRadius(),
                                      context.tokens.handle);
  Rect widget_bounds = bounds();
  Rect handle_tile_bounds =
      GetHandleTileBounds(widget_bounds, handle.extents(), context.track_gap,
                          context.axis.isVertical());

  if (has_left_inactive_clip) {
    DrawTrackPiece(canvas, inactive_track, widget_bounds, left_inactive_clip,
                   context.axis.isVertical());
  }
  if (has_right_inactive_clip) {
    DrawTrackPiece(canvas, inactive_track, widget_bounds, right_inactive_clip,
                   context.axis.isVertical());
  }
  if (!active_clip.empty()) {
    DrawTrackPiece(canvas, active_track, widget_bounds, active_clip,
                   context.axis.isVertical());
  }
  if (variant_ == SliderVariant::kCentered) {
    int16_t gap_min =
        (int16_t)floorf(center_anchor_primary - (float)kCenterGapHalfPixels) +
        1;
    int16_t gap_max =
        (int16_t)ceilf(center_anchor_primary + (float)kCenterGapHalfPixels) - 1;
    if (gap_min < 0) gap_min = 0;
    if (gap_max >= context.axis.primarySpan()) {
      gap_max = context.axis.primarySpan() - 1;
    }
    if (gap_min <= gap_max) {
      Rect gap_rect =
          context.axis.isVertical()
              ? Rect(0, context.axis.primarySpan() - 1 - gap_max, width() - 1,
                     context.axis.primarySpan() - 1 - gap_min)
              : Rect(gap_min, 0, gap_max, height() - 1);
      canvas.fillRect(gap_rect, canvas.bgcolor());
    }
  }
  canvas.drawTiled(handle, handle_tile_bounds, kNoAlign);
}

void Slider::paintStops(const Canvas& canvas, Clipper& clipper,
                        const PaintContext& context) const {
  if (!ShouldRenderStops(*this) || width() <= 0 || height() <= 0 ||
      context.segment_count == 0) {
    return;
  }

  Rect reserved_icon_rect = trackIconReservedRect(context);
  StopRun runs[3];
  int32_t stop_count =
      (int32_t)lroundf((range_.to - range_.from) / range_.step);
  for (int32_t i = 0; i <= stop_count; ++i) {
    float value = (i == stop_count) ? range_.to : range_.from + i * range_.step;
    uint16_t pos = internal::SliderPosFromValue(range_.from, range_.to, value);
    if (pos == getPos()) continue;
    float primary_center = context.axis.centerFromPos(pos);
    for (int segment_index = 0; segment_index < context.segment_count;
         ++segment_index) {
      if (!SegmentContains(context.segments[segment_index], primary_center)) {
        continue;
      }
      auto stop = SmoothFilledCircle(
          StopMarkCenter(context.axis, primary_center), kStopMarkRadiusPixels,
          context.segments[segment_index].stop_color);
      if (!reserved_icon_rect.empty() &&
          reserved_icon_rect.intersects(stop.extents())) {
        break;
      }
      IncludeStopExtentsInRun(context.axis, stop.extents(),
                              runs[segment_index]);
      break;
    }
  }

  for (int segment_index = 0; segment_index < context.segment_count;
       ++segment_index) {
    if (!runs[segment_index].has_marks) continue;
    roo_display::Box segment_clip_box =
        StopSegmentClipBox(context.axis, context.segments[segment_index]);
    if (segment_clip_box.empty()) continue;
    int16_t cross_start = (context.axis.crossSpan() - kStopMarkSpanPixels) / 2;
    roo_display::Box run_box = context.axis.boxFromPrimaryCross(
        runs[segment_index].min_primary, cross_start,
        runs[segment_index].max_primary, cross_start + kStopMarkSpanPixels - 1);
    run_box = roo_display::Box::Intersect(run_box, segment_clip_box);
    if (run_box.empty()) continue;

    Canvas stop_canvas = canvas;
    stop_canvas.set_bgcolor(context.segments[segment_index].track_color);
    stop_canvas.clip(segment_clip_box.translate(canvas.dx(), canvas.dy()));
    bool has_previous_stop = false;
    StopSpan previous_span{0, -1};
    for (int32_t i = 0; i <= stop_count; ++i) {
      float value =
          (i == stop_count) ? range_.to : range_.from + i * range_.step;
      uint16_t pos =
          internal::SliderPosFromValue(range_.from, range_.to, value);
      if (pos == getPos()) continue;
      float primary_center = context.axis.centerFromPos(pos);
      if (!SegmentContains(context.segments[segment_index], primary_center)) {
        continue;
      }
      auto stop = SmoothFilledCircle(
          StopMarkCenter(context.axis, primary_center), kStopMarkRadiusPixels,
          context.segments[segment_index].stop_color);
      if (!reserved_icon_rect.empty() &&
          reserved_icon_rect.intersects(stop.extents())) {
        continue;
      }
      StopSpan current_span =
          context.axis.isVertical()
              ? StopSpan{(int16_t)(context.axis.primarySpan() - 1 -
                                   stop.extents().yMax()),
                         (int16_t)(context.axis.primarySpan() - 1 -
                                   stop.extents().yMin())}
              : StopSpan{stop.extents().xMin(), stop.extents().xMax()};
      if (has_previous_stop) {
        int16_t gap_min_primary = previous_span.max_primary + 1;
        int16_t gap_max_primary = current_span.min_primary - 1;
        if (gap_max_primary >= gap_min_primary) {
          stop_canvas.fillRect(
              context.axis.boxFromPrimaryCross(
                  gap_min_primary, cross_start, gap_max_primary,
                  cross_start + kStopMarkSpanPixels - 1),
              context.segments[segment_index].track_color);
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
  if (style_.orientation == SliderOrientation::kVertical) {
    return roo_display::FpPoint{axis.centeredCross(),
                                axis.displayCenterFromPos(getPos())};
  }
  return roo_display::FpPoint{axis.displayCenterFromPos(getPos()),
                              axis.centeredCross()};
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
                                  SliderTrackIconAnchor anchor) {
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
  const internal::SliderSizeMetrics& size_metrics =
      internal::ResolveSliderSizeMetrics(style_.size);
  Rect bubble_local = ValueIndicatorBubble::ConservativeBounds(
      width(), height(), size_metrics.handle_width, style_.value_indicator,
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
  // dirtied a tight rectangle (set by invalidatePosChange()), narrow the
  // canvas clip to that rectangle so paint() does not redraw the entire
  // envelope. isInvalidated() being true means the framework asked us to
  // fully repaint (e.g. style change, visibility toggle), in which case
  // we keep the full clip.
  internal::DirtySpan pending_content_span = pending_content_dirty_span_;
  internal::DirtySpan pending_indicator_span = pending_indicator_dirty_span_;
  pending_content_dirty_span_ = internal::DirtySpan();

  Rect pending_content = internal::ContentRectFromDisplayMainSpan(
      pending_content_span, width(), height(),
      style_.orientation == SliderOrientation::kVertical);
  Rect pending_indicator = IndicatorDirtyRectFromSpan(
      pending_indicator_span, width(), height(), style_);

  internal::DirtySpan current_indicator_span = pending_indicator_span;

  auto paint_indicator = [&](const Canvas& indicator_canvas) {
    if (!ShowsValueIndicator(*this) || width() <= 0 || height() <= 0) return;
    // Pre-paint the value indicator bubble BEFORE the slider's track and
    // thumb. This way, even if the bubble's geometry overlapped the slider's
    // rectangular bounds (it does not today, but might if kGap or kPaddingV
    // were tuned aggressively), the slider would still appear behind the
    // bubble: subsequent slider writes inside the bubble's inscribed inner
    // rectangle are blocked by the clipper exclusion installed by decorate(),
    // and writes in the bubble's rounded-corner strips have the bubble's
    // decoration overlay alpha-composited on top.
    //
    // The canvas received here has been shifted to slider-local coordinates
    // and clipped to maxBounds(). Because Widget::getVisualBounds() (the
    // default for maxBounds()) unions in getTransientPaintBounds(), and our
    // getParentTransientPaintBounds() override already accounts for the
    // bubble's conservative envelope when the indicator is showing, the
    // canvas clip already covers the bubble area.
    char scratch[64];
    roo::string_view text = formatLabel(value_, scratch, sizeof(scratch));

    const Theme& th = theme();
    Color bubble_color =
        isEnabled() ? th.color.inverseSurface : th.color.onSurface.withA(0x3D);
    Color text_color =
        isEnabled() ? th.color.inverseOnSurface : th.color.surface;

    internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
    float thumb_center = axis.displayCenterFromPos(getPos());
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

  bool invalidated = isInvalidated();

  if (invalidated || !pending_indicator.empty()) {
    Canvas indicator_canvas = canvas;
    if (!invalidated) {
      indicator_canvas.clipToExtents(pending_indicator);
    }
    paint_indicator(indicator_canvas);
  }

  Canvas content_canvas = prepareContentsCanvas(canvas);
  if (!invalidated && !pending_content.empty()) {
    content_canvas.clipToExtents(pending_content);
  }
  if (!content_canvas.clip_box().empty()) {
    clipper.setBounds(content_canvas.clip_box());
    PaintContext context = buildPaintContext();
    paintTrackIcons(content_canvas, clipper, context);
    paintStops(content_canvas, clipper, context);
    paintTrackAndThumb(content_canvas, context);
  }
  pending_indicator_dirty_span_ = ShowsValueIndicator(*this)
                                      ? current_indicator_span
                                      : internal::DirtySpan();
  markClean();
}

}  // namespace material3
}  // namespace roo_windows
