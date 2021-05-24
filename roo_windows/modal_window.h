#pragma once

#include "roo_display.h"
#include "roo_display/shape/basic_shapes.h"
#include "roo_windows/main_window.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class ModalWindow : public Panel {
 public:
  ModalWindow(MainWindow* main_window, const Box& bounds)
      : Panel(nullptr, bounds), main_window_(main_window) {}

  MainWindow* getMainWindow() override { return main_window_; }
  const MainWindow* getMainWindow() const override { return main_window_; }

  void enter() {
    invalidate();
    main_window_->enterModal(this);
  }

  void exit() { main_window_->exitModal(this); }

  void paint(const roo_display::Surface& s) override {
    if (s.fill_mode() == roo_display::FILL_MODE_RECTANGLE || needs_repaint_) {
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
    Panel::paint(news);
  }

 private:
  MainWindow* main_window_;
};

}  // namespace roo_windows
