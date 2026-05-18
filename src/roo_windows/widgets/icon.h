#pragma once

#include "roo_display/image/image.h"
#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

/// Pictogram widget that draws a single `roo_display::Pictogram` in a
/// configurable color.
///
/// The pictogram pointer is held by reference, so the underlying glyph data
/// must outlive the widget (typically a constexpr/program-memory icon).
class Icon : public BasicWidget {
 public:
  Icon(const Environment& env, Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(nullptr), color_(color) {}

  Icon(const Environment& env, const roo_display::Pictogram& def,
       Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(&def), color_(color) {}

  /// Paints the pictogram centered in the widget bounds, in either the
  /// configured color or the resolved theme default when transparent.
  void paint(const Canvas& canvas) const override;

  /// Returns the currently configured pictogram. Must only be called after
  /// a non-null icon has been set.
  const roo_display::Pictogram& icon() const { return *icon_; }

  /// Replaces the pictogram and invalidates the widget.
  void setIcon(const roo_display::Pictogram& icon);

  /// Reports the difference between the pictogram's ink extents and its
  /// nominal bounds so layout/invalidation accounts for asymmetric glyphs.
  Insets getInkInsets() const override;

  /// Reports the pictogram's nominal width and height.
  Dimensions getSuggestedMinimumDimensions() const override;

  roo_display::Color color() const { return color_; }

  /// Replaces the foreground color used to draw the pictogram. Transparent
  /// means: defer to the theme's content color.
  void setColor(roo_display::Color color);

 private:
  const roo_display::Pictogram* icon_;
  roo_display::Color color_;
};

}  // namespace roo_windows
