#include "scrollable_panel.h"

#include <Arduino.h>

namespace roo_windows {

void ScrollablePanel::scrollTo(int16_t x, int16_t y) {
  Widget* c = contents();
  if (c->width() >= width()) {
    if (x > 0) x = 0;
    if (x < width() - c->width()) {
      x = width() - c->width();
    }
  } else {
    if (x < 0) x = 0;
    if (x > width() - c->width()) x = width() - c->width();
  }
  if (c->height() >= height()) {
    if (y > 0) y = 0;
    if (y < height() - c->height()) {
      y = height() - c->height();
    }
  } else {
    if (y < 0) y = 0;
    if (y > height() - c->height()) y = height() - c->height();
  }
  c->moveTo(c->bounds().translate(x, y));
}

void ScrollablePanel::paintWidgetContents(const Surface& s, Clipper& clipper) {
  bool scroll_in_progress = (scroll_start_vx_ != 0 || scroll_start_vy_ != 0);
  // If scroll in progress, take it into account.
  if (scroll_in_progress) {
    Widget* c = contents();
    int16_t drag_x_delta = 0;
    int16_t drag_y_delta = 0;

    // Calculate the total offset to be moved.
    unsigned long t_end = millis();
    if ((long)(t_end - scroll_end_time_) >= 0) {
      // The time to full stop elapsed; the scroll is certainly over now.
      t_end = scroll_end_time_;
      scroll_in_progress = false;
    }
    // Now, a little bit of physics. We calculate total distance traveled
    // by the scrolled content, as S = vt * at^2 (where a < 0), seperately
    // for the horizontal and vertical components.
    // Note that when the scroll gets abruptly terminated in either
    // horizontal or vertical direction because of bumping into a boundary,
    // the corresponding start velocity is set to zero.
    float t = (t_end - scroll_start_time_) / 1000.0;
    if (scroll_start_vx_ != 0) {
      // The scrolling continues horizontally.
      int32_t scroll_x_total =
          (int32_t)(scroll_start_vx_ * t + scroll_decel_x_ * t * t / 2);
      drag_x_delta = scroll_x_total - (c->xOffset() - dxStart_);
      // Don't move outside the boundary.
      if (drag_x_delta < -c->xOffset() + width() - c->width()) {
        drag_x_delta = -c->xOffset() + width() - c->width();
        scroll_start_vx_ = 0;
      }
      if (drag_x_delta > -c->xOffset()) {
        drag_x_delta = -c->xOffset();
        scroll_start_vx_ = 0;
      }
    }
    if (scroll_start_vy_ != 0) {
      // The scrolling continues vertically.
      int32_t scroll_y_total =
          (int32_t)(scroll_start_vy_ * t + scroll_decel_y_ * t * t / 2);
      drag_y_delta = scroll_y_total - (c->yOffset() - dyStart_);
      // Don't move outside the boundary.
      if (drag_y_delta < -c->yOffset() + height() - c->height()) {
        drag_y_delta = -c->yOffset() + height() - c->height();
        scroll_start_vy_ = 0;
      }
      if (drag_y_delta > -c->yOffset()) {
        drag_y_delta = -c->yOffset();
        scroll_start_vy_ = 0;
      }
    }
    scrollBy(drag_x_delta, drag_y_delta);

    // If we're done, e.g. due to bumping against both horizontal and
    // vertical boundaries, then stop refreshing the view.
    if (scroll_start_vx_ == 0 && scroll_start_vy_ == 0) {
      scroll_in_progress = false;
    }
    if (scroll_in_progress == false) {
      scroll_start_vx_ = scroll_start_vy_ = 0;
    }
  }
  Panel::paintWidgetContents(s, clipper);

  if (scroll_in_progress) {
    // TODO: use a scheduler to cap the frequency of invalidation,
    // to something like 50-60 fps.
    invalidateInterior();
  }
}

bool ScrollablePanel::onTouch(const TouchEvent& event) {
  Widget* c = contents();
  // Stop the scroll.
  scroll_start_vx_ = 0;
  scroll_start_vy_ = 0;
  if (Panel::onTouch(event)) return true;
  if (event.type() == TouchEvent::RELEASED ||
      event.type() == TouchEvent::PRESSED) {
    dxStart_ = c->xOffset();
    dyStart_ = c->yOffset();
  }
  // Handle drag and swipe.
  if (event.type() == TouchEvent::DRAGGED) {
    int16_t drag_x_delta = event.dx() - (c->xOffset() - dxStart_);
    int16_t drag_y_delta = event.dy() - (c->yOffset() - dyStart_);
    // To reduce flicker on random noise, ignore very small drags.
    if (drag_x_delta < 3 && drag_x_delta > -3 && drag_y_delta < 3 &&
        drag_y_delta > -3) {
      return true;
    }
    scrollBy(drag_x_delta, drag_y_delta);
    return true;
  } else if (event.type() == TouchEvent::SWIPED) {
    scroll_start_time_ = millis();
    // Capture the initial velocity of the scroll.
    scroll_start_vx_ = 1000.0 * event.dx() / (float)event.duration();
    scroll_start_vy_ = 1000.0 * event.dy() / (float)event.duration();
    // If we're already on the boundary and swiping outside of the bounded
    // region, cap the horizontal and/or vertical component of the scroll.
    if (scroll_start_vx_ < 0 && c->xOffset() == 0) {
      scroll_start_vx_ = 0;
    }
    if (scroll_start_vx_ > 0 && -c->xOffset() + width() == c->width()) {
      scroll_start_vx_ = 0;
    }
    if (scroll_start_vy_ < 0 && c->yOffset() == 0) {
      scroll_start_vy_ = 0;
    }
    if (scroll_start_vy_ > 0 && -c->yOffset() + height() == c->height()) {
      scroll_start_vy_ = 0;
    }
    // If the swipe is completely in the outside direction, ignore it.
    if (scroll_start_vx_ == 0 && scroll_start_vy_ == 0) return true;
    // Calculate the length of the initial velocity vector.
    float v_abs = sqrtf(scroll_start_vx_ * scroll_start_vx_ +
                        scroll_start_vy_ * scroll_start_vy_);
    // Cap that scroll velocity if too large.
    if (v_abs > maxVel) {
      float mult = maxVel / v_abs;
      scroll_start_vx_ *= mult;
      scroll_start_vy_ *= mult;
      v_abs = maxVel;
    }
    // Scroll is constantly deccelerating (with configured const
    // decceleration). Calculate the total scroll time until it stops on
    // its own if uninterrupted.
    unsigned long scroll_duration =
        (unsigned long)(1000.0 * v_abs / decceleration);
    scroll_end_time_ = scroll_start_time_ + scroll_duration;
    // Capture horizontal and vertical components of the decceleration.
    scroll_decel_x_ = -decceleration * scroll_start_vx_ / v_abs;
    scroll_decel_y_ = -decceleration * scroll_start_vy_ / v_abs;
    invalidateInterior();
    return true;
  }
}

}  // namespace roo_windows
