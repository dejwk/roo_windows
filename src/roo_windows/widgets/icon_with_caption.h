#pragma once

#include "roo_display/shape/basic.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_backport/string_view.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

class IconWithCaption : public BasicWidget {
 public:
  IconWithCaption(const Environment& env, const roo_display::Pictogram& def,
                  const std::string& caption)
      : IconWithCaption(env, def, caption, &font_caption()) {}

  IconWithCaption(const Environment& env, const roo_display::Pictogram& def,
                  const std::string& caption, const roo_display::Font* font);

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  const roo_display::Pictogram& icon() const { return *icon_; }

  void setIcon(const roo_display::Pictogram& icon) {
    if (icon_ == &icon) return;
    icon_ = &icon;
    setDirty();
  }

  roo::string_view caption() const { return caption_; }

  void setCaption(std::string caption) {
    if (caption_ == caption) return;
    caption_ = std::move(caption);
    setDirty();
  }

  roo_display::Color color() const { return color_; }

  void setColor(roo_display::Color color) {
    if (color_ == color) return;
    color_ = color;
    setDirty();
  }

 protected:
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
