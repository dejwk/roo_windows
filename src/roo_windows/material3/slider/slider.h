#pragma once

#include <stddef.h>
#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_windows/core/basic_widget.h"

namespace roo_windows {
namespace material3 {

namespace internal {
struct SliderAxisMetrics;
}  // namespace internal

enum class SliderVariant : uint8_t {
  kStandard,
  kCentered,
};

enum class SliderOrientation : uint8_t {
  kHorizontal,
  kVertical,
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
  kAuto,
  kHidden,
  kShowStops,
};

// Compact packed style state. Read on every paint, so kept inline. Reserved
// bits leave room for later steps.
struct SliderStyle {
  SliderOrientation orientation : 1;
  SliderSize size : 3;
  SliderValueIndicatorBehavior value_indicator : 2;
  SliderTickMode tick_mode : 2;
  bool show_stop_indicators : 1;

  constexpr SliderStyle()
      : orientation(SliderOrientation::kHorizontal),
        size(SliderSize::kExtraSmall),
        value_indicator(SliderValueIndicatorBehavior::kHidden),
        tick_mode(SliderTickMode::kAuto),
        show_stop_indicators(false) {}

  bool operator==(const SliderStyle& o) const {
    return orientation == o.orientation && size == o.size &&
           value_indicator == o.value_indicator && tick_mode == o.tick_mode &&
           show_stop_indicators == o.show_stop_indicators;
  }
  bool operator!=(const SliderStyle& o) const { return !(*this == o); }
};

struct SliderRange {
  float from = 0.0f;
  float to = 1.0f;
  float step = 0.0f;
};

class Slider : public BasicWidget {
 public:
  explicit Slider(const Environment& env, uint16_t pos = 0);

  Slider(const Environment& env, SliderRange range, float value = 0.0f,
         SliderVariant variant = SliderVariant::kStandard,
         SliderStyle style = {});

  Padding getDefaultPadding() const override { return Padding(0); }
  Margins getDefaultMargins() const override { return Margins(0); }

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override;

  bool useOverlayOnPress() const override { return false; }

  bool showClickAnimation() const override { return false; }

  bool isClickable() const override { return true; }

  bool onDown(XDim x, YDim y) override;

  bool onSingleTapUp(XDim x, YDim y) override;

  void onShowPress(XDim x, YDim y) override;

  bool supportsScrolling() const override { return true; }

  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;

  void onCancel() override;

  roo_display::FpPoint getPointOverlayFocus() const override;

  ColorRole effectiveContainerRole() const override;

  Rect getParentTransientPaintBounds() const override;

  const SliderRange& range() const { return range_; }

  bool setRange(SliderRange range);

  SliderVariant variant() const { return variant_; }

  bool setVariant(SliderVariant variant);

  const SliderStyle& style() const { return style_; }

  bool setStyle(SliderStyle style);

  float value() const { return value_; }

  bool setValue(float value);

  uint16_t getPos() const;

  bool setPos(uint16_t pos);

  virtual void onValueChange(float value, bool from_user) {}

  virtual void onInteractionStart() {}

  virtual void onInteractionEnd(float value) {}

  // Format a numeric slider value into a short, human-readable label. Writes
  // into the caller-provided scratch buffer (which must remain valid for the
  // lifetime of the returned view) and returns a view into it. Must not
  // allocate. Override in a subclass to customize formatting.
  virtual roo::string_view formatLabel(float value, char* scratch,
                                       size_t scratch_size) const;

 protected:
  void paintWidgetContents(const Canvas& s, Clipper& clipper) override;

 private:
  bool setPosInternal(uint16_t pos, bool from_user);

  // Marks the minimal local-coord region that needs to be redrawn for a
  // thumb position change. See implementation for details.
  void invalidatePosChange(const internal::SliderAxisMetrics& axis,
                           uint16_t old_pos, uint16_t new_pos);

  SliderRange range_;
  SliderVariant variant_;
  SliderStyle style_;
  float value_;
  bool is_dragging_;
  // Tight bounding rect (widget-local) of the area that needs repainting
  // due to value/style state changes since the last paint. Used by
  // paintWidgetContents() to narrow the canvas clip when this widget was
  // dirtied without being fully invalidated, so paint() draws only the
  // narrow column around the moving thumb instead of the full conservative
  // envelope reported by getParentTransientPaintBounds(). Reset to empty
  // at the start of each paintWidgetContents() call.
  Rect pending_dirty_rect_;
};

}  // namespace material3
}  // namespace roo_windows
