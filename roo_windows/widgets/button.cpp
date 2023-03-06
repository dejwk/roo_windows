#include "button.h"

#include "roo_display/core/raster.h"
#include "roo_display/shape/basic_shapes.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"

using namespace roo_display;

namespace roo_windows {

Button::Button(const Environment& env, Style style)
    : BasicWidget(env),
      style_(style),
      outline_color_(style == CONTAINED  ? env.theme().color.primary
                     : style == OUTLINED ? env.theme().color.onBackground
                                         : color::Transparent),
      interior_color_(style == CONTAINED ? env.theme().color.primary
                                         : env.theme().color.background),
      elevation_resting_(0),
      elevation_pressed_(0),
      corner_radius_(Scaled(4)) {
  outline_color_.set_a(0x80);
}

void Button::setElevation(uint8_t resting, uint8_t pressed) {
  if (elevation_resting_ != resting) {
    uint8_t larger = std::max(elevation_resting_, resting);
    elevation_resting_ = resting;
    if (!isPressed()) {
      elevationChanged(larger);
    }
  }
  if (elevation_pressed_ != pressed) {
    uint8_t larger = std::max(elevation_pressed_, pressed);
    elevation_pressed_ = pressed;
    if (isPressed()) {
      elevationChanged(larger);
    }
  }
}

Padding Button::getDefaultPadding() const { return Padding(4, 4); }

Padding SimpleButton::getDefaultPadding() const { return Padding(14, 4); }

SimpleButton::SimpleButton(const Environment& env, const MonoIcon* icon,
                           std::string label, Style style)
    : Button(env, style),
      content_color_(style == CONTAINED ? env.theme().color.onPrimary
                                        : env.theme().color.primary),
      font_(&font_button()),
      label_(std::move(label)),
      icon_(icon) {}

Dimensions SimpleButton::getSuggestedMinimumDimensions() const {
  if (!hasLabel()) {
    if (!hasIcon()) return Dimensions(0, 0);
    // Icon only.
    return Dimensions(Scaled(4) + icon().anchorExtents().width(),
                      Scaled(4) + icon().anchorExtents().height());
  }
  // We must measure the text.
  auto metrics = font_->getHorizontalStringMetrics(label_);
  int16_t text_height = ((font_->metrics().maxHeight()) + 1) & ~1;
  if (!hasIcon()) {
    // Label only.
    return Dimensions(Scaled(4) + metrics.width(), Scaled(4) + text_height);
  }
  // Both icon and label.
  int16_t gap = (icon().anchorExtents().width() / 2) & 0xFFFC;
  return Dimensions(
      Scaled(4) + icon().anchorExtents().width() + gap + metrics.width(),
      Scaled(4) + std::max(icon().anchorExtents().height(), text_height));
}

void SimpleButton::paint(const Canvas& canvas) const {
  Rect bounds = this->bounds();
  if (!hasIcon() && label().empty()) {
    canvas.clearRect(bounds);
    return;
  }
  if (label().empty()) {
    // Icon only.
    MonoIcon ic = icon();
    ic.color_mode().setColor(contentColor());
    canvas.drawTiled(ic, bounds, kCenter | kMiddle);
    return;
  }
  if (!hasIcon()) {
    canvas.drawTiled(StringViewLabel(label_, *font_, contentColor()), bounds,
                     kCenter | kMiddle);
    return;
  }
  // Both text and icon.
  MonoIcon ic = icon();
  StringViewLabel l(label_, *font_, contentColor());
  ic.color_mode().setColor(contentColor());
  int16_t gap = (ic.anchorExtents().width() / 2) & 0xFFFC;
  int16_t x_offset = (bounds.width() - ic.anchorExtents().width() -
                      l.anchorExtents().width() - gap) /
                     2;
  int16_t x_cursor = bounds.xMin();
  const int16_t yMin = bounds.yMin();
  const int16_t yMax = bounds.yMax();
  // Left border.
  if (x_offset > 0) {
    canvas.clearRect(x_cursor, yMin, x_cursor + x_offset - 1, yMax);
  }
  x_cursor += x_offset;  // Note: x_offset may be negative.
  // Icon.
  {
    Rect r(x_cursor, yMin, x_cursor + ic.anchorExtents().width() - 1, yMax);
    canvas.drawTiled(ic, r, kCenter | kMiddle);
  }
  x_cursor += ic.anchorExtents().width();
  // Gap.
  canvas.clearRect(x_cursor, yMin, x_cursor + gap - 1, yMax);
  x_cursor += gap;
  // Text.
  {
    Rect r(x_cursor, yMin, x_cursor + l.anchorExtents().width() - 1, yMax);
    canvas.drawTiled(l, r, kCenter | kMiddle);
  }
  x_cursor += l.anchorExtents().width();
  // Right border.
  if (x_offset <= bounds.xMax()) {
    canvas.clearRect(x_cursor, yMin, bounds.xMax(), yMax);
  }
}

}  // namespace roo_windows
