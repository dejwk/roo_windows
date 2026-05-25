#pragma once

#include <string>

#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/border_style.h"

namespace roo_windows {

// Implements basic button functionality: clickability, border, elevations.
class Button : public BasicSurfaceWidget {
 public:
  /// Visual style. `CONTAINED` is filled, `OUTLINED` has a stroked border
  /// over a transparent interior, `TEXT` is borderless and flat.
  enum Style { CONTAINED, OUTLINED, TEXT };

  Style style() const { return style_; }

  /// Overrides the fill color of the button's interior. No-op if the color
  /// is unchanged. Invalidates the interior on change.
  void setInteriorColor(roo_display::Color color) {
    if (interior_color_ == color) return;
    interior_color_ = color;
    invalidateInterior();
  }

  /// Returns the cached outline color (typically only painted when
  /// `style() == OUTLINED`).
  roo_display::Color getOutlineColor() const override { return outline_color_; }

  /// Returns the cached interior fill color used by the surface pipeline.
  Color background() const override { return interior_color_; }

  /// Overrides the outline color used when style() == OUTLINED. No-op if the
  /// color is unchanged. Invalidates the interior on change.
  void setOutlineColor(roo_display::Color color) {
    if (outline_color_ == color) return;
    outline_color_ = color;
    invalidateInterior();
  }

  /// Sets the corner radius (in pixels) used by getBorderStyle().
  void setCornerRadius(uint8_t corner_radius) {
    corner_radius_ = corner_radius;
  }

  Padding getDefaultPadding() const override;

  /// Buttons always opt in as clickable so they receive touch animation and
  /// click dispatch.
  bool isClickable() const override { return true; }

  /// Sets the resting and pressed elevations in dp. The current elevation
  /// follows `isPressed()`.
  void setElevation(uint8_t resting, uint8_t pressed);

  /// Resolves the container color role for the active style (e.g. surface
  /// roles for OUTLINED/TEXT; primary roles for CONTAINED).
  ColorRole containerRole() const override;

  /// Returns `elevation_pressed_` while pressed, otherwise the resting value.
  uint8_t getElevation() const override {
    return isPressed() ? elevation_pressed_ : elevation_resting_;
  }

  /// Builds a BorderStyle from the current corner radius; outline width is
  /// 18/16 dp for OUTLINED and 0 otherwise.
  BorderStyle getBorderStyle() const override {
    return BorderStyle(corner_radius_, style() == OUTLINED
                                           ? SmallNumber::Of16ths(Scaled(18))
                                           : SmallNumber(0));
  }

 protected:
  Button(ApplicationContext& context, Style style = CONTAINED);

  /// Tracks press transitions to swap between resting and pressed elevations.
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  Style style_;
  roo_display::Color outline_color_;
  roo_display::Color interior_color_;

  uint8_t elevation_resting_;
  uint8_t elevation_pressed_;
  uint8_t current_elevation_;
  uint8_t corner_radius_;
};

// Button with a text label, an icon, or both.
class SimpleButton : public Button {
 public:
  SimpleButton(ApplicationContext& context, const MonoIcon& icon,
               Style style = CONTAINED)
      : SimpleButton(context, &icon, "", style) {}

  SimpleButton(ApplicationContext& context, std::string label,
               Style style = CONTAINED)
      : SimpleButton(context, nullptr, label, style) {}

  SimpleButton(ApplicationContext& context, const MonoIcon& icon, std::string label,
               Style style = CONTAINED)
      : SimpleButton(context, &icon, label, style) {}

  Padding getDefaultPadding() const override;

  /// Color used to draw the icon and label. Transparent means the theme's
  /// content color for the active container role.
  roo_display::Color contentColor() const { return content_color_; }

  /// Sets the icon/label foreground color. No-op if unchanged.
  void setContentColor(roo_display::Color color) {
    if (content_color_ == color) return;
    content_color_ = color;
    setDirty();
  }

  bool hasLabel() const { return !label_.empty(); }

  const std::string& label() const { return label_; }

  /// Replaces the displayed label. No-op if unchanged.
  void setLabel(const std::string& label) {
    if (label_ == label) return;
    label_ = label;
    setDirty();
  }

  bool hasIcon() const { return icon_ != nullptr; }

  /// Returns the icon. Must only be called when `hasIcon()` is true.
  const MonoIcon& icon() const { return *icon_; }

  /// Replaces the icon (nullptr removes it). No-op if unchanged.
  void setIcon(const MonoIcon* icon) {
    if (icon_ == icon) return;
    icon_ = icon;
    setDirty();
  }

  /// Sets the font used to render the label. Lifetime: caller-owned.
  void setFont(const roo_display::Font& font) { font_ = &font; }

  const roo_display::Font& getFont() const { return *font_; }

  /// Paints the icon and/or label centered inside the button's content area,
  /// using `contentColor()` (or the theme-resolved fallback).
  void paint(PaintContext& ctx) const override;

  /// Reports the minimum size required to contain the icon and label using
  /// the configured font and the default Material 3 button paddings.
  Dimensions getSuggestedMinimumDimensions() const override;

 private:
  SimpleButton(ApplicationContext& context, const MonoIcon* icon, std::string label,
               Style style);

  roo_display::Color content_color_;
  const roo_display::Font* font_;
  std::string label_;
  const MonoIcon* icon_;
};

}  // namespace roo_windows
