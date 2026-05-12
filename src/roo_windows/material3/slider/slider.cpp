#include "roo_windows/material3/slider/slider.h"

#include <algorithm>
#include <cmath>

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

internal::SliderAxisMetrics MakeSliderAxisMetrics(const Slider& slider) {
  return internal::SliderAxisMetrics(
      slider.width(), slider.height(), kHandleWidth, kTrackHandleGap,
      slider.style().orientation == SliderOrientation::kVertical);
}

}  // namespace

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
  invalidatePosChange(axis, old_pos, new_pos);
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
    invalidatePosChange(axis, old_pos, new_pos);
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
    invalidatePosChange(axis, old_pos, new_pos);
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
                                 uint16_t old_pos, uint16_t new_pos) {
  Rect thumb_rect = axis.invalidationRectForPosChange(old_pos, new_pos);
  Rect bubble_envelope(0, 0, -1, -1);
  if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
    float c_old = axis.displayCenterFromPos(old_pos);
    float c_new = axis.displayCenterFromPos(new_pos);
    bubble_envelope = ValueIndicatorBubble::EnvelopeForCenterRange(
        width(), height(), std::min(c_old, c_new), std::max(c_old, c_new),
        style_.value_indicator, style_.orientation);
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

namespace {

Rect IndicatorDirtyRectFromSpan(const internal::DirtySpan& span, int16_t width,
                                int16_t height, SliderStyle style) {
  if (span.empty() || width <= 0 || height <= 0) return Rect(0, 0, -1, -1);
  Rect conservative = ValueIndicatorBubble::ConservativeBounds(
      width, height, kHandleWidth, style.value_indicator, style.orientation);
  if (conservative.empty()) return Rect(0, 0, -1, -1);
  if (style.orientation == SliderOrientation::kVertical) {
    return Rect(conservative.xMin(), span.min_coord, conservative.xMax(),
                span.max_coord);
  }
  return Rect(span.min_coord, conservative.yMin(), span.max_coord,
              conservative.yMax());
}

}  // namespace

uint16_t Slider::getPos() const {
  return internal::SliderPosFromValue(range_.from, range_.to, value_);
}

void Slider::paint(const Canvas& canvas) const {
  Tokens tokens = ResolveTokens(*this);
  internal::SliderAxisMetrics axis = MakeSliderAxisMetrics(*this);
  float center_anchor_primary = axis.centerFromPos(32768);
  bool thumb_on_or_right_of_center =
      axis.centerFromPos(getPos()) >= center_anchor_primary;
  int16_t thumb_width = ThumbWidthForState(isPressed());
  int16_t track_gap = TrackGapForThumbWidth(thumb_width);
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis, axis.centerFromPos(getPos()), thumb_width, kTrackHeight, track_gap,
      kHandleHeight);

  float active_track_min_primary = -0.5f;
  float active_track_max_primary = layout.active_track_max_primary;
  roo_display::Box active_clip = axis.boxFromPrimaryCross(
      0, layout.track_cross_start, layout.activeClipMax(axis.primarySpan()),
      layout.track_cross_start + kTrackHeight - 1);
  bool has_left_inactive_clip = false;
  bool has_right_inactive_clip = true;
  roo_display::Box left_inactive_clip;
  roo_display::Box right_inactive_clip = axis.boxFromPrimaryCross(
      (int16_t)ceilf(layout.inactive_track_min_primary),
      layout.track_cross_start, axis.primarySpan() - 1,
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
    if (active_clip_max >= axis.primarySpan()) {
      active_clip_max = axis.primarySpan() - 1;
    }
    active_clip = axis.boxFromPrimaryCross(
        active_clip_min, layout.track_cross_start, active_clip_max,
        layout.track_cross_start + kTrackHeight - 1);

    has_left_inactive_clip = true;
    int16_t left_inactive_max =
        thumb_on_or_right_of_center
            ? (int16_t)floorf(center_anchor_primary)
            : (int16_t)floorf(layout.active_track_max_primary);
    if (left_inactive_max < 0) {
      has_left_inactive_clip = false;
    } else {
      left_inactive_clip = axis.boxFromPrimaryCross(
          0, layout.track_cross_start,
          left_inactive_max >= axis.primarySpan() ? axis.primarySpan() - 1
                                                  : left_inactive_max,
          layout.track_cross_start + kTrackHeight - 1);
    }

    int16_t right_inactive_min =
        thumb_on_or_right_of_center
            ? (int16_t)ceilf(layout.inactive_track_min_primary)
            : (int16_t)ceilf(center_anchor_primary);
    if (right_inactive_min >= axis.primarySpan()) {
      has_right_inactive_clip = false;
    } else {
      if (right_inactive_min < 0) right_inactive_min = 0;
      right_inactive_clip = axis.boxFromPrimaryCross(
          right_inactive_min, layout.track_cross_start, axis.primarySpan() - 1,
          layout.track_cross_start + kTrackHeight - 1);
    }
  } else {
    int16_t right_inactive_min =
        (int16_t)ceilf(layout.inactive_track_min_primary);
    if (right_inactive_min >= axis.primarySpan()) {
      has_right_inactive_clip = false;
    } else {
      if (right_inactive_min < 0) right_inactive_min = 0;
      right_inactive_clip = axis.boxFromPrimaryCross(
          right_inactive_min, layout.track_cross_start, axis.primarySpan() - 1,
          layout.track_cross_start + kTrackHeight - 1);
    }
  }

  auto inactive_track_bounds = axis.paintRectFromPrimaryCross(
      TrackShapeMinPrimary(0.0f), layout.track_min_cross,
      axis.primarySpan() - 0.5f, layout.track_max_cross);
  auto active_track_bounds = axis.paintRectFromPrimaryCross(
      TrackShapeMinPrimary(active_track_min_primary), layout.track_min_cross,
      active_track_max_primary + (float)kTrackRadius, layout.track_max_cross);
  auto handle_bounds = axis.paintRectFromPrimaryCross(
      layout.thumb_min_primary, layout.thumb_min_cross,
      layout.thumb_max_primary, layout.thumb_max_cross);

  auto inactive_track = SmoothFilledRoundRect(
      inactive_track_bounds.x_min, inactive_track_bounds.y_min,
      inactive_track_bounds.x_max, inactive_track_bounds.y_max, kTrackRadius,
      tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      active_track_bounds.x_min, active_track_bounds.y_min,
      active_track_bounds.x_max, active_track_bounds.y_max, kTrackRadius,
      tokens.active_track);
  auto handle = SmoothFilledRoundRect(handle_bounds.x_min, handle_bounds.y_min,
                                      handle_bounds.x_max, handle_bounds.y_max,
                                      kHandleCornerRadius, tokens.handle);
  Rect widget_bounds = bounds();
  Rect handle_tile_bounds = GetHandleTileBounds(widget_bounds, handle.extents(),
                                                track_gap, axis.isVertical());

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
  canvas.drawTiled(handle, handle_tile_bounds, kNoAlign);
}

