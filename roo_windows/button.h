#pragma once

#include <string>

#include "roo_windows/widget.h"

namespace roo_windows {

class Button : public Widget {
 public:
  enum Style { CONTAINED, OUTLINED, TEXT };

  Button(Panel* parent, const Box& bounds, std::string label,
         Style style = CONTAINED);

  const std::string& label() const { return label_; }
  Style style() const { return style_; }

  roo_display::Color textColor() const { return textColor_; }

  void setTextColor(roo_display::Color color) { textColor_ = color; }

  roo_display::Color interiorColor() const { return interiorColor_; }

  void setInteriorColor(roo_display::Color color) {
    interiorColor_ = color;
  }

  roo_display::Color outlineColor() const { return outlineColor_; }

  void setOutlineColor(roo_display::Color color) { outlineColor_ = color; }

  void defaultPaint(const Surface& s) override;

  bool isClickable() const override { return true; }

 private:
  Style style_;
  std::string label_;
  roo_display::Color outlineColor_;
  roo_display::Color interiorColor_;
  roo_display::Color textColor_;
};

}  // namespace roo_windows