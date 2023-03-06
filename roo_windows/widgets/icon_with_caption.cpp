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
      icon_->anchorExtents().height() + font_->metrics().maxHeight();
  YDim border = rect.height() - total_height;
  hi_border_ = border / 2;
  lo_border_ = border - hi_border_;
}

void IconWithCaption::paint(const Canvas& canvas) const {
  Color color = color_;
  if (color == roo_display::color::Transparent) {
    const Theme& myTheme = theme();
    color = myTheme.color.defaultColor(canvas.bgcolor());
    if (isActivated() && usesHighlighterColor()) {
      color = myTheme.color.highlighterColor(canvas.bgcolor());
    }
  }
  color = AlphaBlend(canvas.bgcolor(), color);
  if (isInvalidated() && hi_border_ > 0) {
    canvas.clearRect(bounds().xMin(), bounds().yMin(), bounds().xMax(),
                    bounds().yMin() + hi_border_ - 1);
  }

  Rect icon_bounds(bounds().xMin(), hi_border_, bounds().xMax(),
                   hi_border_ + icon_->anchorExtents().height() - 1);
  MaterialIcon icon(*icon_);
  icon.color_mode().setColor(color);
  canvas.drawTiled(icon, icon_bounds,
                   kCenter | kBaseline.shiftBy(icon_bounds.yMin()));

  Rect caption_bounds(bounds().xMin(), icon_bounds.yMax() + 1, bounds().xMax(),
                      icon_bounds.yMax() + font_->metrics().maxHeight());
  canvas.drawTiled(StringViewLabel(caption_, *font_, color), caption_bounds,
                   kCenter | kBaseline.shiftBy(caption_bounds.yMin() +
                                               font_->metrics().ascent()));

  if (isInvalidated() && lo_border_ > 0) {
    canvas.clearRect(bounds().xMin(), bounds().yMax() - lo_border_ + 1,
                    bounds().xMax(), bounds().yMax());
  }
}

Dimensions IconWithCaption::getSuggestedMinimumDimensions() const {
  auto metrics = font_->getHorizontalStringMetrics(caption_);
  return Dimensions(
      std::max(icon_->anchorExtents().width(), (int16_t)metrics.width()),
      icon_->anchorExtents().height() + font_->metrics().maxHeight());
}

}  // namespace roo_windows
