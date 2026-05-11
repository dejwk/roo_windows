#include "roo_windows/material3/slider/slider.h"

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
      is_dragging_(false) {
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

bool Slider::setPos(uint16_t pos) { return setPosInternal(pos, false); }

bool Slider::setPosInternal(uint16_t pos, bool from_user) {
  uint16_t old_pos = getPos();
  if (pos == old_pos) return false;
  Rect old_bounds = IndicatorEnabled(style_) ? maxParentBounds() : Rect();
  value_ = internal::NormalizeSliderValueForRange(
      internal::SliderValueFromNormalizedPos(range_.from, range_.to, pos),
      range_.from, range_.to, range_.step);
  uint16_t new_pos = getPos();
  if (new_pos == old_pos) return false;
  onValueChange(value_, from_user);
  if (width() <= 0 || height() <= 0) {
    return true;
  }
  internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                   kInteractionRadius);
  invalidateInterior(axis.invalidationRectForPosChange(old_pos, new_pos));
  if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  return true;
}

bool Slider::setRange(SliderRange range) {
  if (!internal::IsValidSliderRange(range.from, range.to, range.step)) {
    return false;
  }
  uint16_t old_pos = getPos();
  Rect old_bounds = IndicatorEnabled(style_) ? maxParentBounds() : Rect();
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
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    invalidateInterior(axis.invalidationRectForPosChange(old_pos, new_pos));
    if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
      notifyParentInvalidatedRegion(
          Rect::Extent(old_bounds, maxParentBounds()));
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

bool Slider::setValue(float value) {
  float new_value = internal::NormalizeSliderValueForRange(
      value, range_.from, range_.to, range_.step);
  if (value_ == new_value) return false;

  uint16_t old_pos = getPos();
  Rect old_bounds = IndicatorEnabled(style_) ? maxParentBounds() : Rect();
  value_ = new_value;
  onValueChange(value_, false);
  uint16_t new_pos = getPos();

  if (width() > 0 && height() > 0 && old_pos != new_pos) {
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    invalidateInterior(axis.invalidationRectForPosChange(old_pos, new_pos));
    if (IndicatorEnabled(style_) && ShowsValueIndicator(*this)) {
      notifyParentInvalidatedRegion(
          Rect::Extent(old_bounds, maxParentBounds()));
    }
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
  bool thumb_on_or_right_of_center =
      axis.centerFromPos(getPos()) >= center_anchor_primary;
  int16_t thumb_width = ThumbWidthForState(isPressed());
  int16_t track_gap = TrackGapForThumbWidth(thumb_width);
  internal::SliderVisualMetrics layout = internal::ResolveSliderVisualMetrics(
      axis, axis.centerFromPos(getPos()), thumb_width, kTrackHeight, track_gap,
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
    int16_t left_inactive_max =
        thumb_on_or_right_of_center
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

    int16_t right_inactive_min =
        thumb_on_or_right_of_center
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
    int16_t right_inactive_min =
        (int16_t)ceilf(layout.inactive_track_min_primary);
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
      TrackShapeMinPrimary(0.0f), layout.track_min_cross, width() - 0.5f,
      layout.track_max_cross, kTrackRadius, tokens.inactive_track);
  auto active_track = SmoothFilledRoundRect(
      TrackShapeMinPrimary(active_track_min_primary), layout.track_min_cross,
      active_track_max_primary + (float)kTrackRadius, layout.track_max_cross,
      kTrackRadius, tokens.active_track);
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
      width(), height(), kHandleWidth, style_.value_indicator);
  if (bubble_local.empty()) return base;
  Rect bubble_parent = bubble_local.translate(offsetLeft(), offsetTop());
  return Rect::Extent(base, bubble_parent);
}

void Slider::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  if (ShowsValueIndicator(*this) && width() > 0 && height() > 0) {
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

    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    float thumb_center = axis.centerFromPos(getPos());
    bool clamp =
        style_.value_indicator == SliderValueIndicatorBehavior::kWithinBounds;

    ValueIndicatorBubble bubble;
    if (bubble.layout(width(), thumb_center, text, clamp, bubble_color,
                      text_color)) {
      bubble.paint(canvas);
      bubble.decorate(canvas, clipper, OverlaySpec());
    }
  }

  // Hand off to the standard contents-paint path, which clips to
  // getContentBounds(), invokes paint() (the slider's track + thumb), and
  // marks the slider clean. The slider's writes flow through the clipper's
  // overlay/exclusion stack, so any pixels it would otherwise put inside
  // the bubble's inscribed rectangle are dropped, and any pixels under the
  // bubble's rounded mask have the bubble color alpha-composited on top.
  BasicWidget::paintWidgetContents(canvas, clipper);
}

}  // namespace material3
}  // namespace roo_windows