Dimensions Slider::getSuggestedMinimumDimensions() const {
  return Dimensions(kHandleHeight, kHandleHeight);
}

PreferredSize Slider::getPreferredSize() const {
  if (style_.orientation == SliderOrientation::kVertical) {
    return PreferredSize(PreferredSize::ExactWidth(kHandleHeight),
                         PreferredSize::MatchParentHeight());
  }
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::ExactHeight(kHandleHeight));
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

roo::string_view Slider::formatLabel(float value, char* scratch,
                                     size_t scratch_size) const {
  return ValueIndicatorBubble::FormatDefault(value, scratch, scratch_size);
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
      width(), height(), kHandleWidth, style_.value_indicator,
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
  pending_indicator_dirty_span_ = internal::DirtySpan();

  Rect pending_content = internal::ContentRectFromDisplayMainSpan(
      pending_content_span, width(), height(),
      style_.orientation == SliderOrientation::kVertical);
  Rect pending_indicator = IndicatorDirtyRectFromSpan(
      pending_indicator_span, width(), height(), style_);

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
      paint(content_canvas);
    }
    markClean();
    return;
  }

  Canvas my_canvas = canvas;
  paint_indicator(my_canvas);

  // Hand off to the standard contents-paint path, which clips to
  // getContentBounds(), invokes paint() (the slider's track + thumb), and
  // marks the slider clean. The slider's writes flow through the clipper's
  // overlay/exclusion stack, so any pixels it would otherwise put inside
  // the bubble's inscribed rectangle are dropped, and any pixels under the
  // bubble's rounded mask have the bubble color alpha-composited on top.
  BasicWidget::paintWidgetContents(my_canvas, clipper);
}

}  // namespace material3
}  // namespace roo_windows
