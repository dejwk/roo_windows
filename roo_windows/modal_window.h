#pragma once

#include "roo_display.h"
#include "roo_windows/panel.h"
#include "roo_windows/main_window.h"

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

  void exit() {
    main_window_->exitModal(this);
  }

 private:
  MainWindow* main_window_;
};

}  // namespace roo_windows
