#include "roo_windows/core/click_animation.h"

#include <Arduino.h>

#include "roo_windows/core/widget.h"

namespace roo_windows {

static const unsigned long kClickAnimationMs = 200;

ClickAnimation::ClickAnimation()
    : click_anim_target_(nullptr),
      click_confirmed_(false),
      deferred_click_(nullptr) {}

void ClickAnimation::tick() {
  unsigned long now = millis();
  // If an in-progress click animation is expired, clear the animation target so
  // that other widgets can be clicked, and possibly deliver the delayed click
  // notification. This is done after the overall redraw, so that the click
  // animation target has a chance to fully redraw itself after the click
  // animation complated but before the click notification is delivered.
  if (click_anim_target_ != nullptr &&
      now - click_anim_start_millis_ >= kClickAnimationMs &&
      !click_anim_target_->isClicking()) {
    if (click_confirmed_) {
      click_confirmed_ = false;
      clickWidget(click_anim_target_);
    }
    click_anim_target_ = nullptr;
  }

  if (deferred_click_ != nullptr) {
    // We want to delier click only after the widget has been released and is no
    // longer animating. This way, the visual updates of the widget and its
    // resulting actions are distinct. This makes the widget feel more snappy,
    // and reduces the redraw area (by splitting the update into smaller
    // updates).
    if (!deferred_click_->isPressed() && !deferred_click_->isDirty()) {
      deferred_click_->onClicked();
      deferred_click_ = nullptr;
      click_confirmed_ = false;
    }
  }
}

bool ClickAnimation::isClickAnimating() const {
  return click_anim_target_ != nullptr;
}

bool ClickAnimation::isClickConfirmed() const {
  return click_confirmed_;
}

void ClickAnimation::start(Widget* widget, int16_t x, int16_t y) {
  click_anim_target_ = widget;
  click_anim_start_millis_ = millis();
  click_anim_x_ = x;
  click_anim_y_ = y;
  click_confirmed_ = false;
  // gesture_detector_.startClickAnimation();
}

void ClickAnimation::confirmClick(Widget* widget) {
  click_confirmed_ = true;
  if (click_anim_target_ == nullptr) {
    deferred_click_ = widget;
  }
}

bool ClickAnimation::getProgress(const Widget* target, float* progress,
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

}  // namespace roo_windows