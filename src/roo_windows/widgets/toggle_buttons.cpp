#include "roo_windows/widgets/toggle_buttons.h"

#include "roo_display/shape/basic.h"
#include "roo_display/ui/tile.h"

namespace roo_windows {

using namespace roo_display;

roo_windows::Widget& ToggleButtons::addButton(const MonoIcon& icon) {
  int16_t width = 0;
  int16_t height = 0;
  for (const auto& i : buttons_) {
    const roo_display::Box& extents = i->icon().anchorExtents();
    width += extents.width();
    width += 2 * padding_;
    height = std::max(height, extents.height());
  }
  height = std::max(height, icon.anchorExtents().height());
  height += 2 * padding_;
  int idx = buttons_.size();
  buttons_.emplace_back(new ToggleButton(env_, icon, *this, idx));
  auto& btn = *buttons_.back();
  Panel::add(btn, Rect(width + 1, 0,
                       width + icon.anchorExtents().width() + 2 * padding_,
                       icon.anchorExtents().height() + 2 * padding_ - 1));
  return btn;
}

void ToggleButtons::paint(const Canvas& canvas) const {
  Color border = theme().color.defaultColor(canvas.bgcolor());
  border.set_a(0x30);
  Dimensions d = getNaturalDimensions();
  canvas.clearRect(0, 0, 0, 0);
  canvas.drawVLine(0, 1, d.height() - 2, border);
  canvas.drawVLine(0, d.height() - 1, d.height() - 1,
                   roo_display::color::Background);
  int16_t x = d.width() - 1;
  canvas.clearRect(x, 0, x, 0);
  canvas.drawVLine(x, 1, d.height() - 2, border);
  canvas.drawVLine(x, d.height() - 1, d.height() - 1,
                   roo_display::color::Background);
  if (d.width() < width()) {
    canvas.fillRect(d.width(), 0, width() - 1, d.height() - 1, background());
  }
  if (d.height() < height()) {
    canvas.fillRect(0, d.height(), width() - 1, height() - 1, background());
  }
}

Dimensions ToggleButtons::getSuggestedMinimumDimensions() const {
  int16_t width = 0;
  int16_t height = 0;
  for (const auto& i : buttons_) {
    const roo_display::Box& extents = i->icon().anchorExtents();
    width += extents.width();
    width += 2;
    height = std::max(height, extents.height());
  }
  return Dimensions(width + 2, height + 2);
}

bool ToggleButtons::setActive(int index) {
  if (index == active_) return false;
  active_ = index;
  for (size_t i = 0; i < buttons_.size(); i++) {
    buttons_[i]->setActivated(i == (size_t)index);
  }
  return true;
}

void ToggleButtons::notifyButtonClicked(int index) {
  if (setActive(index)) {
    triggerInteractiveChange();
  }
}

void ToggleButtons::ToggleButton::paint(const Canvas& canvas) const {
  Color color = theme().color.defaultColor(canvas.bgcolor());
  Color internal_border = color;
  internal_border.set_a(0x10);
  Color external_border = color;
  external_border.set_a(0x30);
  canvas.drawHLine(0, 0, width() - 1, external_border);
  canvas.drawVLine(0, 1, height() - 3, internal_border);
  canvas.drawHLine(1, 1, width() - 2, internal_border);
  MonoIcon icon(icon_);
  icon.color_mode().setColor(color);
  canvas.drawTiled(icon,
                   Rect(bounds().xMin() + 1, bounds().yMin() + 2,
                        bounds().xMax() - 1, bounds().yMax() - 2),
                   kCenter | kMiddle);
  canvas.drawVLine(width() - 1, 0, height() - 3, external_border);
  canvas.fillRect(0, height() - 2, width() - 1, height() - 1, external_border);
}

}  // namespace roo_windows
