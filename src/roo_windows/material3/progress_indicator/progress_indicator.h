#pragma once

#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/widget.h"

namespace roo_windows::material3 {

/// Selects known progress or unknown-duration activity.
enum class ProgressIndicatorMode : uint8_t { kDeterminate, kIndeterminate };

/// Passive foreground indicator; mutations belong on the UI thread.
class ProgressIndicator : public Widget {
 public:
  ProgressIndicator(const ProgressIndicator&) = delete;
  ProgressIndicator& operator=(const ProgressIndicator&) = delete;
  ProgressIndicator(ProgressIndicator&&) = delete;
  ProgressIndicator& operator=(ProgressIndicator&&) = delete;

  /// Clamps finite values to [0, 1] and selects determinate mode. Rejects
  /// nonfinite values without changes.
  bool setProgress(float fraction);

  /// Returns the last known fraction, including while indeterminate.
  float progress() const { return fraction_; }

  /// Selects unknown-duration activity, preserving the last known fraction.
  void setIndeterminate();

  /// Returns the current semantic mode.
  ProgressIndicatorMode mode() const {
    return indeterminate_ ? ProgressIndicatorMode::kIndeterminate
                          : ProgressIndicatorMode::kDeterminate;
  }

  /// Enables motion, or selects static indeterminate geometry when disabled.
  void setMotionEnabled(bool enabled);

  /// Returns the explicit component motion policy.
  bool motionEnabled() const { return motion_enabled_; }

 protected:
  explicit ProgressIndicator(ApplicationContext& context, bool circular);
  Rect getDirectPaintExclusionBounds() const override {
    return Rect(0, 0, -1, -1);
  }
  void invalidateInk();
  bool rightToLeft() const { return rtl_; }
  void setRightToLeft(bool rtl);
  uint16_t phaseMillis() const { return phase_ms_; }

 private:
  float fraction_ = 0;
  uint16_t phase_ms_ = 0;
  bool indeterminate_ : 1;
  bool motion_enabled_ : 1;
  bool rtl_ : 1;
  bool circular_ : 1;
};

/// Standard rounded linear Material 3 progress indicator.
class LinearProgressIndicator : public ProgressIndicator {
 public:
  /// Creates a determinate zero indicator with motion enabled and LTR
  /// direction.
  explicit LinearProgressIndicator(ApplicationContext& context);

  /// Mirrors all geometry, including the end stop and moving segments.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns the explicit logical direction.
  LayoutDirection layoutDirection() const;

  /// Registers foreground segments against the ancestor surface.
  void paint(PaintContext& ctx) const override;

  /// Returns a cheap intrinsic size; constraints may shrink it further.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
};

/// Standard clockwise circular Material 3 progress indicator.
class CircularProgressIndicator : public ProgressIndicator {
 public:
  /// Creates a determinate zero indicator with motion enabled.
  explicit CircularProgressIndicator(ApplicationContext& context);

  /// Registers foreground arcs, preserving the transparent center.
  void paint(PaintContext& ctx) const override;

  /// Returns the nominal ring and inset footprint.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
};

}  // namespace roo_windows::material3
