#pragma once

#include <stddef.h>
#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_display/image/image.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/material3/slider/slider_internal.h"

namespace roo_windows {
namespace material3 {

namespace internal {
struct SliderPaintTokens;
}

enum class SliderVariant : uint8_t {
  kStandard,
  kCentered,
};

enum class SliderOrientation : uint8_t {
  kHorizontal,
  kVertical,
};

enum class SliderDirection : uint8_t {
  kDefault,
  kNormal,
  kInverted,
};

enum class SliderSize : uint8_t {
  kExtraSmall,
  kSmall,
  kMedium,
  kLarge,
  kExtraLarge,
};

enum class SliderValueIndicatorBehavior : uint8_t {
  kHidden,
  kShowOnInteraction,
  kWithinBounds,
  kAlways,
};

enum class SliderTickMode : uint8_t {
  // Ticks follow the slider's default policy for the current variant.
  kAuto,
  // Ticks are never rendered, even when the slider is discrete.
  kHidden,
  // Ticks are rendered for every valid step in a discrete slider.
  kShowTicks,
};

enum class SliderInsetIconAnchor : uint8_t {
  kStart,
  kEnd,
};

// Compact packed style state. Read on every paint, so kept inline.
struct SliderStyle {
  SliderOrientation orientation : 1;
  SliderDirection direction : 2;
  SliderSize size : 3;
  SliderValueIndicatorBehavior value_indicator : 2;
  SliderTickMode tick_mode : 2;
  // Material 3 stop indicators are enabled by default and can remain visible
  // even when tick rendering is hidden.
  bool show_stop_indicators : 1;

  constexpr SliderStyle()
      : orientation(SliderOrientation::kHorizontal),
        direction(SliderDirection::kDefault),
        size(SliderSize::kExtraSmall),
        value_indicator(SliderValueIndicatorBehavior::kHidden),
        tick_mode(SliderTickMode::kAuto),
        show_stop_indicators(true) {}

  bool operator==(const SliderStyle& o) const {
    return orientation == o.orientation && direction == o.direction &&
           size == o.size && value_indicator == o.value_indicator &&
           tick_mode == o.tick_mode &&
           show_stop_indicators == o.show_stop_indicators;
  }
  bool operator!=(const SliderStyle& o) const { return !(*this == o); }
};

constexpr SliderDirection ResolveSliderDirection(SliderStyle style) {
  if (style.direction != SliderDirection::kDefault) {
    return style.direction;
  }
  return style.orientation == SliderOrientation::kVertical
             ? SliderDirection::kInverted
             : SliderDirection::kNormal;
}

constexpr bool IsSliderDirectionInverted(SliderStyle style) {
  return ResolveSliderDirection(style) == SliderDirection::kInverted;
}

struct SliderRange {
  float from = 0.0f;
  float to = 1.0f;
  float step = 0.0f;
};

struct InsetIcon {
  const roo_display::Pictogram* icon = nullptr;
  SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kStart;
};

class Slider : public BasicWidget {
 public:
  /// Creates a single-value Material 3 slider over the supplied semantic value
  /// range.
  Slider(const Environment& env, SliderRange range = {}, float value = 0.0f,
         SliderVariant variant = SliderVariant::kStandard,
         SliderStyle style = {});

  /// Returns the configured semantic value domain and optional step size.
  const SliderRange& range() const { return range_; }

  /// Replaces the value domain and snaps the current value into the new valid
  /// range if needed.
  bool setRange(SliderRange range);

  /// Returns whether the slider paints as standard or centered.
  SliderVariant variant() const { return variant_; }

  /// Switches between standard fill behavior and centered fill behavior.
  bool setVariant(SliderVariant variant);

  /// Returns the packed presentation settings used while painting.
  const SliderStyle& style() const { return style_; }

  /// Applies orientation, direction, size, indicator, and stop/tick
  /// presentation flags.
  bool setStyle(SliderStyle style);

  /// Returns the current semantic value after clamping and snapping.
  float value() const { return value_; }

  /// Sets the semantic value, clamping and snapping it into the current domain.
  bool setValue(float value);

  /// Called after the slider value changes.
  virtual void onValueChange(float value, bool from_user) {}

  /// Called when a user gesture starts controlling the slider.
  virtual void onInteractionStart() {}

  /// Called when a user gesture finishes with the supplied final value.
  virtual void onInteractionEnd(float value) {}

  /// Formats the current value indicator label into the caller-provided
  /// scratch buffer without allocating.
  virtual roo::string_view formatLabel(float value, char* scratch,
                                       size_t scratch_size) const;

  /// Sliders do not add default content padding.
  Padding getDefaultPadding() const override { return Padding(0); }

  /// Sliders do not add default outer margins.
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Paints the slider track and thumb for the current slider state.
  void paint(const Canvas& canvas) const override;

  /// Returns the minimum square footprint required by the active size preset.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Advertises match-parent on the main axis and exact size on the cross axis.
  PreferredSize getPreferredSize() const override;

  /// Sliders paint their own pressed visuals instead of using a framework
  /// overlay.
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }

