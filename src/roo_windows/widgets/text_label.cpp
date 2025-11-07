#include "roo_display/ui/text_label.h"

#include "roo_backport/string_view.h"
#include "roo_display/ui/string_printer.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

TextLabel::TextLabel(const Environment& env, std::string value,
                     const roo_display::Font& font)
    : TextLabel(env, std::move(value), font, kGravityLeft | kGravityMiddle) {}

TextLabel::TextLabel(const Environment& env, std::string value,
                     const roo_display::Font& font, Gravity gravity)
    : TextLabel(env, std::move(value), font, roo_display::color::Transparent,
                gravity) {}

TextLabel::TextLabel(const Environment& env, std::string value,
                     const roo_display::Font& font, roo_display::Color color,
                     Gravity gravity)
    : BasicWidget(env),
      value_(std::move(value)),
      font_(font),
      color_(color),
      gravity_(gravity) {}

void TextLabel::paint(const Canvas& canvas) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  canvas.drawTiled(roo_display::StringViewLabel(value_, font_, color), bounds(),
                   adjustAlignment(gravity_.asAlignment()));
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
  // setDirty();
  invalidateInterior();
  requestLayout();
}

void TextLabel::setText(const char* value) { setText(roo::string_view(value)); }

void TextLabel::setText(roo::string_view value) {
  if (value_ == value) return;
  value_ = std::string((const char*)value.data(), value.size());
  // setDirty();
  invalidateInterior();
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

void TextLabel::clearText() {
  if (value_.empty()) return;
  value_.clear();
  // setDirty();
  invalidateInterior();
  requestLayout();
}

StringViewLabel::StringViewLabel(const Environment& env, roo::string_view value,
                                 const roo_display::Font& font)
    : StringViewLabel(env, std::move(value), font,
                      kGravityLeft | kGravityMiddle) {}

StringViewLabel::StringViewLabel(const Environment& env, roo::string_view value,
                                 const roo_display::Font& font, Gravity gravity)
    : StringViewLabel(env, std::move(value), font,
                      roo_display::color::Transparent, gravity) {}

StringViewLabel::StringViewLabel(const Environment& env, roo::string_view value,
                                 const roo_display::Font& font,
                                 roo_display::Color color, Gravity gravity)
    : BasicWidget(env),
      value_(std::move(value)),
      font_(font),
      color_(color),
      gravity_(gravity) {}

void StringViewLabel::paint(const Canvas& canvas) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  canvas.drawTiled(roo_display::StringViewLabel(value_, font_, color), bounds(),
                   adjustAlignment(gravity_.asAlignment()));
}

Dimensions StringViewLabel::getSuggestedMinimumDimensions() const {
  // NOTE: we could consider pre-calculating and storing these (and avoid
  // re-measuring in paint), but it is an extra 20 bytes per label so it is
  // not a clear win.
  auto metrics = font_.getHorizontalStringMetrics(value_);
  return Dimensions(metrics.advance(), font_.metrics().ascent() -
                                           font_.metrics().descent() +
                                           font_.metrics().linegap());
}

void StringViewLabel::setText(roo::string_view value) {
  if (value_ == value) return;
  value_ = std::move(value);
  // setDirty();
  invalidateInterior();
  requestLayout();
}

void StringViewLabel::clearText() {
  if (value_.empty()) return;
  value_ = "";
  // setDirty();
  invalidateInterior();
  requestLayout();
}

}  // namespace roo_windows