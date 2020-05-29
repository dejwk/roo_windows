#pragma once

#include <memory>

#include "roo_windows/widget.h"

namespace roo_windows {

class T : public Drawable {
 public:
  T(Widget* widget) : widget_(widget) {}

  roo_display::Box extents() const override {
    return widget_->bounds();
  }

  void drawTo(const roo_display::Surface& s) const override {
    widget_->update(s);
  }

 private:
  Widget* widget_;
};

class WindowManager {
 public:
  WindowManager(roo_display::Display* display, Widget* main)
      : display_(display), main_(main) {}
  
  void tick() {
    // Check if no events to process.
    int16_t x, y;
    bool down = display_->getTouch(&x, &y);
    if (!touch_down_ && down) {
      // Pressed.
      main_->onTouch(TouchEvent(TouchEvent::DOWN, x, y));
    } else if (touch_down_ && !down) {
      // Released.
      main_->onTouch(TouchEvent(TouchEvent::UP, touch_x_, touch_y_));
    } else if (down) {
      main_->onTouch(TouchEvent(TouchEvent::MOVE, x, y));
    }
    touch_x_ = x;
    touch_y_ = y;

    roo_display::DrawingContext dc(display_);
    T t(main_.get());
    dc.draw(t);
  }

 private:
  roo_display::Display* display_;
  std::unique_ptr<Widget> main_;
  int16_t touch_x_, touch_y_;
  bool touch_down_;
};

}  // namespace roo_windows
