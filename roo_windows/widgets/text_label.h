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
            const roo_display::Font& font, roo_display::Alignment alignment)
      : TextLabel(env, std::move(value), font, roo_display::color::Transparent,
                  alignment) {}

  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Color color,
            roo_display::Alignment alignment)
      : Widget(env),
        value_(std::move(value)),
        font_(font),
        color_(color),
        alignment_(alignment) {}

  bool paint(const roo_display::Surface& s) override {
    roo_display::Color color =
        color_.a() == 0 ? parent()->defaultColor() : color_;
    s.drawObject(roo_display::MakeTileOf(
        roo_display::StringViewLabel(value_, font_, color), bounds(),
        alignment_));
    return true;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    // NOTE: we could consider pre-calculating and storing these (and avoid
    // re-measuring in paint), but it is an extra 20 bytes per label so it is
    // not a clear win.
    auto metrics = font_.getHorizontalStringMetrics(value_);
    return Dimensions(metrics.width(), metrics.height());
  }

  const std::string& content() const { return value_; }

  void setContent(std::string value) {
    if (value_ == value) return;
    value_ = std::move(value);
    markDirty();
    requestLayout();
  }

  const roo_display::Font& font() const { return font_; }

 private:
  std::string value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows
