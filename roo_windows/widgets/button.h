#pragma once

#include <string>

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class ButtonBase : public BasicWidget {
 public:
  enum Style { CONTAINED, OUTLINED, TEXT };

  Style style() const { return style_; }

  roo_display::Color interiorColor() const { return interior_color_; }

  void setInteriorColor(roo_display::Color color) {
    if (interior_color_ == color) return;
    interior_color_ = color;
    invalidateInterior();
  }

  roo_display::Color outlineColor() const { return outline_color_; }

  void setOutlineColor(roo_display::Color color) {
    if (outline_color_ == color) return;
    outline_color_ = color;
    invalidateInterior();
  }

  roo_display::Color contentColor() const { return content_color_; }

  void setContentColor(roo_display::Color color) {
    if (content_color_ == color) return;
    content_color_ = color;
    markDirty();
  }

  Color background() const override { return interior_color_; }

  void paint(const Canvas& canvas) const override;

  Padding getDefaultPadding() const override;

  bool isClickable() const override { return true; }

 protected:
  ButtonBase(const Environment& env, Style style = CONTAINED);

  virtual void paintInterior(const Canvas& canvas, Rect bounds) const = 0;

 private:
  Style style_;
  roo_display::Color outline_color_;
  roo_display::Color interior_color_;
  roo_display::Color content_color_;
};

class Button : public ButtonBase {
 public:
  Button(const Environment& env, const MonoIcon& icon, Style style = CONTAINED)
      : Button(env, &icon, "", style) {}

  Button(const Environment& env, std::string label, Style style = CONTAINED)
      : Button(env, nullptr, label, style) {}

  Button(const Environment& env, const MonoIcon& icon, std::string label,
         Style style = CONTAINED)
      : Button(env, &icon, label, style) {}

  bool hasLabel() const { return !label_.empty(); }

  const std::string& label() const { return label_; }

  void setLabel(const std::string& label) {
    if (label_ == label) return;
    label_ = label;
    markDirty();
  }

  bool hasIcon() const { return icon_ != nullptr; }

  // Must not be called if hasIcon() returns true.
  const MonoIcon& icon() const { return *icon_; }

  void setIcon(const MonoIcon* icon) {
    if (icon_ == icon) return;
    icon_ = icon;
    markDirty();
  }

  void setFont(const roo_display::Font& font) { font_ = &font; }

  const roo_display::Font& getFont() const { return *font_; }

  void paintInterior(const Canvas& canvas, Rect bounds) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

 private:
  Button(const Environment& env, const MonoIcon* icon, std::string label,
         Style style);

  const roo_display::Font* font_;
  std::string label_;
  const MonoIcon* icon_;
};

}  // namespace roo_windows
