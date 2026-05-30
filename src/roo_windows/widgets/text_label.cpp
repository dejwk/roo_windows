#include "roo_display/ui/text_label.h"

#include "roo_backport/string_view.h"
#include "roo_display/ui/string_printer.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

namespace {

Dimensions MeasureLabelText(const roo_display::Font& font,
                            roo::string_view text) {
  auto metrics = font.getHorizontalStringMetrics(text);
  return Dimensions(metrics.advance(), font.metrics().ascent() -
                                           font.metrics().descent() +
                                           font.metrics().linegap());
}

bool DimensionsDiffer(const Dimensions& a, const Dimensions& b) {
  return a.width() != b.width() || a.height() != b.height();
}

Insets InsetsFromContentBounds(const Rect& logical_bounds,
                               const Rect& content_bounds) {
  return Insets(content_bounds.xMin() - logical_bounds.xMin(),
                content_bounds.yMin() - logical_bounds.yMin(),
                logical_bounds.xMax() - content_bounds.xMax(),
                logical_bounds.yMax() - content_bounds.yMax());
}

Rect ResolveLabelContentBounds(const Rect& logical_bounds,
                               const roo_display::Font& font,
                               roo::string_view text,
                               roo_display::Alignment alignment) {
  auto metrics = font.getHorizontalStringMetrics(text);
  Rect anchor_bounds(0, -font.metrics().ascent() - font.metrics().linegap(),
                     metrics.advance() - 1, -font.metrics().descent());
  auto offset =
      ResolveAlignmentOffset(logical_bounds, anchor_bounds, alignment);
  return Rect(metrics.screen_extents()).translate(offset.first, offset.second);
}

}  // namespace

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

void TextLabel::paint(PaintContext& ctx) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  ctx.drawTiled(roo_display::StringViewLabel(value_, font_, color), bounds(),
                adjustAlignment(gravity_.asAlignment()));
}

Insets TextLabel::getInkInsets() const {
  if (value_.empty()) return Insets::Zero();
  return InsetsFromContentBounds(
      bounds(),
      ResolveLabelContentBounds(bounds(), font_, value_,
                                adjustAlignment(gravity_.asAlignment())));
}

Dimensions TextLabel::getSuggestedMinimumDimensions() const {
  // NOTE: we could consider pre-calculating and storing these (and avoid
  // re-measuring in paint), but it is an extra 20 bytes per label so it is
  // not a clear win.
  return MeasureLabelText(font_, value_);
}

void TextLabel::setText(std::string value) {
  if (value_ == value) return;
  bool had_old_content = !value_.empty();
  Rect old_bounds = had_old_content ? maxParentBounds() : Rect(0, 0, -1, -1);
  Dimensions old_dimensions = MeasureLabelText(font_, value_);
  Dimensions new_dimensions = MeasureLabelText(font_, value);
  value_ = std::move(value);
  invalidateInterior();
  if (had_old_content) {
    notifyParentInvalidatedRegion(old_bounds);
  }
  if (DimensionsDiffer(old_dimensions, new_dimensions)) {
    requestLayout();
  }
}

void TextLabel::setText(const char* value) { setText(roo::string_view(value)); }

void TextLabel::setText(roo::string_view value) {
  if (value_ == value) return;
  bool had_old_content = !value_.empty();
  Rect old_bounds = had_old_content ? maxParentBounds() : Rect(0, 0, -1, -1);
  Dimensions old_dimensions = MeasureLabelText(font_, value_);
  Dimensions new_dimensions = MeasureLabelText(font_, value);
  value_ = std::string((const char*)value.data(), value.size());
  invalidateInterior();
  if (had_old_content) {
    notifyParentInvalidatedRegion(old_bounds);
  }
  if (DimensionsDiffer(old_dimensions, new_dimensions)) {
    requestLayout();
  }
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
  Rect old_bounds = maxParentBounds();
  value_.clear();
  invalidateInterior();
  notifyParentInvalidatedRegion(old_bounds);
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

void StringViewLabel::paint(PaintContext& ctx) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  ctx.drawTiled(roo_display::StringViewLabel(value_, font_, color), bounds(),
                adjustAlignment(gravity_.asAlignment()));
}

Insets StringViewLabel::getInkInsets() const {
  if (value_.empty()) return Insets::Zero();
  return InsetsFromContentBounds(
      bounds(),
      ResolveLabelContentBounds(bounds(), font_, value_,
                                adjustAlignment(gravity_.asAlignment())));
}

Dimensions StringViewLabel::getSuggestedMinimumDimensions() const {
  // NOTE: we could consider pre-calculating and storing these (and avoid
  // re-measuring in paint), but it is an extra 20 bytes per label so it is
  // not a clear win.
  return MeasureLabelText(font_, value_);
}

void StringViewLabel::setText(roo::string_view value) {
  if (value_ == value) return;
  bool had_old_content = !value_.empty();
  Rect old_bounds = had_old_content ? maxParentBounds() : Rect(0, 0, -1, -1);
  Dimensions old_dimensions = MeasureLabelText(font_, value_);
  Dimensions new_dimensions = MeasureLabelText(font_, value);
  value_ = std::move(value);
  invalidateInterior();
  if (had_old_content) {
    notifyParentInvalidatedRegion(old_bounds);
  }
  if (DimensionsDiffer(old_dimensions, new_dimensions)) {
    requestLayout();
  }
}

void StringViewLabel::clearText() {
  if (value_.empty()) return;
  Rect old_bounds = maxParentBounds();
  value_ = "";
  invalidateInterior();
  notifyParentInvalidatedRegion(old_bounds);
  requestLayout();
}

}  // namespace roo_windows