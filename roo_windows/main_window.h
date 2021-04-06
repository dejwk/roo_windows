#pragma once

#include "roo_display.h"
#include "roo_windows/panel.h"

namespace roo_windows {

class MainWindow : public Panel {
 public:
  MainWindow(roo_display::Display* display)
      : MainWindow(display, display->extents()) {}

  MainWindow(roo_display::Display* display, const Box& bounds)
      : Panel(nullptr, bounds), display_(display), touch_down_(false) {}

  void tick() {
    // Check if no events to process.
    int16_t x, y;
    bool down = display_->getTouch(&x, &y);
    if (!touch_down_ && down) {
      // Pressed.
      onTouch(TouchEvent(TouchEvent::DOWN, x, y));
    } else if (touch_down_ && !down) {
      // Released.
      onTouch(TouchEvent(TouchEvent::UP, touch_x_, touch_y_));
    } else if (down) {
      onTouch(TouchEvent(TouchEvent::MOVE, x, y));
    }
    touch_x_ = x;
    touch_y_ = y;

    roo_display::DrawingContext dc(display_);
    dc.draw(Adapter(this));
  }

 private:
  class Adapter : public roo_display::Drawable {
   public:
    Adapter(Widget* widget) : widget_(widget) {}

    roo_display::Box extents() const override { return widget_->bounds(); }

    void drawTo(const roo_display::Surface& s) const override {
      widget_->paint(s);
    }

   private:
    Widget* widget_;
  };

  roo_display::Display* display_;
  int16_t touch_x_, touch_y_;
  bool touch_down_;
};

}  // namespace roo_windows
