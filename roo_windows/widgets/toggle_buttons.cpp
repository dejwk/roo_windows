#include "roo_windows/widgets/toggle_buttons.h"

#include "roo_display/shape/basic_shapes.h"
#include "roo_display/ui/tile.h"

namespace roo_windows {

roo_windows::Widget& ToggleButtons::addButton(const MonoIcon& icon) {
  int16_t width = 0;
  int16_t height = 0;
  for (const auto& i : buttons_) {
    const Box& extents = i->icon().extents();
    width += extents.width();
    width += 2 * padding_;
    height = std::max(height, extents.height());
  }
  height = std::max(height, icon.extents().height());
  height += 2 * padding_;
  int idx = buttons_.size();
  buttons_.emplace_back(new ToggleButton(env_, icon, *this, idx));
  auto& btn = *buttons_.back();
  Panel::add(btn,
             Box(width + 1, 0, width + icon.extents().width() + 2 * padding_,
                 icon.extents().height() + 2 * padding_ - 1));
  return btn;
}

bool ToggleButtons::paint(const roo_display::Surface& s) {
  Color border = theme().color.defaultColor(s.bgcolor());
  border.set_a(0x30);
  Dimensions d = getNaturalDimensions();
  s.drawObject(roo_display::Line(0, 0, 0, 0, roo_display::color::Transparent));
  s.drawObject(roo_display::Line(0, 1, 0, d.height() - 2, border));
  s.drawObject(roo_display::Line(0, d.height() - 1, 0, d.height() - 1,
                                 roo_display::color::Transparent));
  int16_t x = d.width() - 1;
  s.drawObject(roo_display::Line(x, 0, x, 0, roo_display::color::Transparent));
  s.drawObject(roo_display::Line(x, 1, x, d.height() - 2, border));
  s.drawObject(roo_display::Line(x, d.height() - 1, x, d.height() - 1,
                                 roo_display::color::Transparent));
  if (d.width() < width()) {
    s.drawObject(roo_display::FilledRect(d.width(), 0, width() - 1,
                                         d.height() - 1, background()));
  }
  if (d.height() < height()) {
    s.drawObject(roo_display::FilledRect(0, d.height(), width() - 1,
                                         height() - 1, background()));
  }
  return true;
}

Dimensions ToggleButtons::getSuggestedMinimumDimensions() const {
  int16_t width = 0;
  int16_t height = 0;
  for (const auto& i : buttons_) {
    const Box& extents = i->icon().extents();
    width += extents.width();
    width += 2;
    height = std::max(height, extents.height());
  }
  return Dimensions(width + 2, height + 2);
}

void ToggleButtons::setActive(int index) {
  if (index == active_) return;
  active_ = index;
  for (size_t i = 0; i < buttons_.size(); i++) {
    buttons_[i]->setActivated(i == (size_t)index);
  }
}

bool ToggleButtons::ToggleButton::paint(const roo_display::Surface& s) {
  Color color = theme().color.defaultColor(s.bgcolor());
  Color internal_border = color;
  internal_border.set_a(0x10);
  Color external_border = color;
  external_border.set_a(0x30);
  s.drawObject(roo_display::Line(0, 0, width() - 1, 0, external_border));
  s.drawObject(roo_display::Line(0, 1, 0, height() - 3, internal_border));
  s.drawObject(roo_display::Line(1, 1, width() - 2, 1, internal_border));
  MonoIcon icon(icon_);
  icon.color_mode().setColor(color);
  roo_display::Tile tile(&icon,
                         Box(bounds().xMin() + 1, bounds().yMin() + 2,
                             bounds().xMax() - 1, bounds().yMax() - 2),
                         roo_display::kCenter | roo_display::kMiddle,
                         roo_display::color::Transparent, s.fill_mode());
  s.drawObject(tile);
  s.drawObject(roo_display::Line(width() - 1, 0, width() - 1, height() - 3,
                                 external_border));
  s.drawObject(roo_display::FilledRect(0, height() - 2, width() - 1,
                                       height() - 1, external_border));
  return true;
}

}  // namespace roo_windows
