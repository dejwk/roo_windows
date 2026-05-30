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

  /// Paints all currently laid-out lines using the configured font, color
  /// and alignment, including any trailing ellipsis when text is clipped.
  void paint(PaintContext& ctx) const override;

  /// Reports ink insets matching the smallest rectangle that contains the
  /// rendered glyphs, so the dirty-region tracker only repaints actual text.
  Insets getInkInsets() const override;

  /// Reports the natural unwrapped block size (longest line wide; total
  /// rendered line height tall) as the minimum.
  Dimensions getSuggestedMinimumDimensions() const override;

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  /// Re-runs (and caches) wrapping for the proposed width and reports the
  /// resulting block dimensions.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Confirms the layout for the final width so subsequent paints use the
  /// cached line breaks.
  void onLayout(bool changed, const Rect& rect) override;

  const std::string& content() const { return value_; }

  const std::string& text() const { return value_; }

  /// Replaces the block's text and invalidates the cached layout.
  void setText(std::string value);

  void setContent(std::string value) { setText(std::move(value)); }

  /// Replaces the foreground color used to render the text.
  void setColor(roo_display::Color color);

  /// Sets the global alignment of the block within its widget bounds.
  void setAlignment(roo_display::Alignment alignment);

  /// Switches between wrapping and single-line modes; invalidates layout.
  void setWrapMode(TextWrapMode wrap_mode);

  /// Sets per-line horizontal alignment (start/center/end/justify).
  void setTextAlign(TextAlign text_align);

  /// Caps the number of rendered lines; excess lines are dropped and the
  /// last visible line gets an ellipsis when `ellipsize()` is true.
  void setMaxLines(uint16_t max_lines);

  /// Toggles trailing ellipsis on truncated content.
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

  void setConservativeInkInsets();

  void updateCachedInkInsetsFromCurrentBounds();

  void ensureLayout(XDim width_limit) const;

  Rect getRenderedTextBounds() const;

  std::string value_;
  Insets ink_insets_;
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
