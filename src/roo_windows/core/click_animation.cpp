#include "roo_windows/core/click_animation.h"

#include <Arduino.h>

#include "roo_windows/core/widget.h"

namespace roo_windows {

ClickAnimation::ClickAnimation()
    : click_anim_target_(nullptr),
      click_confirmed_(false),
      deferred_click_(nullptr),
      has_prev_transient_bounds_(false),
      prev_transient_x0_(0),
      prev_transient_y0_(0),
      prev_transient_x1_(-1),
      prev_transient_y1_(-1),
      click_anim_start_millis_(0),
      sampled_elapsed_millis_(0),
      awaiting_release_(false),
      click_anim_x_(0),
      click_anim_y_(0) {}

void ClickAnimation::tick() {
  sampleFrameTime();
  if (click_anim_target_ != nullptr && click_anim_target_->isClicking()) {
    click_anim_target_->invalidateInterior();
    Rect transient_bounds = click_anim_target_->getParentTransientPaintBounds();
    Rect repaint_bounds = transient_bounds;
    if (has_prev_transient_bounds_) {
      repaint_bounds = Rect::Extent(
          repaint_bounds, Rect(prev_transient_x0_, prev_transient_y0_,
                               prev_transient_x1_, prev_transient_y1_));
    }
    if (repaint_bounds != click_anim_target_->parent_bounds()) {
      click_anim_target_->notifyParentInvalidatedRegion(repaint_bounds);
    }
    has_prev_transient_bounds_ = true;
    prev_transient_x0_ = transient_bounds.xMin();
    prev_transient_y0_ = transient_bounds.yMin();
    prev_transient_x1_ = transient_bounds.xMax();
    prev_transient_y1_ = transient_bounds.yMax();
    if (sampled_elapsed_millis_ > kPressAnimationMillis + 100 &&
        !click_confirmed_) {
      // 100 ms is a grace period to allow the widget to draw the full click
      // state and then mark itself as non-clicking. If the widget is dragging
      // its feet, it may mean it became invisible or clipped out and is not
      // refreshing anymore. An unconfirmed animation can be retired safely.
      // A confirmed animation must instead retain its full overlay until a
      // completed paint clears it; otherwise the target can disappear while
      // its deferred selection is still waiting for the widget to become
      // clean.
      click_anim_target_->clearClicking();
    }
  }

  // If an in-progress click animation is expired, clear the animation target so
  // that other widgets can be clicked, and possibly deliver the delayed click
  // notification. This is done after the overall redraw, so that the click
  // animation target has a chance to fully redraw itself after the click
  // animation completed but before the click notification is delivered.
  if (click_anim_target_ != nullptr &&
      sampled_elapsed_millis_ >= kPressAnimationMillis &&
      !click_anim_target_->isClicking()) {
    // A finished held press stays attached after its one settlement
    // invalidation. This lets a release coalesce its state change into that
    // repaint instead of first drawing the old state without an overlay.
    if (awaiting_release_) return;

    // The final paint frame may still use the pre-clear transient state.
    // Invalidate the full transient spill region once more before delivering
    // the deferred click so siblings underneath that spill are refreshed.
    Rect transient_bounds = click_anim_target_->getParentTransientPaintBounds();
    Rect repaint_bounds = transient_bounds;
    if (has_prev_transient_bounds_) {
      repaint_bounds = Rect::Extent(
          repaint_bounds, Rect(prev_transient_x0_, prev_transient_y0_,
                               prev_transient_x1_, prev_transient_y1_));
    }
    if (repaint_bounds != click_anim_target_->parent_bounds()) {
      click_anim_target_->notifyParentInvalidatedRegion(repaint_bounds);
    }
    Widget* target = click_anim_target_;
    if (!click_confirmed_ && target->isPressed()) {
      target->invalidateInterior();
      awaiting_release_ = true;
      has_prev_transient_bounds_ = false;
      return;
    } else if (click_confirmed_) {
      click_confirmed_ = false;
      clickWidget(target);
    } else {
      // An unconfirmed target which is no longer pressed has no deferred action
      // to schedule its settlement frame.
      target->invalidateInterior();
    }
    click_anim_target_ = nullptr;
    has_prev_transient_bounds_ = false;
  }

  if (deferred_click_ != nullptr) {
    // We want to deliver click only after the widget has been released and is
    // no longer animating. This way, the visual updates of the widget and its
    // resulting actions are distinct. This makes the widget feel more snappy,
    // and reduces the redraw area (by splitting the update into smaller
    // updates).
    if (!deferred_click_->isPressed() && !deferred_click_->isDirty()) {
      Widget* target = deferred_click_;

      // The final animation frame was computed while the click overlay was
      // still active. Invalidate immediately before invoking the action so
      // there is exactly one subsequent paint: state-changing controls draw
      // their new state directly, while unchanged controls still restore their
      // normal foreground and the background between its pixels.
      deferred_click_ = nullptr;
      click_confirmed_ = false;
      target->invalidateInterior();
      target->onClicked();
    }
  }
}

void ClickAnimation::sampleFrameTime() {
  if (click_anim_target_ == nullptr) return;
  sampled_elapsed_millis_ = millis() - click_anim_start_millis_;
}

bool ClickAnimation::isClickAnimating() const {
  return click_anim_target_ != nullptr;
}

float ClickAnimation::progress() const {
  if (click_anim_target_ == nullptr) return 1.0f;
  float result = (float)sampled_elapsed_millis_ / kPressAnimationMillis;
  if (result > 1.0f) result = 1.0f;
  return result;
}

int16_t ClickAnimation::xCenter() const { return click_anim_x_; }

int16_t ClickAnimation::yCenter() const { return click_anim_y_; }

const Widget* ClickAnimation::target() const { return click_anim_target_; }

bool ClickAnimation::isClickConfirmed() const { return click_confirmed_; }

void ClickAnimation::start(Widget* widget, int16_t x, int16_t y) {
  click_anim_target_ = widget;
  click_anim_start_millis_ = millis();
  click_anim_x_ = x;
  click_anim_y_ = y;
  click_confirmed_ = false;
  has_prev_transient_bounds_ = false;
  sampled_elapsed_millis_ = 0;
  awaiting_release_ = false;
}

void ClickAnimation::cancel() {
  click_anim_target_ = nullptr;
  has_prev_transient_bounds_ = false;
  sampled_elapsed_millis_ = 0;
  awaiting_release_ = false;
}

void ClickAnimation::confirmClick(Widget* widget) {
  if (click_anim_target_ == widget && !widget->isClicking() &&
      sampled_elapsed_millis_ >= kPressAnimationMillis) {
    // The final animated frame has already been emitted. Merge the click's
    // visual state change into the next settlement frame, whether retirement
    // has run already or is still pending.
    click_anim_target_ = nullptr;
    click_confirmed_ = false;
    has_prev_transient_bounds_ = false;
    awaiting_release_ = false;
    widget->invalidateInterior();
    widget->onClicked();
    return;
  }
  click_confirmed_ = true;
  if (click_anim_target_ == nullptr) {
    deferred_click_ = widget;
  }
}

}  // namespace roo_windows
