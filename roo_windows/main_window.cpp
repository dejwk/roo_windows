#include "main_window.h"

#include "roo_display/filter/foreground.h"
#include "roo_windows/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

inline int32_t dsquare(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

static const unsigned long kClickAnimationMs = 250;

MainWindow::MainWindow(Display* display)
    : MainWindow(display, display->extents()) {}

MainWindow::MainWindow(Display* display, const Box& bounds)
    : Panel(nullptr, bounds),
      display_(display),
      touch_down_(false),
      click_anim_target_(nullptr) {}

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

void MainWindow::tick() {
  // Check if no events to process.
  int16_t x, y;
  bool down = display_->getTouch(&x, &y);
  if (!touch_down_ && down) {
    // Pressed.
    touch_down_ = true;
    touch_x_ = x;
    touch_y_ = y;
    last_x_ = x;
    last_y_ = y;
    touch_time_ms_ = millis();
    onTouch(TouchEvent(TouchEvent::PRESSED, touch_time_ms_, x, y, x, y));
  } else if (touch_down_ && !down) {
    // Released.
    touch_down_ = false;
    onTouch(TouchEvent(TouchEvent::RELEASED, touch_time_ms_, touch_x_, touch_y_,
                       last_x_, last_y_));
  } else if (down) {
    last_x_ = x;
    last_y_ = y;
    onTouch(TouchEvent(TouchEvent::DRAGGED, touch_time_ms_, touch_x_, touch_y_,
                       x, y));
  }

  roo_display::DrawingContext dc(display_);
  dc.draw(Adapter(this));
}

bool MainWindow::animateClicked(Widget* target) {
  if (click_anim_target_ != nullptr) return false;
  click_anim_bounds_ = target->absolute_bounds();
  if (click_anim_target_ != nullptr || !touch_down_ ||
      touch_x_ < click_anim_bounds_.xMin() - kTouchMargin ||
      touch_x_ > click_anim_bounds_.xMax() + kTouchMargin ||
      touch_y_ < click_anim_bounds_.yMin() - kTouchMargin ||
      touch_y_ > click_anim_bounds_.yMax() + kTouchMargin) {
    return false;
  }
  int32_t ul = dsquare(touch_x_, touch_y_, click_anim_bounds_.xMin(),
                       click_anim_bounds_.yMin());
  int32_t ur = dsquare(touch_x_, touch_y_, click_anim_bounds_.xMax(),
                       click_anim_bounds_.yMin());
  int32_t dl = dsquare(touch_x_, touch_y_, click_anim_bounds_.xMin(),
                       click_anim_bounds_.yMax());
  int32_t dr = dsquare(touch_x_, touch_y_, click_anim_bounds_.xMax(),
                       click_anim_bounds_.yMax());
  int32_t max = 0;
  if (ul > max) max = ul;
  if (ur > max) max = ur;
  if (dl > max) max = dl;
  if (dr > max) max = dr;

  click_anim_target_ = target;
  click_anim_x_ = touch_x_;
  click_anim_y_ = touch_y_;
  click_anim_start_millis_ = millis();
  click_anim_max_radius_ = (int16_t)(sqrt(max) + 1);

  Color bg = click_anim_target_->background();
  click_anim_overlay_color_ =
      click_anim_target_->usesHighlighterColor()
          ? click_anim_target_->theme().color.highlighterColor(bg)
          : click_anim_target_->theme().color.defaultColor(bg);
  click_anim_overlay_color_.set_a(
      click_anim_target_->theme().pressAnimationOpacity(bg));
  return true;
}

void MainWindow::paint(const Surface& s) {
  if (click_anim_target_ != nullptr) {
    unsigned long elapsed = millis() - click_anim_start_millis_;
    double r = ((double)elapsed / kClickAnimationMs) * click_anim_max_radius_;
    click_anim_target_->invalidate();
    Surface news(s);
    PressOverlay overlay(click_anim_x_, click_anim_y_, (int16_t)r,
                         click_anim_overlay_color_);
    roo_display::ForegroundFilter filter(s.out(), &overlay);
    news.set_out(&filter);
    news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), background()));
    news.set_fill_mode(roo_display::FILL_MODE_RECTANGLE);
    news.set_clip_box(
        roo_display::Box::intersect(news.clip_box(), click_anim_bounds_));
    news.set_dx(news.dx() + click_anim_bounds_.xMin());
    news.set_dy(news.dy() + click_anim_bounds_.yMin());
    click_anim_target_->paint(news);
    if (elapsed >= kClickAnimationMs) {
      click_anim_target_->invalidate();
      click_anim_target_ = nullptr;
    }
  } else {
    if (isDirty()) {
      Panel::paint(s);
    }
  }
}

}  // namespace roo_windows
