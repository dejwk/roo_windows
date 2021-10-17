#include "scrollable_panel.h"

namespace roo_windows {

// Sets the relative position of the underlying content, relative to the the
// visible rectangle.
void ScrollablePanel::setOffset(int16_t dx, int16_t dy) {
  if (dx > 0) dx = 0;
  if (dy > 0) dy = 0;
  if (dx < parent_bounds().width() - width_) {
    dx = parent_bounds().width() - width_;
  }
  if (dy < parent_bounds().height() - height_) {
    dy = parent_bounds().height() - height_;
  }
  if (dx != dx_ || dy_ != dy) invalidateInterior();
  dx_ = dx;
  dy_ = dy;
}

void ScrollablePanel::paintWidgetContents(const Surface& s) {
  bool scroll_in_progress = (scroll_start_vx_ != 0 || scroll_start_vy_ != 0);
  // If scroll in progress, take it into account.
  if (scroll_in_progress) {
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
      int16_t drag_x_delta = scroll_x_total - (dx_ - dxStart_);
      // Don't move outside the boundary.
      if (drag_x_delta < -dx_ + parent_bounds().width() - width_) {
        drag_x_delta = -dx_ + parent_bounds().width() - width_;
        scroll_start_vx_ = 0;
      }
      if (drag_x_delta > -dx_) {
        drag_x_delta = -dx_;
        scroll_start_vx_ = 0;
      }
      // Move!
      dx_ += drag_x_delta;
    }
    if (scroll_start_vy_ != 0) {
      // The scrolling continues vertically.
      int32_t scroll_y_total =
          (int32_t)(scroll_start_vy_ * t + scroll_decel_y_ * t * t / 2);
      int16_t drag_y_delta = scroll_y_total - (dy_ - dyStart_);
      // Don't move outside the boundary.
      if (drag_y_delta < -dy_ + parent_bounds().height() - height_) {
        drag_y_delta = -dy_ + parent_bounds().height() - height_;
        scroll_start_vy_ = 0;
      }
      if (drag_y_delta > -dy_) {
        drag_y_delta = -dy_;
        scroll_start_vy_ = 0;
      }
      dy_ += drag_y_delta;
    }
    // If we're done, e.g. due to bumping against both horizontal and
    // vertical boundaries, then stop refreshing the view.
    if (scroll_start_vx_ == 0 && scroll_start_vy_ == 0) {
      scroll_in_progress = false;
    }
    if (scroll_in_progress == false) {
      scroll_start_vx_ = scroll_start_vy_ = 0;
    }
  }
  Surface news = s;
  news.set_dx(news.dx() + dx_);
  news.set_dy(news.dy() + dy_);
  invalid_region_ = invalid_region_.translate(-dx_, -dy_);
  Panel::paintWidgetContents(news);
  if (scroll_in_progress) {
    // TODO: use a scheduler to cap the frequency of invalidation,
    // to something like 50-60 fps.
    invalidateInterior();
  }
}

void ScrollablePanel::getAbsoluteBounds(Box* full, Box* visible) const {
  Panel::getAbsoluteBounds(full, visible);
  *full =
      Box(full->xMin() + dx_, full->yMin() + dy_,
          full->xMin() + dx_ + width_ - 1, full->xMin() + dx_ + height_ - 1);
  *visible = Box::intersect(*full, *visible);
}

bool ScrollablePanel::onTouch(const TouchEvent& event) {
  // Stop the scroll.
  scroll_start_vx_ = 0;
  scroll_start_vy_ = 0;
  TouchEvent shifted(event.type(), event.duration(), event.startX() - dxStart_,
                     event.startY() - dyStart_, event.x() - dx_,
                     event.y() - dy_);
  if (event.type() == TouchEvent::RELEASED ||
      event.type() == TouchEvent::PRESSED) {
    dxStart_ = dx_;
    dyStart_ = dy_;
  }
  if (Panel::onTouch(shifted)) return true;
  // Handle drag and swipe.
  if (shifted.type() == TouchEvent::DRAGGED) {
    int16_t drag_x_delta = event.dx() - (dx_ - dxStart_);
    int16_t drag_y_delta = event.dy() - (dy_ - dyStart_);
    // Cap the movement by the boundaries of the underlying content.
    if (drag_x_delta < -dx_ + parent_bounds().width() - width_) {
      drag_x_delta = -dx_ + parent_bounds().width() - width_;
    }
    if (drag_x_delta > -dx_) {
      drag_x_delta = -dx_;
    }
    if (drag_y_delta < -dy_ + parent_bounds().height() - height_) {
      drag_y_delta = -dy_ + parent_bounds().height() - height_;
    }
    if (drag_y_delta > -dy_) {
      drag_y_delta = -dy_;
    }
    // To reduce flicker on random noise, ignore very small drags.
    if (drag_x_delta < 3 && drag_x_delta > -3 && drag_y_delta < 3 &&
        drag_y_delta > -3) {
      return true;
    }
    if (drag_x_delta != 0 || drag_y_delta != 0) {
      dx_ += drag_x_delta;
      dy_ += drag_y_delta;
      invalidateInterior();
    }
    return true;
  } else if (shifted.type() == TouchEvent::SWIPED) {
    scroll_start_time_ = millis();
    // Capture the initial velocity of the scroll.
    scroll_start_vx_ = 1000.0 * shifted.dx() / (float)shifted.duration();
    scroll_start_vy_ = 1000.0 * shifted.dy() / (float)shifted.duration();
    // If we're already on the boundary and swiping outside of the bounded
    // region, cap the horizontal and/or vertical component of the scroll.
    if (scroll_start_vx_ < 0 && dx_ == 0) {
      scroll_start_vx_ = 0;
    }
    if (scroll_start_vx_ > 0 && -dx_ + parent_bounds().width() == width_) {
      scroll_start_vx_ = 0;
    }
    if (scroll_start_vy_ < 0 && dy_ == 0) {
      scroll_start_vy_ = 0;
    }
    if (scroll_start_vy_ > 0 && -dy_ + parent_bounds().height() == height_) {
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