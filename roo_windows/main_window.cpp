#include "main_window.h"

#include "roo_display/core/color.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_display/internal/hashtable.h"
#include "roo_windows/modal_window.h"
#include "roo_windows/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

static const unsigned long kClickAnimationMs = 200;

// How much movement in pixels before we consider it a swipe.
static const uint16_t kSwipeThreshold = 8;

MainWindow::MainWindow(Display* display)
    : MainWindow(display, display->extents()) {}

namespace {

void maybeAddColor(roo_display::internal::ColorSet& palette, Color color) {
  if (palette.size() >= 15) return;
  palette.insert(color);
}

}  // namespace

MainWindow::MainWindow(Display* display, const Box& bounds)
    : Panel(nullptr, bounds),
      display_(display),
      touch_down_(false),
      click_anim_target_(nullptr),
      modal_window_(nullptr),
      background_fill_buffer_(display->width(), display->height()) {
  roo_display::internal::ColorSet color_set;
  const auto& theme_colors = theme().color;
  maybeAddColor(color_set, theme_colors.background);
  maybeAddColor(color_set, theme_colors.surface);
  maybeAddColor(color_set, theme_colors.primary);
  maybeAddColor(color_set, theme_colors.primaryVariant);
  maybeAddColor(color_set, theme_colors.secondary);
  maybeAddColor(color_set, theme_colors.error);
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
  bool down = display_->getTouch(&x, &y);
  unsigned long now = millis();

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

  roo_display::DrawingContext dc(display_);
  dc.draw(Adapter(this));

  // If an in-progress click animation is expired, clear the animation target so
  // that other widgets can be clicked, and possibly deliver the delayed click
  // notification. This is done after the overall redraw, so that the click
  // animation target has a chance to fully redraw itself after the click
  // animation complated but before the click notification is delivered.
  if (click_anim_target_ != nullptr &&
      now - click_anim_start_millis_ >= kClickAnimationMs &&
      !click_anim_target_->isClicking()) {
    if (click_anim_target_->isClicked()) {
      click_anim_target_->clearClicked();
      click_anim_target_->onClicked();
    }
    click_anim_target_ = nullptr;
  }
}

bool MainWindow::isClickAnimating() const {
  return click_anim_target_ != nullptr;
}

void MainWindow::startClickAnimation(Widget* widget) {
  click_anim_target_ = widget;
  click_anim_start_millis_ = millis();
  click_anim_x_ = touch_x_;
  click_anim_y_ = touch_y_;
}

bool MainWindow::getClick(const Widget* target, float* progress,
                          int16_t* x_center, int16_t* y_center) const {
  if (click_anim_target_ != target) {
    return false;
  }
  *progress = (float)(millis() - click_anim_start_millis_) / kClickAnimationMs;
  if (*progress > 1.0) *progress = 1.0;
  *x_center = click_anim_x_;
  *y_center = click_anim_y_;
  return true;
}

void MainWindow::paintWindow(const Surface& s) {
  Surface news = s;
  news.set_fill_mode(roo_display::FILL_MODE_RECTANGLE);
  roo_display::BackgroundFillOptimizer bg_optimizer(s.out(),
                                                    &background_fill_buffer_);
  news.set_out(&bg_optimizer);
  paint(news);
}

void MainWindow::paint(const Surface& s) {
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
    TouchEvent shifted(event.type(), event.duration(),
                       event.startX() - bounds.xMin(),
                       event.startY() - bounds.yMin(),
                       event.x() - bounds.xMin(), event.y() - bounds.yMin());
    modal_window_->onTouch(shifted);
  } else if (parent_bounds().xMin() != 0 || parent_bounds().yMin() != 0) {
    TouchEvent shifted(
        event.type(), event.duration(), event.startX() - parent_bounds().xMin(),
        event.startY() - parent_bounds().yMin(),
        event.x() - parent_bounds().xMin(), event.y() - parent_bounds().yMin());
    onTouch(shifted);
  } else {
    onTouch(event);
  }
}

}  // namespace roo_windows
