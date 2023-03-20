#pragma once

#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class TextLabel : public BasicWidget {
 public:
  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font);

  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Alignment alignment);

  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Color color,
            roo_display::Alignment alignment);

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  const std::string& content() const { return value_; }

  void setText(std::string value);

  void setText(const char* value);

  void setText(roo_display::StringView value);

  void setTextf(const char* format, ...);

  void setTextvf(const char* format, va_list arg);

  const roo_display::Font& font() const { return font_; }

 private:
  std::string value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows
