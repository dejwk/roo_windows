#include "roo_windows/widgets/text_block.h"

#include "roo_display/shape/basic_shapes.h"

namespace roo_windows {

namespace {

class Interior : public roo_display::Drawable {
 public:
  Interior(roo_display::Box extents, roo_display::StringView value,
           const roo_display::Font& font, roo_display::Color color)
      : extents_(extents), value_(value), font_(font), color_(color) {}

  roo_display::Box extents() const override { return extents_; }

 private:
  void drawTo(const roo_display::Surface& s) const override {
    if (value_.empty()) return;
    int16_t line_height = font_.metrics().maxHeight();
    int16_t y = 0;
    const uint8_t* p = &*value_.begin();
    const uint8_t* end = &*value_.end();
    do {
      const uint8_t* start = p;
      while (p != end && *p != '\n') {
        ++p;
      }
      roo_display::Surface news = s;
      news.set_dy(s.dy() + y + font_.metrics().glyphYMax());
      auto metrics = font_.getHorizontalStringMetrics(start, p - start);
      font_.drawHorizontalString(news, start, p - start, color_);
      if (metrics.width() < extents_.width()) {
        news.drawObject(roo_display::FilledRect(
            metrics.width(), -font_.metrics().glyphYMax(), extents_.width() - 1,
            -font_.metrics().glyphYMin(), news.bgcolor()));
      }

      y += line_height;

      // Skip over the '\n' character.
      if (p != end) ++p;
    } while (p != end);
  }

  roo_display::Box extents_;
  const roo_display::StringView value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
};

}  // namespace

TextBlock::TextBlock(const Environment& env, std::string value,
                     const roo_display::Font& font, roo_display::Color color,
                     roo_display::Alignment alignment)
    : BasicWidget(env),
      value_(),
      font_(font),
      color_(color),
      alignment_(alignment) {
  setContent(value);
}

void TextBlock::paint(const Canvas& canvas) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  canvas.drawTiled(Interior(roo_display::Box(0, 0, text_dims_.width() - 1,
                                             text_dims_.height() - 1),
                            value_, font_, color),
                   bounds(), adjustAlignment(alignment_));
}

Dimensions TextBlock::getSuggestedMinimumDimensions() const {
  return text_dims_;
}

void TextBlock::setContent(std::string value) {
  if (value_ == value) return;
  value_ = std::move(value);

  if (value_.empty()) {
    text_dims_ = Dimensions(0, 0);
  } else {
    int16_t line_count = 0;
    int16_t max_line_width = 0;
    const uint8_t* p = (const uint8_t*)&*value_.begin();
    const uint8_t* end = (const uint8_t*)&*value_.end();
    do {
      ++line_count;
      const uint8_t* start = p;
      while (p != end && *p != '\n') {
        ++p;
      }
      if (p != start) {
        auto metrics = font_.getHorizontalStringMetrics(start, p - start);
        if (metrics.width() > max_line_width) {
          max_line_width = metrics.width();
        }
      }
      // Skip over the '\n' character.
      if (p != end) ++p;
    } while (p != end);

    text_dims_ =
        Dimensions(max_line_width, line_count * font_.metrics().maxHeight());
  }

  markDirty();
  requestLayout();
}

void TextBlock::setColor(roo_display::Color color) {
  if (color_ == color) return;
  color_ = color;
  markDirty();
}

}  // namespace roo_windows