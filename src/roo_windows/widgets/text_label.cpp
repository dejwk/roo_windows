#include "roo_display/ui/text_label.h"

#include "roo_backport/string_view.h"
#include "roo_display/ui/string_printer.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

namespace {

int16_t FloorHalf(int16_t value) {
  return value >= 0 ? value / 2 : -((-value + 1) / 2);
}

int16_t CeilHalf(int16_t value) { return value - FloorHalf(value); }

roo_display::Box StyledLabelAnchorExtents(const TextStyle& text_style,
                                          int16_t advance) {
  // Shift the traditional single-line anchor box so that its middle follows
  // the middle of the ascent. Descent does not affect visual centering.
  const int16_t excess = text_style.lineGap() + text_style.descent();
  return roo_display::Box(
      0, -(text_style.ascent() + text_style.lineGap()) + FloorHalf(excess),
      advance - 1, -text_style.descent() + CeilHalf(excess));
}

class StyledStringViewLabel : public roo_display::Drawable {
 public:
  StyledStringViewLabel(roo::string_view text, const TextStyle& text_style,
                        roo_display::Color color)
      : text_(text),
        text_style_(text_style),
        color_(color),
        metrics_(text_style.font().getHorizontalStringMetrics(
            text, text_style.fontOptions())) {}

  roo_display::Box extents() const override {
    return metrics_.screen_extents();
  }

  roo_display::Box anchorExtents() const override {
    return StyledLabelAnchorExtents(text_style_, metrics_.advance());
  }

 private:
  void drawTo(const roo_display::Surface& surface) const override {
    text_style_.font().drawHorizontalString(surface, text_.data(), text_.size(),
                                            color_, text_style_.fontOptions());
  }

  roo::string_view text_;
  const TextStyle& text_style_;
  roo_display::Color color_;
  roo_display::GlyphMetrics metrics_;
};

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
  const auto& font = text_style.font();
  auto metrics =
      font.getHorizontalStringMetrics(text, text_style.fontOptions());
  Rect anchor_bounds(StyledLabelAnchorExtents(text_style, metrics.advance()));
  auto offset =
      ResolveAlignmentOffset(logical_bounds, anchor_bounds, alignment);
  // A constrained label may clip a long string. Its direct-paint exclusion
  // must not extend into adjacent sibling or surface pixels.
  return Rect::Intersect(
      logical_bounds,
      Rect(metrics.screen_extents()).translate(offset.first, offset.second));
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
    : BasicWidget(context),
      value_(std::move(value)),
      text_style_(&text_style),
      color_(color),
      gravity_(gravity) {}

void TextLabel::paint(PaintContext& ctx) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  ctx.drawTiled(StyledStringViewLabel(value_, textStyle(), color), bounds(),
                adjustAlignment(gravity_.asAlignment()));
}

Insets TextLabel::getInkInsets() const {
  if (value_.empty()) return Insets::Zero();
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
    : BasicWidget(context),
      value_(std::move(value)),
      text_style_(&text_style),
      color_(color),
      gravity_(gravity) {}

void StringViewLabel::paint(PaintContext& ctx) const {
  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;
  ctx.drawTiled(StyledStringViewLabel(value_, textStyle(), color), bounds(),
                adjustAlignment(gravity_.asAlignment()));
}

Insets StringViewLabel::getInkInsets() const {
  if (value_.empty()) return Insets::Zero();
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

void StringViewLabel::setTextStyle(const TextStyle& text_style) {
  if (text_style_ == &text_style) return;
  Rect old_bounds = value_.empty() ? Rect(0, 0, -1, -1) : maxParentBounds();
  text_style_ = &text_style;
  invalidateInterior();
  if (!value_.empty()) notifyParentInvalidatedRegion(old_bounds);
  requestLayout();
}

}  // namespace roo_windows
