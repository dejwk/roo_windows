#pragma once

#include "roo_display.h"
#include "roo_display/shape/basic_shapes.h"
#include "roo_windows/main_window.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class ModalWindow : public Panel {
 public:
  ModalWindow(Panel* parent, const Box& bounds) : Panel(parent, bounds) {}

  void enter() {
    setVisible(true);
    getMainWindow()->enterModal(this);
  }

  void exit() {
    setVisible(false);
    getMainWindow()->exitModal(this);
  }

  void paintWidget(const roo_display::Surface& s) override {
    if (isVisible() && needs_repaint_) {
      Color color = theme().color.defaultColor(s.bgcolor());
      color.set_a(0x20);
      s.drawObject(roo_display::FilledRect(0, 0, width() - 1, 1, color));
      s.drawObject(roo_display::FilledRect(0, 2, 1, height() - 4, color));
      color.set_a(0x40);
      s.drawObject(roo_display::FilledRect(width() - 2, 2, width() - 1,
                                           height() - 4, color));
      s.drawObject(roo_display::FilledRect(0, height() - 3, width() - 1,
                                           height() - 1, color));
    }
    roo_display::Surface news(s);
    news.clipToExtents(Box(2, 2, width() - 3, height() - 4));
    Panel::paintWidget(news);
  }
};

}  // namespace roo_windows
