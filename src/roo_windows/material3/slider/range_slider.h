#pragma once

#include <stddef.h>

#include "roo_backport/string_view.h"
#include "roo_windows/material3/slider/slider.h"

namespace roo_windows {
namespace material3 {

namespace internal {
class SliderAxisMetrics;
struct SliderPaintTokens;
}  // namespace internal

// Supports both horizontal and vertical range selection. Material 3 guidance
// discourages the vertical range variant, but it is kept for API completeness.
class RangeSlider : public BasicWidget {
 public:
  /// Creates a two-thumb slider over the supplied semantic value range.
  RangeSlider(const Environment& env, SliderRange range, float start_value,
              float end_value, SliderStyle style = {});

  /// Returns the configured semantic value domain and optional step size.
  const SliderRange& range() const { return range_; }

  /// Replaces the value domain, clamps the minimum-separation constraint, and
  /// snaps both thumbs into the new valid configuration.
  bool setRange(SliderRange range);

  /// Returns the packed presentation settings used while painting.
  const SliderStyle& style() const { return style_; }

  /// Applies orientation, size, indicator, and stop/tick presentation flags.
  bool setStyle(SliderStyle style);

  /// Returns the minimum allowed distance between the two semantic values.
  float minSeparation() const { return min_separation_; }

  /// Enforces a minimum semantic distance between the thumbs, clamping it to
  /// the configured value domain.
  bool setMinSeparation(float value);

  /// Returns the current lower value after ordering, clamping, and snapping.
  float startValue() const { return start_value_; }

  /// Returns the current upper value after ordering, clamping, and snapping.
  float endValue() const { return end_value_; }

  /// Sets both semantic thumb values at once, preserving ordering and minimum
  /// separation.
  bool setValues(float start_value, float end_value);

  /// Returns the currently active thumb index, or -1 when no thumb is active.
  int activeThumbIndex() const {
    return active_thumb_ >= 0 ? active_thumb_ : -1;
  }

  /// Called after either thumb value changes.
  virtual void onValueChange(float start, float end, int active_thumb,
                             bool from_user) {}

  /// Called when a user gesture starts controlling one of the thumbs.
  virtual void onInteractionStart(int active_thumb) {}

  /// Called when a user gesture finishes with the supplied final values.
  virtual void onInteractionEnd(float start, float end) {}

  /// Formats the active thumb's value-indicator label into the caller-provided
  /// scratch buffer without allocating.
  virtual roo::string_view formatLabel(float value, char* scratch,
                                       size_t scratch_size) const;

  /// Range sliders do not add default content padding.
  Padding getDefaultPadding() const override { return Padding(0); }

  /// Range sliders do not add default outer margins.
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Paints the range track and both thumbs for the current slider state.
  void paint(const Canvas& canvas) const override;

  /// Returns the minimum square footprint required by the active size preset.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Advertises match-parent on the main axis and exact size on the cross axis.
  PreferredSize getPreferredSize() const override;

  /// Range sliders paint their own pressed visuals instead of using an overlay.
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }

  /// Returns the thumb center used by point-style overlays and focus effects.
  roo_display::FpPoint getPointOverlayFocus() const override;

  /// The slider thumb does not use the generic overlay-on-press treatment.
  bool useOverlayOnPress() const override { return false; }

  /// The slider uses direct pressed-state painting rather than click animation.
  bool showClickAnimation() const override { return false; }

  /// Range sliders participate in press handling.
  bool isClickable() const override { return true; }

  /// Accepts pointer-down only while the slider is enabled.
  bool onDown(XDim x, YDim y) override;

  /// Handles tap-to-jump by choosing the nearer thumb and moving it.
  bool onSingleTapUp(XDim x, YDim y) override;

  /// Starts drag interaction when the press lands near one or both thumbs.
  void onShowPress(XDim x, YDim y) override;

  /// Range sliders can capture scroll gestures as drag gestures.
  bool supportsScrolling() const override { return true; }

  /// Converts scroll motion along the slider axis into live thumb updates.
  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  /// Finalizes drag interaction and consumes the release when the slider owns
  /// it.
  bool onTouchUp(XDim x, YDim y) override;

  /// Cancels any in-progress interaction state.
  void onCancel() override;

  /// Material 3 range sliders use the primary role for container-derived
  /// visuals.
  ColorRole effectiveContainerRole() const override;

  /// Extends sloppy-touch reach along the slider's main interaction axis.
  Rect getSloppyTouchParentBounds() const override;

  /// Expands parent invalidation to cover value-indicator overflow when needed.
  Rect getParentTransientPaintBounds() const override;

 protected:
  /// Paints range-slider content in staged order so bubbles and exclusions
  /// settle correctly.
  void paintWidgetContents(const Canvas& s, Clipper& clipper) override;

  /// Handles pressed-state invalidation for the active thumb and indicator.
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  struct Metrics;

  /// Paints the range track and both thumbs for the supplied paint state.
  void paintTrackAndThumb(const Canvas& canvas, const Metrics& metrics,
                          const internal::SliderPaintTokens& tokens) const;

  /// Paints discrete stop marks and excludes them from later track redraw.
  void paintStops(const Canvas& canvas, Clipper& clipper,
                  const Metrics& metrics,
                  const internal::SliderPaintTokens& tokens) const;

  /// Resolves all per-frame painting metrics for the current thumb state.
  Metrics buildMetrics() const;

  /// Applies a two-value update while preserving ordering and min-separation
  /// rules.
  bool setValuesInternal(float start_value, float end_value, bool from_user,
                         int active_thumb);

  /// Moves the currently active thumb using normalized position coordinates.
  bool setActiveThumbPos(uint16_t pos);

  /// Marks the minimal local repaint envelope needed for a two-thumb move.
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
