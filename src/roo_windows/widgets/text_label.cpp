#include "roo_display/ui/text_label.h"

#include "roo_backport/string_view.h"
#include "roo_display/ui/string_printer.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

namespace {

Dimensions MeasureLabelText(const TextStyle& text_style,
                            roo::string_view text) {
  auto metrics = text_style.font().getHorizontalStringMetrics(
      text, text_style.fontOptions());
  return Dimensions(metrics.advance(), text_style.lineHeight());
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
                               const TextStyle& text_style,
                               roo::string_view text,
                               roo_display::Alignment alignment) {
  roo_display::StringViewLabel label(text, text_style.font(),
                                     roo_display::color::Transparent,
                                     text_style.fontOptions());
  auto offset = ResolveAlignmentOffset(logical_bounds,
                                       Rect(label.anchorExtents()), alignment);
  return Rect(label.extents()).translate(offset.first, offset.second);
}

}  // namespace

TextLabel::TextLabel(ApplicationContext& context, std::string value,
                     const TextStyle& text_style)
    : TextLabel(context, std::move(value), text_style,
                kGravityLeft | kGravityMiddle) {}

TextLabel::TextLabel(ApplicationContext& context, std::string value,
                     const TextStyle& text_style, Gravity gravity)
    : TextLabel(context, std::move(value), text_style,
                roo_display::color::Transparent, gravity) {}

TextLabel::TextLabel(ApplicationContext& context, std::string value,
                     const TextStyle& text_style, roo_display::Color color,
                     Gravity gravity)
    : PaddingMixin<Widget>(context),
      value_(std::move(value)),
      text_style_(&text_style),
      color_(color),
      gravity_(gravity) {}

void TextLabel::paint(PaintContext& ctx) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  roo_display::StringViewLabel label(value_, textStyle().font(), color,
                                     textStyle().fontOptions());
  auto offset = ResolveAlignmentOffset(bounds(), Rect(label.anchorExtents()),
                                       adjustAlignment(gravity_.asAlignment()));
  ctx.drawObject(
      roo_display::Tile(&label, label.extents(), roo_display::kNoAlign),
      offset.first, offset.second);
}

Insets TextLabel::getInkInsets() const {
  if (value_.empty()) return Insets(0, 0, bounds().width(), bounds().height());
  return InsetsFromContentBounds(
      bounds(),
      ResolveLabelContentBounds(bounds(), textStyle(), value_,
                                adjustAlignment(gravity_.asAlignment())));
}

Dimensions TextLabel::getSuggestedMinimumDimensions() const {
  // NOTE: we could consider pre-calculating and storing these (and avoid
  // re-measuring in paint), but it is an extra 20 bytes per label so it is
  // not a clear win.
  return MeasureLabelText(textStyle(), value_);
}

void TextLabel::setText(std::string value) {
  if (value_ == value) return;
  bool had_old_content = !value_.empty();
  Rect old_bounds = had_old_content ? maxParentBounds() : Rect(0, 0, -1, -1);
  Dimensions old_dimensions = MeasureLabelText(textStyle(), value_);
  Dimensions new_dimensions = MeasureLabelText(textStyle(), value);
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
  Dimensions old_dimensions = MeasureLabelText(textStyle(), value_);
  Dimensions new_dimensions = MeasureLabelText(textStyle(), value);
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

void TextLabel::setTextStyle(const TextStyle& text_style) {
  if (text_style_ == &text_style) return;
  Rect old_bounds = value_.empty() ? Rect(0, 0, -1, -1) : maxParentBounds();
  text_style_ = &text_style;
  invalidateInterior();
  if (!value_.empty()) notifyParentInvalidatedRegion(old_bounds);
  requestLayout();
}

StringViewLabel::StringViewLabel(ApplicationContext& context,
                                 roo::string_view value,
                                 const TextStyle& text_style)
    : StringViewLabel(context, std::move(value), text_style,
                      kGravityLeft | kGravityMiddle) {}

StringViewLabel::StringViewLabel(ApplicationContext& context,
                                 roo::string_view value,
                                 const TextStyle& text_style, Gravity gravity)
    : StringViewLabel(context, std::move(value), text_style,
                      roo_display::color::Transparent, gravity) {}

StringViewLabel::StringViewLabel(ApplicationContext& context,
                                 roo::string_view value,
                                 const TextStyle& text_style,
                                 roo_display::Color color, Gravity gravity)
    : PaddingMixin<Widget>(context),
      value_(std::move(value)),
      text_style_(&text_style),
      color_(color),
      gravity_(gravity) {}

void StringViewLabel::paint(PaintContext& ctx) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  roo_display::StringViewLabel label(value_, textStyle().font(), color,
                                     textStyle().fontOptions());
  auto offset = ResolveAlignmentOffset(bounds(), Rect(label.anchorExtents()),
                                       adjustAlignment(gravity_.asAlignment()));
  ctx.drawObject(
      roo_display::Tile(&label, label.extents(), roo_display::kNoAlign),
      offset.first, offset.second);
}

Insets StringViewLabel::getInkInsets() const {
  if (value_.empty()) return Insets(0, 0, bounds().width(), bounds().height());
  return InsetsFromContentBounds(
      bounds(),
      ResolveLabelContentBounds(bounds(), textStyle(), value_,
                                adjustAlignment(gravity_.asAlignment())));
}

Dimensions StringViewLabel::getSuggestedMinimumDimensions() const {
  // NOTE: we could consider pre-calculating and storing these (and avoid
  // re-measuring in paint), but it is an extra 20 bytes per label so it is
  // not a clear win.
  return MeasureLabelText(textStyle(), value_);
}

void StringViewLabel::setText(roo::string_view value) {
  if (value_ == value) return;
  bool had_old_content = !value_.empty();
  Rect old_bounds = had_old_content ? maxParentBounds() : Rect(0, 0, -1, -1);
  Dimensions old_dimensions = MeasureLabelText(textStyle(), value_);
  Dimensions new_dimensions = MeasureLabelText(textStyle(), value);
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

void StringViewLabel::setColor(roo_display::Color color) {
  if (color_ == color) return;
  color_ = color;
  invalidateInterior();
}

void StringViewLabel::setTextStyle(const TextStyle& text_style) {
  if (text_style_ == &text_style) return;
  Rect old_bounds = value_.empty() ? Rect(0, 0, -1, -1) : maxParentBounds();
  text_style_ = &text_style;
  invalidateInterior();
  if (!value_.empty()) notifyParentInvalidatedRegion(old_bounds);
  requestLayout();
}

}  // namespace roo_windows
