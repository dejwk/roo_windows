#pragma once

#include "roo_display/core/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/tile.h"

namespace roo_windows {

class TextLabel : public Widget {
 public:
  TextLabel(Panel* parent, Box bounds, std::string value, const roo_display::Font& font,
            roo_display::Color color, roo_display::HAlign halign,
            roo_display::VAlign valign)
      : Widget(parent, bounds),
        value_(std::move(value)),
        font_(font),
        color_(color),
        halign_(halign),
        valign_(valign) {}

  void paint(const roo_display::Surface& s) override {
    s.drawObject(roo_display::MakeTileOf(
      roo_display::TextLabel(font_, value_, color_), bounds(), halign_, valign_));
  }

  void setContent(std::string value) {
    if (value_ == value) return;
    value_ = value;
    markDirty();
  }

 private:
  std::string value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::HAlign halign_;
  roo_display::VAlign valign_;
};

}  // namespace roo_windows
