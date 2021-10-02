#pragma once

#include <string>

#include "roo_windows/widget.h"

namespace roo_windows {

class Button : public Widget {
 public:
  enum Style { CONTAINED, OUTLINED, TEXT };

  Button(Panel* parent, const Box& bounds, const MonoIcon& icon,
         Style style = CONTAINED)
      : Button(parent, bounds, &icon, "", style) {}

  Button(Panel* parent, const Box& bounds, std::string label,
         Style style = CONTAINED)
      : Button(parent, bounds, nullptr, label, style) {}

  Button(Panel* parent, const Box& bounds, const MonoIcon& icon,
         std::string label, Style style = CONTAINED)
      : Button(parent, bounds, &icon, label, style) {}

  bool hasLabel() const { return !label_.empty(); }
  const std::string& label() const { return label_; }

  bool hasIcon() const { return icon_ != nullptr; }

  // Must not be called if hasIcon() returns true.
  const MonoIcon& icon() const { return *icon_; }

  Style style() const { return style_; }

  roo_display::Color textColor() const { return textColor_; }

  void setTextColor(roo_display::Color color) {
    textColor_ = color;
    markDirty();
  }

  roo_display::Color interiorColor() const { return interiorColor_; }

  void setInteriorColor(roo_display::Color color) {
    interiorColor_ = color;
    markDirty();
  }

  roo_display::Color outlineColor() const { return outlineColor_; }

  void setOutlineColor(roo_display::Color color) {
    outlineColor_ = color;
    markDirty();
  }

  void paint(const Surface& s) override;

  bool isClickable() const override { return true; }

 private:
  Button(Panel* parent, const Box& bounds, const MonoIcon* icon,
         std::string label, Style style);

  Style style_;
  std::string label_;
  const MonoIcon* icon_;
  roo_display::Color outlineColor_;
  roo_display::Color interiorColor_;
  roo_display::Color textColor_;
};

}  // namespace roo_windows