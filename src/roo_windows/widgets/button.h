#pragma once

#include <string>

#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/border_style.h"

namespace roo_windows {

// Implements basic button functionality: clickability, border, elevations.
class Button : public BasicWidget {
 public:
  enum Style { CONTAINED, OUTLINED, TEXT };

  Style style() const { return style_; }

  void setInteriorColor(roo_display::Color color) {
    if (interior_color_ == color) return;
    interior_color_ = color;
    invalidateInterior();
  }

  roo_display::Color getOutlineColor() const override { return outline_color_; }
  Color background() const override { return interior_color_; }

  void setOutlineColor(roo_display::Color color) {
    if (outline_color_ == color) return;
    outline_color_ = color;
    invalidateInterior();
  }

  void setCornerRadius(uint8_t corner_radius) {
    corner_radius_ = corner_radius;
  }

  Padding getDefaultPadding() const override;

  bool isClickable() const override { return true; }

  void setElevation(uint8_t resting, uint8_t pressed);

  uint8_t getElevation() const override {
    return isPressed() ? elevation_pressed_ : elevation_resting_;
  }

  BorderStyle getBorderStyle() const override {
    return BorderStyle(corner_radius_, style() == OUTLINED
                                           ? SmallNumber::Of16ths(Scaled(18))
                                           : SmallNumber(0));
  }

 protected:
  Button(const Environment& env, Style style = CONTAINED);

 private:
  Style style_;
  roo_display::Color outline_color_;
  roo_display::Color interior_color_;

  uint8_t elevation_resting_;
  uint8_t elevation_pressed_;
  uint8_t corner_radius_;
};

// Button with a text label, an icon, or both.
class SimpleButton : public Button {
 public:
  SimpleButton(const Environment& env, const MonoIcon& icon,
               Style style = CONTAINED)
      : SimpleButton(env, &icon, "", style) {}

  SimpleButton(const Environment& env, std::string label,
               Style style = CONTAINED)
      : SimpleButton(env, nullptr, label, style) {}

  SimpleButton(const Environment& env, const MonoIcon& icon, std::string label,
               Style style = CONTAINED)
      : SimpleButton(env, &icon, label, style) {}

  Padding getDefaultPadding() const override;

  roo_display::Color contentColor() const { return content_color_; }

  void setContentColor(roo_display::Color color) {
    if (content_color_ == color) return;
    content_color_ = color;
    setDirty();
  }

  bool hasLabel() const { return !label_.empty(); }

  const std::string& label() const { return label_; }

  void setLabel(const std::string& label) {
    if (label_ == label) return;
    label_ = label;
    setDirty();
  }

  bool hasIcon() const { return icon_ != nullptr; }

  // Must not be called if hasIcon() returns true.
  const MonoIcon& icon() const { return *icon_; }

  void setIcon(const MonoIcon* icon) {
    if (icon_ == icon) return;
    icon_ = icon;
    setDirty();
  }

  void setFont(const roo_display::Font& font) { font_ = &font; }

  const roo_display::Font& getFont() const { return *font_; }

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

 private:
  SimpleButton(const Environment& env, const MonoIcon* icon, std::string label,
               Style style);

  roo_display::Color content_color_;
  const roo_display::Font* font_;
  std::string label_;
  const MonoIcon* icon_;
};

}  // namespace roo_windows
