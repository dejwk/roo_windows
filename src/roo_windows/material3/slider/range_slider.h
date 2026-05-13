#pragma once

#include <stddef.h>

#include "roo_backport/string_view.h"
#include "roo_windows/material3/slider/slider.h"

namespace roo_windows {
namespace material3 {

namespace internal {
class SliderAxisMetrics;
}  // namespace internal

// Supports both horizontal and vertical range selection. Material 3 guidance
// discourages the vertical range variant, but it is kept for API completeness.
class RangeSlider : public BasicWidget {
 public:
  RangeSlider(const Environment& env, SliderRange range, float start_value,
              float end_value, SliderStyle style = {});

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override;

  OverlayType getOverlayType() const override { return OVERLAY_NONE; }

  roo_display::FpPoint getPointOverlayFocus() const override;

  bool useOverlayOnPress() const override { return false; }

  bool showClickAnimation() const override { return false; }

  bool isClickable() const override { return true; }

  bool onDown(XDim x, YDim y) override;

  bool onSingleTapUp(XDim x, YDim y) override;

  void onShowPress(XDim x, YDim y) override;

  bool supportsScrolling() const override { return true; }

  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  bool onTouchUp(XDim x, YDim y) override;

  void onCancel() override;

  ColorRole effectiveContainerRole() const override;

  Rect getSloppyTouchParentBounds() const override;

  Rect getParentTransientPaintBounds() const override;

  const SliderRange& range() const { return range_; }

  bool setRange(SliderRange range);

  const SliderStyle& style() const { return style_; }

  bool setStyle(SliderStyle style);

  float minSeparation() const { return min_separation_; }

  bool setMinSeparation(float value);

  float startValue() const { return start_value_; }

  float endValue() const { return end_value_; }

  bool setValues(float start_value, float end_value);

  int activeThumbIndex() const {
    return active_thumb_ >= 0 ? active_thumb_ : -1;
  }

  virtual void onValueChange(float start, float end, int active_thumb,
                             bool from_user) {}

  virtual void onInteractionStart(int active_thumb) {}

  virtual void onInteractionEnd(float start, float end) {}

  // Format a numeric slider value into a short human-readable label. Writes
  // into the caller-provided scratch buffer; must not allocate. The default
  // formatter renders a compact decimal. Override to customize.
  virtual roo::string_view formatLabel(float value, char* scratch,
                                       size_t scratch_size) const;

 protected:
  void paintWidgetContents(const Canvas& s, Clipper& clipper) override;
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  bool setValuesInternal(float start_value, float end_value, bool from_user,
                         int active_thumb);

  bool setActiveThumbPos(uint16_t pos);

  // Marks the minimal local-coord region that needs to be redrawn for a
  // value change between (old_start_pos, old_end_pos) and
  // (new_start_pos, new_end_pos). See implementation for details.
  void invalidateValueChange(const internal::SliderAxisMetrics& axis,
                             uint16_t old_start_pos, uint16_t old_end_pos,
                             uint16_t new_start_pos, uint16_t new_end_pos,
                             float new_start_value, float new_end_value);

  SliderRange range_;
  SliderStyle style_;
  float start_value_;
  float end_value_;
  float min_separation_;
  int8_t active_thumb_;
  int8_t overlay_thumb_;
  bool is_dragging_;
  bool awaiting_direction_;
  // Tight bounding span (widget-local, within bounds()) of the slider track
  // and thumbs area that needs repainting due to a value/style state change.
  internal::DirtySpan pending_content_dirty_span_;
  // Indicator span state. When the range slider is clean, this stores the
  // current on-screen indicator footprint. Once the range slider is dirty, the
  // same field is reused as the indicator repaint span so coalesced updates
  // can union the next bubble bounds against the previously shown footprint
  // without a second cached span.
  internal::DirtySpan pending_indicator_dirty_span_;
};

}  // namespace material3
}  // namespace roo_windows
