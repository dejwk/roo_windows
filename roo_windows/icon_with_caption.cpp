#include "icon_with_caption.h"

using namespace roo_display;

namespace roo_windows {

IconWithCaption::IconWithCaption(Panel* parent, const Box& bounds,
                                 const MaterialIcon& def,
                                 const std::string& caption, const Font* font)
    : Widget(parent, bounds), icon_(def), caption_(caption), font_(font) {
  int16_t total_height = icon_.extents().height() + font->metrics().maxHeight();
  int16_t border = bounds.height() - total_height;
  hi_border_ = border / 2;
  lo_border_ = border - hi_border_;
}

void IconWithCaption::defaultPaint(const Surface& s) {
  Color color = theme().color.defaultColor(s.bgcolor());
  if (isActivated() && usesHighlighterColor()) {
    color = theme().color.highlighterColor(s.bgcolor());
  }
  if (s.fill_mode() == FILL_MODE_RECTANGLE && needs_repaint_ &&
      hi_border_ > 0) {
    s.drawObject(FilledRect(bounds().xMin(), bounds().yMin(), bounds().xMax(),
                            bounds().yMin() + hi_border_ - 1,
                            color::Transparent));
  }

  Box icon_bounds(bounds().xMin(), hi_border_, bounds().xMax(),
                  hi_border_ + icon_.extents().height() - 1);
  MaterialIcon icon(icon_);
  icon.color_mode().setColor(color);
  s.drawObject(Tile(&icon, icon_bounds, HAlign::Center(),
                    VAlign::None(icon_bounds.yMin())));

  Box caption_bounds(bounds().xMin(), icon_bounds.yMax() + 1, bounds().xMax(),
                     icon_bounds.yMax() + font_->metrics().maxHeight());
  s.drawObject(MakeTileOf(
      TextLabel(*font_, caption_, color), caption_bounds, HAlign::Center(),
      VAlign::None(caption_bounds.yMin() + font_->metrics().ascent())));

  if (s.fill_mode() == FILL_MODE_RECTANGLE && needs_repaint_ &&
      lo_border_ > 0) {
    s.drawObject(FilledRect(bounds().xMin(), bounds().yMax() - lo_border_ + 1,
                            bounds().xMax(), bounds().yMax(),
                            color::Transparent));
  }
}

}  // namespace roo_windows