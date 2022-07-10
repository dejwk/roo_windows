#pragma once

#include "roo_display/core/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"

#include "roo_windows/core/panel.h"

namespace roo_windows {

class TextLabel : public Widget {
 public:
  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::HAlign halign,
            roo_display::VAlign valign)
      : TextLabel(env, value, font, roo_display::color::Transparent, halign,
                  valign) {}

  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Color color,
            roo_display::HAlign halign, roo_display::VAlign valign)
      : Widget(env),
        value_(std::move(value)),
        font_(font),
        color_(color),
        halign_(halign),
        valign_(valign) {}

  bool paint(const roo_display::Surface& s) override {
    roo_display::Color color =
        color_.a() == 0 ? parent()->defaultColor() : color_;
    s.drawObject(
        roo_display::MakeTileOf(roo_display::TextLabel(font_, value_, color),
                                bounds(), halign_, valign_));
    return true;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    auto metrics = font_.getHorizontalStringMetrics(
        (const uint8_t*)value_.c_str(), value_.size());
    return Dimensions(metrics.width(), metrics.height());
  }

  const std::string& content() const { return value_; }

  void setContent(std::string value) {
    if (value_ == value) return;
    value_ = std::move(value);
    markDirty();
  }

  const roo_display::Font& font() const { return font_; }

 private:
  std::string value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::HAlign halign_;
  roo_display::VAlign valign_;
};

}  // namespace roo_windows
