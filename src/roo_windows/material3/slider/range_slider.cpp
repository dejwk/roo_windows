#include "roo_windows/material3/slider/range_slider.h"

#include <algorithm>

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
static constexpr int kInteractionRadius = kPointOverlayDiameter / 2;

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

void NormalizeOrderedValues(const SliderRange& range, float start_value,
                           float end_value, float& normalized_start,
                           float& normalized_end) {
  normalized_start = NormalizeValueForRange(start_value, range);
  normalized_end = NormalizeValueForRange(end_value, range);
  if (normalized_start > normalized_end) {
    std::swap(normalized_start, normalized_end);
  }
}

Rect UnionRects(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

}  // namespace

RangeSlider::RangeSlider(const Environment& env, SliderRange range,
                         float start_value, float end_value)
    : BasicWidget(env),
      range_(NormalizeRangeOrDefault(range)),
      start_value_(range_.from),
      end_value_(range_.to) {
  NormalizeOrderedValues(range_, start_value, end_value, start_value_,
                         end_value_);
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
  NormalizeOrderedValues(range, start_value_, end_value_, new_start_value,
                         new_end_value);

  bool changed = range_.from != range.from || range_.to != range.to ||
                 range_.step != range.step || start_value_ != new_start_value ||
                 end_value_ != new_end_value;
  if (!changed) return false;

  range_ = range;
  start_value_ = new_start_value;
  end_value_ = new_end_value;

  if (width() > 0 && height() > 0) {
    uint16_t new_start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t new_end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    Rect invalidation = UnionRects(
        axis.invalidationRectForPosChange(old_start_pos, new_start_pos),
        axis.invalidationRectForPosChange(old_end_pos, new_end_pos));
    invalidateInterior(invalidation);
  }
  return true;
}

bool RangeSlider::setValues(float start_value, float end_value) {
  float new_start_value;
  float new_end_value;
  NormalizeOrderedValues(range_, start_value, end_value, new_start_value,
                         new_end_value);
  if (start_value_ == new_start_value && end_value_ == new_end_value) {
    return false;
  }

  uint16_t old_start_pos =
      internal::SliderPosFromValue(range_.from, range_.to, start_value_);
  uint16_t old_end_pos =
      internal::SliderPosFromValue(range_.from, range_.to, end_value_);

  start_value_ = new_start_value;
  end_value_ = new_end_value;

  if (width() > 0 && height() > 0) {
    uint16_t new_start_pos =
        internal::SliderPosFromValue(range_.from, range_.to, start_value_);
    uint16_t new_end_pos =
        internal::SliderPosFromValue(range_.from, range_.to, end_value_);
    internal::SliderAxisMetrics axis(width(), height(), kHandleWidth,
                                     kInteractionRadius);
    Rect invalidation = UnionRects(
        axis.invalidationRectForPosChange(old_start_pos, new_start_pos),
        axis.invalidationRectForPosChange(old_end_pos, new_end_pos));
    invalidateInterior(invalidation);
  }
  return true;
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