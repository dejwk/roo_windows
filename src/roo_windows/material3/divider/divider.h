#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows::material3 {

/// A uniform separator over the inherited surface, with no built-in insets.
class HorizontalDivider : public Widget {
 public:
  /// Creates a separator attached to @p context.
  explicit HorizontalDivider(ApplicationContext& context) : Widget(context) {}

  /// Returns live M3 outline-variant ink. Invalidate paint after changing it.
  virtual Color dividerColor() const;

  /// Returns positive physical-pixel thickness, defaulting to scaled 1 dp.
  /// Request layout and invalidate paint after changing an override.
  virtual int16_t dividerThickness() const;

  /// Composes the stripe with inherited background and effects in one pass.
  void paint(PaintContext& ctx) const override;

  /// Reserves the stroke thickness across the stretching axis.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, dividerThickness());
  }

  /// Leaves inset placement to the parent layout.
  Margins getMargins() const override { return Margins(0); }

  /// Stretches along the separator and uses exact stroke thickness across it.
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::ExactHeight(dividerThickness()));
  }
};

/// A uniform separator over the inherited surface, with no built-in insets.
class VerticalDivider : public Widget {
 public:
  /// Creates a separator attached to @p context.
  explicit VerticalDivider(ApplicationContext& context) : Widget(context) {}

  /// Returns live M3 outline-variant ink. Invalidate paint after changing it.
  virtual Color dividerColor() const;

  /// Returns positive physical-pixel thickness, defaulting to scaled 1 dp.
  /// Request layout and invalidate paint after changing an override.
  virtual int16_t dividerThickness() const;

  /// Composes the stripe with inherited background and effects in one pass.
  void paint(PaintContext& ctx) const override;

  /// Reserves the stroke thickness across the stretching axis.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(dividerThickness(), 0);
  }

  /// Leaves inset placement to the parent layout.
  Margins getMargins() const override { return Margins(0); }

  /// Stretches along the separator and uses exact stroke thickness across it.
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(dividerThickness()),
                         PreferredSize::MatchParentHeight());
  }
};

}  // namespace roo_windows::material3
