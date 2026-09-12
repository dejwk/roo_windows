#include "roo_windows/core/click_animation.h"

#include <Arduino.h>

#include "roo_windows/core/application.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

namespace {
constexpr uint32_t kClickFrameIntervalMillis = 20;
}  // namespace

void ClickAnimation::RequestFrame(Widget& target) {
  Application* app = target.getApplication();
  if (app != nullptr) app->requestAnimationFrameAt(roo_time::Uptime::Now());
}

roo_time::Uptime ClickAnimation::nextFrameDeadline() const {
  if (!isAnimationPending() || !target_->isClicking()) {
    return roo_time::Uptime::Max();
  }
  roo_time::Uptime now = roo_time::Uptime::Now();
  // Derive the deadline from the last sampled frame, not from this query.
  // Use the same wrapping millisecond clock as the retained click sample.
  uint32_t next_ms = static_cast<uint32_t>(click_anim_start_millis_) +
                     static_cast<uint32_t>(sampled_elapsed_millis_) +
                     kClickFrameIntervalMillis;
  int32_t remaining =
      static_cast<int32_t>(next_ms - static_cast<uint32_t>(now.inMillis()));
  return remaining <= 0 ? now
                        : now + roo_time::Millis(remaining) -
                              roo_time::Micros(now.inMicros() % 1000);
}

ClickAnimation::ClickAnimation()
    : target_(nullptr),
      phase_(Phase::kIdle),
      finishing_sampled_(false),
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

  RequestFrame(*target_);
  // The final paint used the pre-clear transient state. Invalidate its spill
  // once more so siblings underneath it are refreshed during settlement.
  invalidateTransientFootprint();
  if (phase_ == Phase::kAnimatingConfirmed ||
      phase_ == Phase::kFinishingConfirmed) {
    deliverClick();
  } else if (phase_ == Phase::kAnimatingDelivered ||
             phase_ == Phase::kFinishingDelivered ||
             phase_ == Phase::kFinishingUnconfirmed) {
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
  sampled_elapsed_millis_ = static_cast<uint32_t>(millis()) -
                            static_cast<uint32_t>(click_anim_start_millis_);
  if (isFinishing()) finishing_sampled_ = true;
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
  finishing_sampled_ = false;
  resetTransientFootprint();
  sampled_elapsed_millis_ = 0;
}

void ClickAnimation::deliverClick() {
  Widget* target = target_;
  RequestFrame(*target);
  // Release ownership before calling user code so a reentrant callback can
  // start another interaction.
  reset();
  target->invalidateInterior();
  target->onClicked();
}

float ClickAnimation::progress() const {
  if (target() == nullptr) return 1.0f;
  if (isFinishing() && finishing_sampled_) return 1.0f;
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
  RequestFrame(widget);
  return true;
}

void ClickAnimation::cancel(Widget& widget) {
  if (target_ != &widget) return;
  RequestFrame(widget);
  // A detached widget can be rebound later (for example, a reusable menu
  // row). Do not leave its visual click state behind after releasing the
  // shared controller.
  invalidateTransientFootprint();
  widget.clearClicking();
  reset();
  widget.invalidateInterior();
}

bool ClickAnimation::forceFinalFrame(const Widget& widget) {
  if (target_ != &widget) return false;
  // Finishing changes only the visual deadline. Separate phases retain whether
  // notifyRefreshCompleted() must deliver, settle an already-delivered action,
  // or discard an interaction that was never confirmed.
  switch (phase_) {
    case Phase::kAnimatingUnconfirmed:
      phase_ = Phase::kFinishingUnconfirmed;
      break;
    case Phase::kAnimatingConfirmed:
    case Phase::kFinishingConfirmed:
      phase_ = Phase::kFinishingConfirmed;
      break;
    case Phase::kAnimatingDelivered:
    case Phase::kFinishingDelivered:
      phase_ = Phase::kFinishingDelivered;
      break;
    case Phase::kFinishingUnconfirmed:
      break;
    case Phase::kIdle:
    case Phase::kAwaitingRelease:
    case Phase::kAwaitingRefresh:
      return false;
  }
  // A forced finish is sampled immediately unless a logical paint still owns
  // the old value. In that case the next new frame applies the finishing phase.
  const MainWindow* window = widget.getMainWindow();
  if (window == nullptr || !window->hasPaintContinuation()) {
    finishing_sampled_ = true;
  }
  target_->invalidateInterior();
  RequestFrame(*target_);
  return true;
}

bool ClickAnimation::tryConfirm(Widget& widget, ClickActivationPolicy policy) {
  if (phase_ == Phase::kIdle) {
    if (policy == ClickActivationPolicy::kAfterRefreshNoAnimation) {
      target_ = &widget;
      phase_ = Phase::kAwaitingRefresh;
      RequestFrame(widget);
      return true;
    }
    if (policy == ClickActivationPolicy::kImmediateNoAnimation) {
      RequestFrame(widget);
      widget.invalidateInterior();
      widget.onClicked();
      return true;
    }
    return false;
  }
  if (target_ != &widget) return false;

  if (phase_ == Phase::kAnimatingUnconfirmed) {
    if (policy == ClickActivationPolicy::kAfterRefreshNoAnimation ||
        policy == ClickActivationPolicy::kImmediateNoAnimation)
      return false;
    RequestFrame(widget);
    switch (policy) {
      case ClickActivationPolicy::kAfterNaturalAnimation:
        phase_ = Phase::kAnimatingConfirmed;
        return true;
      case ClickActivationPolicy::kAfterForcedFinalFrame:
        phase_ = Phase::kAnimatingConfirmed;
        return forceFinalFrame(widget);
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
         phase_ == Phase::kFinishingUnconfirmed ||
         phase_ == Phase::kFinishingConfirmed ||
         phase_ == Phase::kAnimatingDelivered ||
         phase_ == Phase::kFinishingDelivered ||
         phase_ == Phase::kAwaitingRefresh;
}

}  // namespace roo_windows
