#pragma once

#include <memory>

#include "roo_display/filter/color_filter.h"
#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_material_icons.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {

class ToggleButtons : public Panel {
 public:
  ToggleButtons(Panel* parent, int16_t dx, int16_t dy, int16_t height)
      : Panel(parent, Box(dx, dy, dx + 1, dy + height - 1)), active_(-1) {}

  roo_windows::Widget* addButton(const MonoIcon& icon, int16_t width) {
    Box box(this->width() - 1, 0, this->width() - 1 + width - 1, height() - 1);
    ToggleButton* btn = new ToggleButton(this, box, icon);
    int idx = buttons_.size();
    btn->setOnClicked([this, idx] { setActive(idx); });
    buttons_.push_back(btn);
    moveTo(Box(parent_bounds().xMin(), parent_bounds().yMin(),
               parent_bounds().xMax() + width, parent_bounds().yMax()));
    return btn;
  }

  void paint(const roo_display::Surface& s) override {
    Color border = theme().color.defaultColor(s.bgcolor());
    border.set_a(0x30);
    s.drawObject(
        roo_display::Line(0, 0, 0, 0, roo_display::color::Transparent));
    s.drawObject(roo_display::Line(0, 1, 0, height() - 2, border));
    s.drawObject(roo_display::Line(0, height() - 1, 0, height() - 1,
                                   roo_display::color::Transparent));
    int16_t x = width() - 1;
    s.drawObject(
        roo_display::Line(x, 0, x, 0, roo_display::color::Transparent));
    s.drawObject(roo_display::Line(x, 1, x, height() - 2, border));
    s.drawObject(roo_display::Line(x, height() - 1, x, height() - 1,
                                   roo_display::color::Transparent));
  }

  int getActive() const { return active_; }

  void setActive(int index) {
    if (index == active_) return;
    active_ = index;
    for (int i = 0; i < buttons_.size(); i++) {
      buttons_[i]->setActivated(i == index);
    }
  }

 private:
  class ToggleButton : public roo_windows::Widget {
   public:
    ToggleButton(ToggleButtons* parent, Box bounds, const MonoIcon& icon)
        : roo_windows::Widget(parent, bounds), icon_(icon) {}

    bool useOverlayOnActivation() const override { return true; }
    bool isClickable() const override { return true; }

    void paint(const roo_display::Surface& s) override {
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
                             roo_display::HAlign::Center(),
                             roo_display::VAlign::Middle(),
                             roo_display::color::Transparent, s.fill_mode());
      s.drawObject(tile);
      s.drawObject(roo_display::Line(width() - 1, 0, width() - 1, height() - 3,
                                     external_border));
      s.drawObject(roo_display::FilledRect(0, height() - 2, width() - 1,
                                           height() - 1, external_border));
    }

   private:
    const ToggleButtons* parentGroup() const {
      return (const ToggleButtons*)parent();
    }

    const MonoIcon& icon_;
  };

  friend class ToggleButton;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  //   roo_display::VAlign alignment_;
  int active_;

  std::vector<ToggleButton*> buttons_;
};

}  // namespace roo_windows
