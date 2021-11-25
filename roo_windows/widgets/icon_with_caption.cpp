#include "icon_with_caption.h"

using namespace roo_display;

namespace roo_windows {

IconWithCaption::IconWithCaption(const Environment& env,
                                 const MaterialIcon& def,
                                 const std::string& caption, const Font* font)
    : Widget(env),
      icon_(def),
      caption_(caption),
      font_(font),
      hi_border_(0),
      lo_border_(0) {}

void IconWithCaption::onLayout(bool changed, const Box& box) {
  int16_t total_height =
      icon_.extents().height() + font_->metrics().maxHeight();
  int16_t border = box.height() - total_height;
  hi_border_ = border / 2;
  lo_border_ = border - hi_border_;
}

bool IconWithCaption::paint(const Surface& s) {
  const Theme& myTheme = theme();
  Color color = myTheme.color.defaultColor(s.bgcolor());
  if (isActivated() && usesHighlighterColor()) {
    color = myTheme.color.highlighterColor(s.bgcolor());
  }
  if (s.fill_mode() == FILL_MODE_RECTANGLE && isInvalidated() &&
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

  if (s.fill_mode() == FILL_MODE_RECTANGLE && isInvalidated() &&
      lo_border_ > 0) {
    s.drawObject(FilledRect(bounds().xMin(), bounds().yMax() - lo_border_ + 1,
                            bounds().xMax(), bounds().yMax(),
                            color::Transparent));
  }
  return true;
}

Dimensions IconWithCaption::getSuggestedMinimumDimensions() const {
  auto metrics =
      font_->getHorizontalStringMetrics((const uint8_t*)caption_.c_str(), caption_.size());
  return Dimensions(std::max(icon_.extents().width(), (int16_t)metrics.width()),
                    icon_.extents().height() + font_->metrics().maxHeight());
}

}  // namespace roo_windows
