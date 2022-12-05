#include "icon_with_caption.h"

using namespace roo_display;

namespace roo_windows {

IconWithCaption::IconWithCaption(const Environment& env,
                                 const MaterialIcon& def,
                                 const std::string& caption, const Font* font)
    : BasicWidget(env),
      icon_(&def),
      caption_(caption),
      font_(font),
      color_(roo_display::color::Transparent),
      hi_border_(0),
      lo_border_(0) {}

void IconWithCaption::onLayout(bool changed, const Rect& rect) {
  int16_t total_height =
      icon_->extents().height() + font_->metrics().maxHeight();
  YDim border = rect.height() - total_height;
  hi_border_ = border / 2;
  lo_border_ = border - hi_border_;
}

bool IconWithCaption::paint(const Canvas& canvas) {
  Color color = color_;
  if (color == roo_display::color::Transparent) {
    const Theme& myTheme = theme();
    color = myTheme.color.defaultColor(canvas.bgcolor());
    if (isActivated() && usesHighlighterColor()) {
      color = myTheme.color.highlighterColor(canvas.bgcolor());
    }
  }
  color = alphaBlend(canvas.bgcolor(), color);
  if (isInvalidated() && hi_border_ > 0) {
    canvas.drawObject(
        FilledRect(bounds().xMin(), bounds().yMin(), bounds().xMax(),
                   bounds().yMin() + hi_border_ - 1, color::Transparent));
  }

  Box icon_bounds(bounds().xMin(), hi_border_, bounds().xMax(),
                  hi_border_ + icon_->extents().height() - 1);
  MaterialIcon icon(*icon_);
  icon.color_mode().setColor(color);
  canvas.drawObject(Tile(&icon, icon_bounds,
                         kCenter | kBaseline.shiftBy(icon_bounds.yMin())));

  roo_display::Box caption_bounds(
      bounds().xMin(), icon_bounds.yMax() + 1, bounds().xMax(),
      icon_bounds.yMax() + font_->metrics().maxHeight());
  canvas.drawObject(
      MakeTileOf(StringViewLabel(caption_, *font_, color), caption_bounds,
                 kCenter | kBaseline.shiftBy(caption_bounds.yMin() +
                                             font_->metrics().ascent())));

  if (isInvalidated() && lo_border_ > 0) {
    canvas.drawObject(
        FilledRect(bounds().xMin(), bounds().yMax() - lo_border_ + 1,
                   bounds().xMax(), bounds().yMax(), color::Transparent));
  }
  return true;
}

Dimensions IconWithCaption::getSuggestedMinimumDimensions() const {
  auto metrics = font_->getHorizontalStringMetrics(caption_);
  return Dimensions(
      std::max(icon_->extents().width(), (int16_t)metrics.width()),
      icon_->extents().height() + font_->metrics().maxHeight());
}

}  // namespace roo_windows
