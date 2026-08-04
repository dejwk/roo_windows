#pragma once

#include <stdint.h>

#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/button/button_types.h"

namespace roo_windows {
namespace material3 {

/// Material 3 color treatment for a non-toggle icon button.
enum class IconButtonStyle : uint8_t {
  kStandard,
  kFilled,
  kFilledTonal,
  kOutlined,
};

/// Material 3 horizontal container width for an icon button.
enum class IconButtonWidth : uint8_t {
  kNarrow,
  kUniform,
  kWide,
};

/// Material 3 icon-only action button.
///
/// The icon is borrowed and must outlive this widget. Size, shape, width, and
/// colors resolve from Material 3 tokens without per-instance appearances.
class IconButton : public BasicSurfaceWidget {
 public:
  /// Creates an icon button that borrows `icon`.
  explicit IconButton(ApplicationContext& context, const MonoIcon& icon,
                      IconButtonStyle style = IconButtonStyle::kFilled);

  /// Returns the active color treatment.
  IconButtonStyle style() const { return (IconButtonStyle)style_; }

  /// Changes the color treatment and invalidates the visual surface.
  void setStyle(IconButtonStyle style);

  /// Returns the active expressive size token.
  ButtonSize size() const { return (ButtonSize)size_; }

  /// Changes size and requests layout and repaint.
  void setSize(ButtonSize size);

  /// Returns the resting corner family.
  ButtonShape shape() const { return (ButtonShape)shape_; }

  /// Changes the resting corner family and invalidates the surface.
  void setShape(ButtonShape shape);

  /// Returns the active horizontal width token.
  IconButtonWidth widthMode() const { return (IconButtonWidth)width_mode_; }

  /// Changes width and requests layout and repaint.
  void setWidthMode(IconButtonWidth width_mode);

  /// Returns the borrowed icon.
  const MonoIcon& icon() const { return *icon_; }

  /// Replaces the borrowed icon and requests layout and repaint.
  void setIcon(const MonoIcon& icon);

  /// Returns the resolved icon slot in the widget's parent-local coordinates.
  virtual Rect getIconBounds() const;

  /// Returns token-derived padding around the resolved icon slot.
  Padding getDefaultPadding() const override;

  /// Returns zero implicit outer margins for toolbar and row composition.
  Margins getDefaultMargins() const override { return Margins(0); }

  /// Returns true so the button participates in activation without a callback.
  bool isClickable() const override { return true; }

  /// Returns the semantic container role used by shared interaction overlays.
  ::roo_windows::material3::ColorToken containerRole() const override;

  /// Returns the enabled or disabled container fill.
  Color background() const override;

  /// Returns the outlined-style border color.
  Color getOutlineColor() const override;

  /// Returns resting or pressed-morphed corner and outline geometry.
  BorderStyle getBorderStyle() const override;

  /// Paints the borrowed icon with its resolved Material 3 content color.
  void paint(PaintContext& ctx) const override;

  /// Returns the icon-slot dimensions before padding.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  /// Invalidates shape geometry when pressed state changes.
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  /// Resolves the icon foreground color for the enabled/disabled state.
  Color resolveContentColor() const;

  const MonoIcon* icon_;
  uint8_t style_ : 2;
  uint8_t size_ : 3;
  uint8_t shape_ : 1;
  uint8_t width_mode_ : 2;
};

}  // namespace material3
}  // namespace roo_windows
