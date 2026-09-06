#include "roo_windows/core/click_animation.h"

#include <Arduino.h>

#include "roo_windows/core/widget.h"

namespace roo_windows {

ClickAnimation::ClickAnimation()
    : target_(nullptr),
      phase_(Phase::kIdle),
      previous_transient_footprint_(0, 0, -1, -1),
      click_anim_start_millis_(0),
      sampled_elapsed_millis_(0),
      click_anim_x_(0),
      click_anim_y_(0) {}

void ClickAnimation::tick() {
  sampleFrameTime();
  if (!isAnimationPending() || !target_->isClicking()) return;
  target_->invalidateInterior();
  invalidateTransientFootprint();
  if (sampled_elapsed_millis_ > kPressAnimationMillis + 100 &&
      phase_ == Phase::kAnimatingUnconfirmed) {
    // 100 ms is a grace period to allow the widget to draw the full click
    // state and then mark itself as non-clicking. If the widget is dragging
    // its feet, it may mean it became invisible or clipped out and is not
    // refreshing anymore. An unconfirmed animation can be retired safely.
    // A confirmed animation must instead retain its full overlay until a
    // completed paint clears it; otherwise the target can disappear before
    // its deferred selection is delivered.
    target_->clearClicking();
  }
}

void ClickAnimation::notifyRefreshCompleted() {
  if (phase_ == Phase::kAwaitingRefresh) {
    deliverClick();
    return;
  }
  if (!isAnimationPending() || target_->isClicking()) return;

  // The final paint used the pre-clear transient state. Invalidate its spill
  // once more so siblings underneath it are refreshed during settlement.
  invalidateTransientFootprint();
  if (phase_ == Phase::kAnimatingConfirmed ||
      phase_ == Phase::kFinishingConfirmed) {
    deliverClick();
  } else if (phase_ == Phase::kAnimatingDelivered) {
    Widget* target = target_;
    reset();
    target->invalidateInterior();
  } else if (target_->isPressed()) {
    target_->invalidateInterior();
    phase_ = Phase::kAwaitingRelease;
    resetTransientFootprint();
  } else {
    Widget* target = target_;
    reset();
    target->invalidateInterior();
  }
}

void ClickAnimation::sampleFrameTime() {
  if (!isAnimationPending()) return;
  sampled_elapsed_millis_ = millis() - click_anim_start_millis_;
}

void ClickAnimation::invalidateTransientFootprint() {
  Rect current = target_->getParentTransientPaintBounds();
  Rect repaint = previous_transient_footprint_.empty()
                     ? current
                     : Rect::Extent(previous_transient_footprint_, current);
  if (repaint != target_->parent_bounds()) {
    target_->notifyParentInvalidatedRegion(repaint);
  }
  previous_transient_footprint_ = current;
}

void ClickAnimation::resetTransientFootprint() {
  previous_transient_footprint_ = Rect(0, 0, -1, -1);
}

void ClickAnimation::reset() {
  target_ = nullptr;
  phase_ = Phase::kIdle;
  resetTransientFootprint();
  sampled_elapsed_millis_ = 0;
}

void ClickAnimation::deliverClick() {
  Widget* target = target_;
  // Release ownership before calling user code so a reentrant callback can
  // start another interaction.
  reset();
  target->invalidateInterior();
  target->onClicked();
}

float ClickAnimation::progress() const {
  if (target() == nullptr) return 1.0f;
  if (phase_ == Phase::kFinishingConfirmed) return 1.0f;
  float result = (float)sampled_elapsed_millis_ / kPressAnimationMillis;
  if (result > 1.0f) result = 1.0f;
  return result;
}

int16_t ClickAnimation::xCenter() const { return click_anim_x_; }

int16_t ClickAnimation::yCenter() const { return click_anim_y_; }

const Widget* ClickAnimation::target() const {
  return isAnimationPending() || phase_ == Phase::kAwaitingRelease ? target_
                                                                   : nullptr;
}

bool ClickAnimation::tryStart(Widget& widget, int16_t x, int16_t y) {
  if (isBusy()) return false;
  target_ = &widget;
  phase_ = Phase::kAnimatingUnconfirmed;
  click_anim_start_millis_ = millis();
  click_anim_x_ = x;
  click_anim_y_ = y;
  resetTransientFootprint();
  sampled_elapsed_millis_ = 0;
  return true;
}

void ClickAnimation::cancel(Widget& widget) {
  if (target_ != &widget) return;
  // A detached widget can be rebound later (for example, a reusable menu
  // row). Do not leave its visual click state behind after releasing the
  // shared controller.
  invalidateTransientFootprint();
  widget.clearClicking();
  reset();
  widget.invalidateInterior();
}

bool ClickAnimation::tryConfirm(Widget& widget,
                                ClickActivationPolicy policy) {
  if (phase_ == Phase::kIdle) {
    if (policy == ClickActivationPolicy::kAfterRefreshNoAnimation) {
      target_ = &widget;
      phase_ = Phase::kAwaitingRefresh;
      return true;
    }
    if (policy == ClickActivationPolicy::kImmediateNoAnimation) {
      widget.invalidateInterior();
      widget.onClicked();
      return true;
    }
    return false;
  }
  if (target_ != &widget) return false;

  if (phase_ == Phase::kAnimatingUnconfirmed) {
    switch (policy) {
      case ClickActivationPolicy::kAfterNaturalAnimation:
        phase_ = Phase::kAnimatingConfirmed;
        return true;
      case ClickActivationPolicy::kAfterForcedFinalFrame:
        phase_ = Phase::kFinishingConfirmed;
        target_->invalidateInterior();
        return true;
      case ClickActivationPolicy::kImmediateCancelAnimation:
        invalidateTransientFootprint();
        widget.clearClicking();
        reset();
        widget.invalidateInterior();
        widget.onClicked();
        return true;
      case ClickActivationPolicy::kImmediateContinueAnimation:
        phase_ = Phase::kAnimatingDelivered;
        widget.onClicked();
        return true;
      case ClickActivationPolicy::kAfterRefreshNoAnimation:
      case ClickActivationPolicy::kImmediateNoAnimation:
        return false;
    }
  }

  if (phase_ == Phase::kAwaitingRelease) {
    deliverClick();
    return true;
  }

  // A matching target is already confirmed; competing targets were rejected
  // above without mutating the interaction.
  return phase_ == Phase::kAnimatingConfirmed ||
         phase_ == Phase::kFinishingConfirmed ||
         phase_ == Phase::kAnimatingDelivered ||
         phase_ == Phase::kAwaitingRefresh;
}

}  // namespace roo_windows
