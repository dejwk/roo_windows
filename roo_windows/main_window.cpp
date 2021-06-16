#include "main_window.h"

#include "roo_display/core/color.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/modal_window.h"
#include "roo_windows/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

inline int32_t dsquare(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

static const unsigned long kClickAnimationMs = 200;

MainWindow::MainWindow(Display* display)
    : MainWindow(display, display->extents()) {}

size_t fill_buffer_size(int16_t w, int16_t h) {
  auto window = roo_display::kBgFillOptimizerWindowSize;
  return (((w - 1) / window + 1) * ((h - 1) / window + 1) + 7) / 8;
}

MainWindow::MainWindow(Display* display, const Box& bounds)
    : Panel(nullptr, bounds),
      display_(display),
      touch_down_(false),
      click_anim_target_(nullptr),
      modal_window_(nullptr),
      background_fill_buffer_(
          new uint8_t[fill_buffer_size(display->width(), display->height())]),
      background_fill_(
          background_fill_buffer_.get(),
          (display->width() - 1) / roo_display::kBgFillOptimizerWindowSize + 1,
          (display->height() - 1) / roo_display::kBgFillOptimizerWindowSize +
              1),
      background_fill_optimizer_(nullptr) {
  memset(background_fill_buffer_.get(), 0,
         fill_buffer_size(display->width(), display->height()));
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
  bool down = display_->getTouch(&x, &y);
  if (!touch_down_ && down) {
    // Pressed.
    touch_down_ = true;
    touch_x_ = x;
    touch_y_ = y;
    last_x_ = x;
    last_y_ = y;
    touch_time_ms_ = millis();
    handleTouch(TouchEvent(TouchEvent::PRESSED, touch_time_ms_, x, y, x, y));
  } else if (touch_down_ && !down) {
    // Released.
    touch_down_ = false;
    handleTouch(TouchEvent(TouchEvent::RELEASED, touch_time_ms_, touch_x_,
                           touch_y_, last_x_, last_y_));
  } else if (down) {
    last_x_ = x;
    last_y_ = y;
    handleTouch(TouchEvent(TouchEvent::DRAGGED, touch_time_ms_, touch_x_,
                           touch_y_, x, y));
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

void MainWindow::paintWindow(const Surface& s) {
  Surface news = s;
  roo_display::BackgroundFillOptimizer bg_optimizer(s.out(), &background_fill_);
  background_fill_optimizer_ = &bg_optimizer;
  bg_optimizer.setBackground(background());
  news.set_out(&bg_optimizer);
  paint(news);
  background_fill_optimizer_ = nullptr;
}

void MainWindow::setCurrentBg(roo_display::Color bgcolor) {
  background_fill_optimizer_->setBackground(bgcolor);
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
      background_fill_.fillRect(
          Box(click_anim_bounds_.xMin() /
                  roo_display::kBgFillOptimizerWindowSize,
              click_anim_bounds_.yMin() /
                  roo_display::kBgFillOptimizerWindowSize,
              click_anim_bounds_.xMax() /
                  roo_display::kBgFillOptimizerWindowSize,
              click_anim_bounds_.yMax() /
                  roo_display::kBgFillOptimizerWindowSize),
          false);
      click_anim_target_->invalidate();
      click_anim_target_ = nullptr;
    }
  } else {
    if (isDirty()) {
      if (modal_window_ == nullptr) {
        Panel::paint(s);
      } else {
        // Exclude the modal window area from redraw.
        roo_display::Surface news(s);
        roo_display::DisplayOutput* out = news.out();
        Box exclusion(modal_window_->parent_bounds());
        roo_display::RectUnion ru(&exclusion, &exclusion + 1);
        roo_display::RectUnionFilter filter(s.out(), &ru);
        news.set_out(&filter);
        Panel::paint(news);
        news.set_out(out);
      }
    }
    if (modal_window_ != nullptr) {
      if (modal_window_->isDirty()) {
        Surface news = s;
        news.set_dx(s.dx() + modal_window_->parent_bounds().xMin());
        news.set_dy(s.dy() + modal_window_->parent_bounds().yMin());
        news.clipToExtents(modal_window_->bounds());
        news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), background()));
        modal_window_->paint(news);
      }
    }
  }
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
    invalidate(modal_window_->parent_bounds());
    modal_window_ = nullptr;
  }
}

void MainWindow::handleTouch(const TouchEvent& event) {
  if (modal_window_ != nullptr) {
    const auto& bounds = modal_window_->parent_bounds();
    TouchEvent shifted(event.type(), event.startTime(),
                       event.startX() - bounds.xMin(),
                       event.startY() - bounds.yMin(),
                       event.x() - bounds.xMin(), event.y() - bounds.yMin());
    modal_window_->onTouch(shifted);
  } else if (parent_bounds().xMin() != 0 || parent_bounds().yMin() != 0) {
    TouchEvent shifted(event.type(), event.startTime(),
                       event.startX() - parent_bounds().xMin(),
                       event.startY() - parent_bounds().yMin(),
                       event.x() - parent_bounds().xMin(),
                       event.y() - parent_bounds().yMin());
    onTouch(shifted);
  } else {
    onTouch(event);
  }
}

}  // namespace roo_windows