  /// The slider thumb does not use the generic overlay-on-press treatment.
  bool useOverlayOnPress() const override { return false; }

  /// The slider uses direct pressed-state painting rather than click animation.
  bool showClickAnimation() const override { return false; }

  /// Sliders participate in press handling.
  bool isClickable() const override { return true; }

  /// Accepts pointer-down only while the slider is enabled.
  bool onDown(XDim x, YDim y) override;

  /// Handles tap-to-jump by moving the thumb directly to the tapped position.
  bool onSingleTapUp(XDim x, YDim y) override;

  /// Starts drag interaction when the press lands near the current thumb.
  void onShowPress(XDim x, YDim y) override;

  /// Sliders can capture scroll gestures as drag gestures.
  bool supportsScrolling() const override { return true; }

  /// Converts scroll motion along the slider axis into live value updates.
  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  /// Finalizes drag interaction and consumes the release when the slider owns
  /// it.
  bool onTouchUp(XDim x, YDim y) override;

  /// Cancels any in-progress interaction state.
  void onCancel() override;

  /// Returns the thumb center used by point-style overlays and focus effects.
  roo_display::FpPoint getPointOverlayFocus() const override;

  /// Material 3 sliders use the primary role for container-derived visuals.
  ColorRole effectiveContainerRole() const override;

  /// Extends sloppy-touch reach along the slider's main interaction axis.
  Rect getSloppyTouchParentBounds() const override;

  /// Expands parent invalidation to cover value-indicator overflow when needed.
  Rect getParentTransientPaintBounds() const override;

 protected:
  /// Paints slider content in staged order so bubbles and exclusions settle
  /// correctly.
  void paintWidgetContents(const Canvas& s, Clipper& clipper) override;

  /// Handles pressed-state invalidation for the thumb and value indicator.
  void notifyStateChanged(uint16_t state_diff) override;

  /// Returns an optional inset icon descriptor for subclasses that provide one.
  virtual InsetIcon getInsetIcon() const { return {}; }

 private:
  struct Metrics;

  /// Resolves all per-frame painting metrics for the current thumb state.
  Metrics buildMetrics() const;

  /// Resolves all per-frame painting metrics for an arbitrary thumb center.
  Metrics buildMetrics(float thumb_center, bool pressed) const;

  /// Returns the exact local bounds of the inset icon if one can be painted.
  Rect insetIconRect(const Metrics& metrics) const;

  /// Returns the icon bounds plus the stop-free buffer reserved around it.
  Rect insetIconReservedRect(const Metrics& metrics) const;

  /// Returns the icon repaint envelope, including stop-mark radius padding.
  Rect insetIconDirtyRect(const Metrics& metrics) const;

  /// Paints the active/inactive track pieces and the current thumb geometry.
  void paintTrackAndThumb(const Canvas& canvas, const Metrics& metrics,
                          const internal::SliderPaintTokens& tokens) const;

  /// Paints any inset icon before track segments exclude the same pixels.
  void paintInsetIcons(const Canvas& canvas, Clipper& clipper,
                       const Metrics& metrics,
                       const internal::SliderPaintTokens& tokens) const;

  /// Paints discrete stop marks and excludes them from later track redraw.
  void paintStops(const Canvas& canvas, Clipper& clipper,
                  const Metrics& metrics,
                  const internal::SliderPaintTokens& tokens) const;

  /// Updates the semantic value (clamped/snapped against the current range)
  /// and triggers the necessary invalidation, flagging whether the change came
  /// from a user gesture.
  bool setValueInternal(float value, bool from_user);

  /// Marks the minimal local repaint envelope needed for a thumb move between
  /// two centers in logical primary-axis space.
  void invalidateValueChange(const internal::SliderAxisMetrics& axis,
                             float old_center, float new_center,
                             float new_value);

  SliderRange range_;
  SliderVariant variant_;
  SliderStyle style_;
  float value_;
  bool is_dragging_;
  // Tight bounding span (widget-local, within bounds()) of the slider track
  // and thumb area that needs repainting due to a value/style state change.
  // Used by paintWidgetContents() to narrow the clip passed to paint().
  internal::DirtySpan pending_content_dirty_span_;
  // Indicator span state. When the slider is clean, this stores the current
  // on-screen indicator footprint. Once the slider is dirty, the same field is
  // reused as the indicator repaint span so coalesced updates can union the
  // next bubble bounds against the previously shown footprint without a second
  // cached span.
  internal::DirtySpan pending_indicator_dirty_span_;
};

class SliderWithInsetIcon : public Slider {
 public:
  /// Inherits Slider constructors.
  using Slider::Slider;

  /// Configures the non-owning inset icon painted inside the track.
  void setIcon(const roo_display::Pictogram* icon,
               SliderInsetIconAnchor anchor = SliderInsetIconAnchor::kStart);

 protected:
  /// Supplies the icon descriptor consumed by Slider's shared painting path.
  InsetIcon getInsetIcon() const override { return icon_; }

 private:
  InsetIcon icon_;
};

}  // namespace material3
}  // namespace roo_windows
