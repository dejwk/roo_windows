#pragma once

#include "roo_backport/string_view.h"
#include "roo_display/shape/basic.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

/// Icon stacked above a short text caption.
///
/// Used by `Destination` (navigation rail entries) and similar compound
/// widgets. The caption font defaults to the theme's caption font.
class IconWithCaption : public BasicWidget {
 public:
  IconWithCaption(ApplicationContext& context, const roo_display::Pictogram& def,
                  const std::string& caption)
      : IconWithCaption(context, def, caption, &font_caption()) {}

  IconWithCaption(ApplicationContext& context, const roo_display::Pictogram& def,
                  const std::string& caption, const roo_display::Font* font);

  /// Paints the icon centered horizontally above the caption, both in the
  /// currently configured color.
  void paint(PaintContext& ctx) const override;

  /// Reports a footprint large enough to contain the icon stacked above the
  /// caption rendered in the configured font, with a small gap between them.
  Dimensions getSuggestedMinimumDimensions() const override;

  const roo_display::Pictogram& icon() const { return *icon_; }

  /// Replaces the icon. No-op if unchanged.
  void setIcon(const roo_display::Pictogram& icon) {
    if (icon_ == &icon) return;
    icon_ = &icon;
    setDirty();
  }

  roo::string_view caption() const { return caption_; }

  /// Replaces the caption string. No-op if unchanged.
  void setCaption(std::string caption) {
    if (caption_ == caption) return;
    caption_ = std::move(caption);
    setDirty();
  }

  roo_display::Color color() const { return color_; }

  /// Sets the foreground color for both the icon and the caption. No-op if
  /// unchanged. Transparent defers to the theme.
  void setColor(roo_display::Color color) {
    if (color_ == color) return;
    color_ = color;
    setDirty();
  }

 protected:
  /// Caches the vertical split point between icon and caption based on the
  /// final widget size.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  const roo_display::Pictogram* icon_;
  std::string caption_;
  const roo_display::Font* font_;
  Color color_;
  int16_t hi_border_;
  int16_t lo_border_;
};

}  // namespace roo_windows
