#pragma once

#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_backport/string_view.h"
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

  void setText(roo::string_view value);

  void setTextf(const char* format, ...);

  void setTextvf(const char* format, va_list arg);

  const roo_display::Font& font() const { return font_; }

 private:
  std::string value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::Alignment alignment_;
};

class StringViewLabel : public BasicWidget {
 public:
  StringViewLabel(const Environment& env, roo::string_view value,
                  const roo_display::Font& font);

  StringViewLabel(const Environment& env, roo::string_view value,
                  const roo_display::Font& font,
                  roo_display::Alignment alignment);

  StringViewLabel(const Environment& env, roo::string_view value,
                  const roo_display::Font& font, roo_display::Color color,
                  roo_display::Alignment alignment);

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  roo::string_view content() const { return value_; }

  void setText(roo::string_view value);

  const roo_display::Font& font() const { return font_; }

 private:
  roo::string_view value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows
