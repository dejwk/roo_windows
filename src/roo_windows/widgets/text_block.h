#pragma once

#include <cstdint>
#include <vector>

#include "roo_backport/string_view.h"
#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

enum class TextWrapMode {
  kNoWrap,
  kWordWrap,
};

enum class TextAlign {
  kStart,
  kCenter,
  kEnd,
  kJustify,
};

// Multi-line block of text. Supports explicit '\n' line breaks and optional
// automatic wrapping.
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

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  const std::string& content() const { return value_; }

  const std::string& text() const { return value_; }

  void setText(std::string value);

  void setContent(std::string value) { setText(std::move(value)); }

  void setColor(roo_display::Color color);

  void setAlignment(roo_display::Alignment alignment);

  void setWrapMode(TextWrapMode wrap_mode);

  void setTextAlign(TextAlign text_align);

  void setMaxLines(uint16_t max_lines);

  void setEllipsize(bool ellipsize);

  TextWrapMode wrapMode() const { return wrap_mode_; }

  TextAlign textAlign() const { return text_align_; }

  uint16_t maxLines() const { return max_lines_; }

  bool ellipsize() const { return ellipsize_; }

  const roo_display::Font& font() const { return font_; }

  struct LineLayout {
    roo::string_view text;
    uint8_t ellipsis_chars;
    uint16_t spaces;
    int16_t width;
    bool ends_paragraph;
  };

 private:
  void invalidateLayoutCache();

  void recalculateNaturalDimensions();

  void ensureLayout(XDim width_limit) const;

  std::string value_;
  Dimensions text_dims_;
  mutable Dimensions layout_dims_;
  mutable std::vector<LineLayout> layout_lines_;
  mutable XDim layout_width_limit_;
  mutable bool layout_valid_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  roo_display::Alignment alignment_;
  TextWrapMode wrap_mode_;
  TextAlign text_align_;
  uint16_t max_lines_;
  bool ellipsize_;
};

}  // namespace roo_windows
