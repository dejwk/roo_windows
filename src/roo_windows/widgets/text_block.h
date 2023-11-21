#pragma once

#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

// Multi-line block of text. Use the '\n' characters to split the lines.
class TextBlock : public BasicWidget {
 public:
  TextBlock(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Alignment alignment)
      : TextBlock(env, value, font, roo_display::color::Transparent,
                  alignment) {}

  TextBlock(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Color color,
            roo_display::Alignment alignment);

  void paint(const Canvas& s) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  const std::string& content() const { return value_; }

  void setContent(std::string value);
  void setColor(roo_display::Color color);

  const roo_display::Font& font() const { return font_; }

 private:
  std::string value_;
  Dimensions text_dims_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows
