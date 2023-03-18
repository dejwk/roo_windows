#include "roo_display/ui/string_printer.h"
#include "roo_display/ui/text_label.h"

#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

TextLabel::TextLabel(const Environment& env, std::string value,
                     const roo_display::Font& font)
    : TextLabel(env, std::move(value), font,
                roo_display::kLeft | roo_display::kMiddle) {}

TextLabel::TextLabel(const Environment& env, std::string value,
                     const roo_display::Font& font,
                     roo_display::Alignment alignment)
    : TextLabel(env, std::move(value), font, roo_display::color::Transparent,
                alignment) {}

TextLabel::TextLabel(const Environment& env, std::string value,
                     const roo_display::Font& font, roo_display::Color color,
                     roo_display::Alignment alignment)
    : BasicWidget(env),
      value_(std::move(value)),
      font_(font),
      color_(color),
      alignment_(alignment) {}

void TextLabel::paint(const Canvas& canvas) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  canvas.drawTiled(roo_display::StringViewLabel(value_, font_, color), bounds(),
                   adjustAlignment(alignment_));
}

Dimensions TextLabel::getSuggestedMinimumDimensions() const {
  // NOTE: we could consider pre-calculating and storing these (and avoid
  // re-measuring in paint), but it is an extra 20 bytes per label so it is
  // not a clear win.
  auto metrics = font_.getHorizontalStringMetrics(value_);
  return Dimensions(metrics.advance(), font_.metrics().ascent() -
                                           font_.metrics().descent() +
                                           font_.metrics().linegap());
}

void TextLabel::setText(std::string value) {
  if (value_ == value) return;
  value_ = std::move(value);
  markDirty();
  requestLayout();
}

void TextLabel::setText(const char* value) {
  setText(roo_display::StringView(value));
}

void TextLabel::setText(roo_display::StringView value) {
  if (value_ == value) return;
  value_ = std::string((const char*)value.data(), value.size());
  markDirty();
  requestLayout();
}

void TextLabel::setTextf(const char* format, ...) {
  va_list arg;
  va_start(arg, format);
  setTextvf(format, arg);
  va_end(arg);
}

void TextLabel::setTextvf(const char* format, va_list arg) {
  setText(roo_display::StringVPrintf(format, arg));
}

}  // namespace roo_windows