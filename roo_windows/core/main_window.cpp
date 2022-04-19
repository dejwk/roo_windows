#include "main_window.h"

#include <Arduino.h>

#include "roo_display/core/color.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_display/internal/hashtable.h"
#include "roo_windows/core/modal_window.h"
#include "roo_windows/core/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

static const unsigned long kClickAnimationMs = 200;

// How much movement in pixels before we consider it a swipe.
static const uint16_t kSwipeThreshold = 8;

MainWindow::MainWindow(const Environment& env, Display& display)
    : MainWindow(env, display, display.extents()) {}

namespace {

void maybeAddColor(roo_display::internal::ColorSet& palette, Color color) {
  if (palette.size() >= 15) return;
  palette.insert(color);
}

}  // namespace

MainWindow::MainWindow(const Environment& env, Display& display,
                       const Box& bounds)
    : Panel(env, env.theme().color.background),
      display_(display),
      theme_(env.theme()),
      touch_down_(false),
      redraw_bounds_(bounds),
      modal_window_(nullptr),
      background_fill_buffer_(display.width(), display.height()) {
  parent_bounds_ = bounds;
  invalidateDescending();
  roo_display::internal::ColorSet color_set;
  maybeAddColor(color_set, env.theme().color.background);
  maybeAddColor(color_set, env.theme().color.surface);
  maybeAddColor(color_set, env.theme().color.primary);
  maybeAddColor(color_set, env.theme().color.primaryVariant);
  maybeAddColor(color_set, env.theme().color.secondary);
  maybeAddColor(color_set, env.keyboardColorTheme().background);

  {
    Color c = env.theme().color.defaultColor(env.theme().color.surface);
    c.set_a(env.theme().pressAnimationOpacity(env.theme().color.surface));
    c = alphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }
  {
    Color c = env.theme().color.highlighterColor(env.theme().color.surface);
    c.set_a(env.theme().pressAnimationOpacity(env.theme().color.surface));
    c = alphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }

  maybeAddColor(color_set, env.keyboardColorTheme().normalButton);
  maybeAddColor(color_set, env.theme().color.error);
  maybeAddColor(color_set, env.keyboardColorTheme().modifierButton);
  Color palette[color_set.size()];
  std::copy(color_set.begin(), color_set.end(), palette);
  background_fill_buffer_.setPalette(palette, color_set.size());
}

class Adapter : public roo_display::Drawable {
 public:
  Adapter(MainWindow* window) : window_(window) {}

  roo_display::Box extents() const override { return window_->bounds(); }

  void drawTo(const roo_display::Surface& s) const override {
    window_->paintWindow(s);
  }

 private:
  MainWindow* window_;
};

void MainWindow::tick() {
  // Check if no events to process.
  int16_t x, y;
  bool down = display_.getTouch(x, y);
  unsigned long now = millis();
  click_animation_.tick();

  if (!touch_down_ && down) {
    // Pressed.
    touch_down_ = true;
    last_x_ = touch_x_ = x;
    last_y_ = touch_y_ = y;
    swipe_dx_ = swipe_dy_ = 0;
    last_time_ms_ = touch_time_ms_ = now;
    handleTouch(TouchEvent(TouchEvent::PRESSED, 0, x, y, x, y));
  } else if (touch_down_ && !down) {
    // Released.
    touch_down_ = false;
    handleTouch(TouchEvent(TouchEvent::RELEASED, now - touch_time_ms_, touch_x_,
                           touch_y_, last_x_, last_y_));
    if (swipe_dx_ * swipe_dx_ + swipe_dy_ * swipe_dy_ >
        kSwipeThreshold * kSwipeThreshold) {
      handleTouch(TouchEvent(TouchEvent::SWIPED, now - last_time_ms_,
                             last_x_ - swipe_dx_, last_y_ - swipe_dy_, last_x_,
                             last_y_));
    }
  } else if (down) {
    swipe_dx_ = x - last_x_;
    swipe_dy_ = y - last_y_;
    last_x_ = x;
    last_y_ = y;
    last_time_ms_ = now;
    handleTouch(TouchEvent(TouchEvent::DRAGGED, now - touch_time_ms_, touch_x_,
                           touch_y_, x, y));
  }

  if (isLayoutRequested()) {
    measure(MeasureSpec::Exactly(width()), MeasureSpec::Exactly(height()));
    layout(bounds());
  }

  roo_display::DrawingContext dc(display_);
  dc.draw(Adapter(this));
}

void MainWindow::paintWindow(const Surface& s) {
  if (!isDirty()) return;
  Surface news = s;
  news.clipToExtents(redraw_bounds_);
  redraw_bounds_ = Box(0, 0, -1, -1);
  news.set_fill_mode(roo_display::FILL_MODE_RECTANGLE);
  roo_display::BackgroundFillOptimizer bg_optimizer(s.out(),
                                                    background_fill_buffer_);
  Clipper clipper(clipper_state_, bg_optimizer);
  news.set_out(clipper.out());
  paintWidget(news, clipper);
}

void MainWindow::enterModal(ModalWindow* modal_window) {
  if (modal_window_ == modal_window) return;
  if (modal_window_ != nullptr) {
    modal_window_->exit();
  }
  modal_window_ = modal_window;
}

void MainWindow::exitModal(ModalWindow* modal_window) {
  if (modal_window_ == modal_window) {
    invalidateInterior(modal_window_->parent_bounds());
    modal_window_ = nullptr;
  }
}

void MainWindow::handleTouch(const TouchEvent& event) {
  if (modal_window_ != nullptr) {
    const auto& bounds = modal_window_->parent_bounds();
    TouchEvent shifted(event.type(), event.duration(),
                       event.startX() - bounds.xMin(),
                       event.startY() - bounds.yMin(),
                       event.x() - bounds.xMin(), event.y() - bounds.yMin());
    modal_window_->onTouch(shifted);
  } else if (xOffset() != 0 || yOffset() != 0) {
    TouchEvent shifted(event.type(), event.duration(),
                       event.startX() - xOffset(), event.startY() - yOffset(),
                       event.x() - xOffset(), event.y() - yOffset());
    onTouch(shifted);
  } else {
    onTouch(event);
  }
}

void MainWindow::propagateDirty(const Widget* child, const Box& box) {
  Box clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = Box::intersect(box, bounds());
  }
  markDirty(clipped);
  if (!clipped.empty()) {
    if (redraw_bounds_.empty()) {
      redraw_bounds_ = clipped;
    } else {
      redraw_bounds_ = Box::extent(redraw_bounds_, clipped);
    }
  }
}

}  // namespace roo_windows
