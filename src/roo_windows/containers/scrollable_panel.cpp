#include "roo_windows/containers/scrollable_panel.h"

#include <Arduino.h>

#include "roo_windows/core/application.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"

namespace roo_windows {

namespace {

static const float kDecceleration = 300.0;
static const float kMaxVel = 5000.0;
static const roo_time::Duration kDelayHideScrollbar = roo_time::Millis(1200);

// Maximum visual overshoot distance in pixels, for both drag and fling.
static const int16_t kMaxOvershootPx = Scaled(40);

// Duration of the spring-back animation (milliseconds).
static const unsigned long kSpringBackDurationMs = 500;

// The area on the side of the panel whose touch is interpreted as an
// interaction with the scroll bar.
static const XDim kScrollBarTouchWidth = Scaled(50);

// Scroll bar height is scaled on the basis of much much content there is to
// scroll, but it will never be smaller than this number of pixels.
static const YDim kScrollBarMinHeightPx = Scaled(20);

template <typename T>
T clamp(T input, T min_val, T max_val) {
  if (input < min_val) return min_val;
  if (input > max_val) return max_val;
  return input;
}

// Returns a damped overshoot in pixels for the given signed
// excess-past-boundary distance. Uses a hyperbolic formula that asymptotes to
// ±kMaxOvershootPx.
int16_t DampedOvershoot(int excess) {
  if (excess == 0) return 0;
  int abs_excess = excess < 0 ? -excess : excess;
  // Hyperbolic formula: asymptotes to ±kMaxOvershootPx.
  int16_t result = (int16_t)roundf((float)kMaxOvershootPx * abs_excess /
                                   (abs_excess + kMaxOvershootPx));
  return excess < 0 ? -result : result;
}

}  // namespace

void VerticalScrollBar::setRange(int16_t begin, int16_t end) {
  if (begin == begin_ && end == end_) return;
  begin_ = begin;
  end_ = end;
  setDirty();
}

void VerticalScrollBar::paint(const Canvas& canvas) const {
  Color s = theme().color.onSurface;
  s.set_a(0xC0);
  s = AlphaBlend(canvas.bgcolor(), s);
  Color semi = theme().color.onSurface;
  semi.set_a(0x40);
  semi = AlphaBlend(canvas.bgcolor(), semi);
  if (begin_ > 0) {
    canvas.fillRect(0, 0, width() - 1, begin_ - 1, semi);
  }
  canvas.fillRect(0, begin_, width() - 1, end_, s);
  if (end_ + 1 < height()) {
    canvas.fillRect(0, end_ + 1, width() - 1, height() - 1, semi);
  }
}

YDim VerticalScrollBar::toOffset(YDim h) const {
  if (h <= height()) {
    // Nothing to scroll.
    return 0;
  }
  // The difference in pixels between the minimum and maximum position of the
  // scroll bar (i.e., the possible range of begin_).
  YDim scroll_range = height() - (end_ - begin_ + 1);
  // The difference in pixels between the minimum and maximum position of the
  // scrolled content (i.e., the possible range of the offset).
  YDim view_range = height() - h;
  return -begin_ * view_range / scroll_range;
}

PreferredSize VerticalScrollBar::getPreferredSize() const {
  return PreferredSize(PreferredSize::ExactWidth(Scaled(6)),
                       PreferredSize::MatchParentHeight());
}

Dimensions VerticalScrollBar::getSuggestedMinimumDimensions() const {
  return Dimensions(Scaled(6), Scaled(6));
}

void SimpleScrollablePanel::scrollTo(XDim x, YDim y) {
  Widget* c = contents();
  if (c == nullptr) return;
  Margins m = c->getMargins();
  Rect inner_pane(m.left(), m.top(), width() - m.right() - 1,
                  height() - m.bottom() - 1);
  auto offset = ResolveAlignmentOffset(bounds(), c->bounds(), alignment_);
  if (c->width() >= inner_pane.width()) {
    x = clamp(x, (XDim)(inner_pane.width() - c->width()), (XDim)0);
  } else {
    x = offset.first;
  }
  if (c->height() >= inner_pane.height()) {
    y = clamp(y, (YDim)(inner_pane.height() - c->height()), (YDim)0);
  } else {
    y = offset.second;
  }
  if (c->height() <= height()) {
    scroll_bar_.setRange(0, height() - 1);
  } else {
    YDim scroll_pix_height =
        std::max(kScrollBarMinHeightPx, height() * height() / c->height());
    YDim scroll_pix_range = height() - scroll_pix_height;
    YDim scroll_pix_begin =
        -(scroll_pix_range) * (y + m.top()) / (c->height() - height());
    scroll_bar_.setRange(scroll_pix_begin,
                         scroll_pix_begin + scroll_pix_height - 1);
  }
  c->moveTo(c->bounds().translate(x + m.left(), y + m.top()));
  onScrollPositionChanged();
}

void SimpleScrollablePanel::scrollToBottom() {
  if (contents() == nullptr) return;
  getMainWindow()->updateLayout();
  Margins m = contents()->getMargins();
  scrollTo(contents()->offsetLeft(),
           height() - m.top() - m.bottom() - contents()->height());
}

void SimpleScrollablePanel::scrollToRaw(XDim x, YDim y) {
  Widget* c = contents();
  if (c == nullptr) return;
  Margins m = c->getMargins();
  Rect inner_pane(m.left(), m.top(), width() - m.right() - 1,
                  height() - m.bottom() - 1);
  auto offset = ResolveAlignmentOffset(bounds(), c->bounds(), alignment_);
  // Compute clamped positions used only for scroll bar display.
  XDim cx = x;
  YDim cy = y;
  if (c->width() >= inner_pane.width()) {
    cx = clamp(cx, (XDim)(inner_pane.width() - c->width()), (XDim)0);
  } else {
    cx = (XDim)offset.first;
  }
  if (c->height() >= inner_pane.height()) {
    cy = clamp(cy, (YDim)(inner_pane.height() - c->height()), (YDim)0);
  } else {
    cy = (YDim)offset.second;
  }
  // Update scroll bar at the clamped (boundary) position.
  if (c->height() <= height()) {
    scroll_bar_.setRange(0, height() - 1);
  } else {
    YDim scroll_pix_height =
        std::max(kScrollBarMinHeightPx, height() * height() / c->height());
    YDim scroll_pix_range = height() - scroll_pix_height;
    YDim scroll_pix_begin =
        -(scroll_pix_range) * (cy + m.top()) / (c->height() - height());
    scroll_bar_.setRange(scroll_pix_begin,
                         scroll_pix_begin + scroll_pix_height - 1);
  }
  // Move content to the unclamped position (may include overshoot).
  c->moveTo(c->bounds().translate(x + m.left(), y + m.top()));
  onScrollPositionChanged();
}

bool SimpleScrollablePanel::isInOvershoot() const {
  const Widget* c = contents();
  if (c == nullptr) return false;
  Margins m = c->getMargins();
  XDim inner_w = (XDim)(width() - m.left() - m.right());
  YDim inner_h = (YDim)(height() - m.top() - m.bottom());
  // Convert to scroll coordinates (origin = content at boundary, no margins).
  XDim sx = c->offsetLeft() - m.left();
  YDim sy = c->offsetTop() - m.top();
  XDim cx = sx;
  YDim cy = sy;
  if (c->width() >= inner_w) {
    cx = clamp(cx, (XDim)(inner_w - c->width()), (XDim)0);
  }
  if (c->height() >= inner_h) {
    cy = clamp(cy, (YDim)(inner_h - c->height()), (YDim)0);
  }
  return cx != sx || cy != sy;
}

void SimpleScrollablePanel::startSpringBack() {
  if (!isInOvershoot()) return;  // Nothing to do.
  Widget* c = contents();
  Margins m = c->getMargins();
  XDim inner_w = (XDim)(width() - m.left() - m.right());
  YDim inner_h = (YDim)(height() - m.top() - m.bottom());
  // Work in scroll coordinates: x=0 means content sits at the left boundary.
  XDim sx = c->offsetLeft() - m.left();
  YDim sy = c->offsetTop() - m.top();
  XDim cx = sx;
  YDim cy = sy;
  if (c->width() >= inner_w) {
    cx = clamp(cx, (XDim)(inner_w - c->width()), (XDim)0);
  }
  if (c->height() >= inner_h) {
    cy = clamp(cy, (YDim)(inner_h - c->height()), (YDim)0);
  }
  animation_ = ScrollAnimation::SPRING_BACK;
  anim_.springback.start_time_ms = millis();
  anim_.springback.start_ox = sx - cx;
  anim_.springback.start_oy = sy - cy;
  anim_.springback.target_x = cx;
  anim_.springback.target_y = cy;
  scheduleScrollAnimationUpdate();
}

PreferredSize SimpleScrollablePanel::getPreferredSize() const {
  // In the dimension that is scrolled over, we will just return 'wrap
  // contents', For example, if the panel scrolls vertically, we report the
  // preferred height as 'wrap contents height'. In the other dimension, we
  // forward the preference of the contents, accounting for our padding and the
  // content's margins (in case when the preference is exact).
  Padding p = getPadding();
  XDim ph = p.left() + p.right();
  YDim pv = p.top() + p.bottom();
  PreferredSize::Width w = PreferredSize::ExactWidth(ph);
  PreferredSize::Height h = PreferredSize::ExactHeight(pv);
  if (contents() != nullptr) {
    PreferredSize c = contents()->getPreferredSize();
    Margins m = contents()->getMargins();
    XDim mh = m.left() + m.right();
    YDim mv = m.top() + m.bottom();
    if (c.width().isExact()) {
      w = PreferredSize::ExactWidth(c.width().value() + ph + mh);
    } else {
      w = c.width();
    }
    if (c.height().isExact()) {
      h = PreferredSize::ExactHeight(c.height().value() + pv + mv);
    } else {
      h = c.height();
    }
  }
  return PreferredSize(direction_ == Direction::kVertical
                           ? w
                           : PreferredSize::WrapContentWidth(),
                       direction_ == Direction::kHorizontal
                           ? h
                           : PreferredSize::WrapContentHeight());
}

Dimensions SimpleScrollablePanel::onMeasure(WidthSpec width,
                                            HeightSpec height) {
  if (contents() == nullptr) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }
  Margins m = contents()->getMargins();
  PreferredSize s = contents()->getPreferredSize();
  WidthSpec child_width =
      (direction_ == Direction::kVertical)
          ? width.getChildWidthSpec(m.left() + m.right(), s.width())
          : WidthSpec::Unspecified(
                std::max(0, width.value() - m.left() - m.right()));
  HeightSpec child_height =
      (direction_ == Direction::kHorizontal)
          ? height.getChildHeightSpec(m.top() + m.bottom(), s.height())
          : HeightSpec::Unspecified(
                std::max((YDim)0, height.value() - m.top() - m.bottom()));
  measured_ = contents()->measure(child_width, child_height);
  scroll_bar_.measure(width, height);
  return Dimensions(
      width.resolveSize(measured_.width() + m.left() + m.right()),
      height.resolveSize(measured_.height() + m.top() + m.bottom()));
}

void SimpleScrollablePanel::onLayout(bool changed, const Rect& rect) {
  Widget* c = contents();
  if (c == nullptr) return;
  Margins m = contents()->getMargins();
  Rect bounds(0, 0, measured_.width() - 1, measured_.height() - 1);
  bounds =
      bounds.translate(c->offsetLeft() + m.left(), c->offsetTop() + m.top());
  c->layout(bounds);
  scroll_bar_.layout(
      Rect(rect.width() - Scaled(6), 0, rect.width() - 1, rect.height() - 1));
  update();
}

void SimpleScrollablePanel::execute(roo_scheduler::EventID id) {
  notification_id_ = -1;

  // --- Spring-back animation (highest priority) ---
  if (animation_ == ScrollAnimation::SPRING_BACK) {
    unsigned long now = millis();
    float t = (float)(now - anim_.springback.start_time_ms) / 1000.0f;
    const float T = kSpringBackDurationMs / 1000.0f;
    if (t >= T) {
      // Animation complete: snap to exact boundary.
      animation_ = ScrollAnimation::IDLE;
      scrollTo(anim_.springback.target_x, anim_.springback.target_y);
      if (scroll_bar_presence_ ==
          VerticalScrollBar::Presence::kShownWhenScrolling) {
        deadline_hide_scrollbar_ =
            roo_time::Uptime::Now() + kDelayHideScrollbar;
        scheduleHideScrollBarUpdate();
      }
    } else {
      // Quadratic ease-out: starts fast, decelerates smoothly to rest.
      float frac = 1.0f - t / T;
      float ease = frac * frac;
      XDim ox = (XDim)roundf((float)anim_.springback.start_ox * ease);
      YDim oy = (YDim)roundf((float)anim_.springback.start_oy * ease);
      scrollToRaw(anim_.springback.target_x + ox,
                  anim_.springback.target_y + oy);
      scheduleScrollAnimationUpdate();
    }
    return;
  }

  if (roo_time::Uptime::Now() >= deadline_hide_scrollbar_) {
    scroll_bar_.setVisibility(Visibility::kInvisible);
  }
  bool scroll_in_progress =
      (contents() != nullptr && animation_ == ScrollAnimation::FLINGING);
  if (!scroll_in_progress) return;

  Widget* c = contents();
  int16_t drag_x_delta = 0;
  int16_t drag_y_delta = 0;

  // Calculate the total offset to be moved.
  unsigned long t_end = millis();
  if ((long)(t_end - anim_.fling.end_time) >= 0) {
    // The time to full stop elapsed; the scroll is certainly over now.
    t_end = anim_.fling.end_time;
    scroll_in_progress = false;
  }
  // Now, a little bit of physics. We calculate total distance traveled
  // by the scrolled content, as S = vt * at^2 (where a < 0), seperately
  // for the horizontal and vertical components.
  // Note that when the scroll gets abruptly terminated in either
  // horizontal or vertical direction because of bumping into a boundary,
  // the corresponding start velocity is set to zero.
  float t = (t_end - anim_.fling.start_time) / 1000.0;
  const bool overshoot_x = (direction_ != Direction::kVertical);
  const bool overshoot_y = (direction_ != Direction::kHorizontal);
  if (anim_.fling.start_vx != 0) {
    // The scrolling continues horizontally.
    int32_t scroll_x_total =
        (int32_t)(anim_.fling.start_vx * t + anim_.fling.decel_x * t * t / 2);
    drag_x_delta = scroll_x_total - (c->offsetLeft() - anim_.fling.dx_start);
    if (overshoot_x) {
      // Allow overshoot up to kMaxOvershootPx past each horizontal boundary.
      XDim new_x = (XDim)(c->offsetLeft() + drag_x_delta);
      if (new_x < (XDim)(width() - c->width()) - (XDim)kMaxOvershootPx) {
        drag_x_delta = (XDim)(width() - c->width()) - (XDim)kMaxOvershootPx -
                       c->offsetLeft();
        anim_.fling.start_vx = 0;
      }
      if (new_x > (XDim)kMaxOvershootPx) {
        drag_x_delta = (XDim)kMaxOvershootPx - c->offsetLeft();
        anim_.fling.start_vx = 0;
      }
    } else {
      // Hard clamp: no horizontal overshoot.
      if (drag_x_delta < -c->offsetLeft() + width() - c->width()) {
        drag_x_delta = -c->offsetLeft() + width() - c->width();
        anim_.fling.start_vx = 0;
      }
      if (drag_x_delta > -c->offsetLeft()) {
        drag_x_delta = -c->offsetLeft();
        anim_.fling.start_vx = 0;
      }
    }
  }
  if (anim_.fling.start_vy != 0) {
    // The scrolling continues vertically.
    int32_t scroll_y_total =
        (int32_t)(anim_.fling.start_vy * t + anim_.fling.decel_y * t * t / 2);
    drag_y_delta = scroll_y_total - (c->offsetTop() - anim_.fling.dy_start);
    if (overshoot_y) {
      // Allow overshoot up to kMaxOvershootPx past each vertical boundary.
      YDim new_y = (YDim)(c->offsetTop() + drag_y_delta);
      if (new_y < (YDim)(height() - c->height()) - (YDim)kMaxOvershootPx) {
        drag_y_delta = (YDim)(height() - c->height()) - (YDim)kMaxOvershootPx -
                       c->offsetTop();
        anim_.fling.start_vy = 0;
      }
      if (new_y > (YDim)kMaxOvershootPx) {
        drag_y_delta = (YDim)kMaxOvershootPx - c->offsetTop();
        anim_.fling.start_vy = 0;
      }
    } else {
      // Hard clamp: no vertical overshoot.
      if (drag_y_delta < -c->offsetTop() + height() - c->height()) {
        drag_y_delta = -c->offsetTop() + height() - c->height();
        anim_.fling.start_vy = 0;
      }
      if (drag_y_delta > -c->offsetTop()) {
        drag_y_delta = -c->offsetTop();
        anim_.fling.start_vy = 0;
      }
    }
  }
  if (drag_x_delta != 0 || drag_y_delta != 0) {
    // Use scrollToRaw to allow the content to visually enter overshoot range.
    scrollToRaw(c->offsetLeft() + drag_x_delta, c->offsetTop() + drag_y_delta);
  }

  // If we're done, e.g. due to bumping against both horizontal and
  // vertical boundaries, then stop refreshing the view.
  if (anim_.fling.start_vx == 0 && anim_.fling.start_vy == 0) {
    scroll_in_progress = false;
  }
  if (scroll_in_progress) {
    scheduleScrollAnimationUpdate();
  } else {
    animation_ = ScrollAnimation::IDLE;
    // If fling left content in overshoot, spring back to the boundary.
    startSpringBack();
    if (animation_ != ScrollAnimation::SPRING_BACK) {
      if (scroll_bar_presence_ ==
          VerticalScrollBar::Presence::kShownWhenScrolling) {
        // Schedule hiding the scroll bar.
        deadline_hide_scrollbar_ =
            roo_time::Uptime::Now() + kDelayHideScrollbar;
        scheduleHideScrollBarUpdate();
      }
    }
  }
}

void SimpleScrollablePanel::cancelPendingUpdate() {
  if (notification_id_ > 0) {
    scheduler_.cancel(notification_id_);
    notification_id_ = -1;
  }
}

void SimpleScrollablePanel::scheduleScrollAnimationUpdate() {
  cancelPendingUpdate();
  notification_id_ = scheduler_.scheduleAfter(roo_time::Millis(10), *this);
}

void SimpleScrollablePanel::scheduleHideScrollBarUpdate() {
  cancelPendingUpdate();
  scheduler_.scheduleAfter(kDelayHideScrollbar, *this);
}

bool SimpleScrollablePanel::onInterceptTouchEvent(const TouchEvent& event) {
  if (animation_ == ScrollAnimation::FLINGING ||
      animation_ == ScrollAnimation::SPRING_BACK) {
    // Scroll or spring-back in progress. Capture all events, including touch
    // down, so that onDown is called on the panel and can stop the animation.
    return true;
  }
  if (scroll_bar_presence_ != VerticalScrollBar::Presence::kAlwaysHidden &&
      contents() != nullptr && contents()->height() > height() &&
      event.x() >= width() - kScrollBarTouchWidth) {
    scroll_bar_gesture_ = true;
    // We set a higher bar for recognizing the interaction as an actual touch of
    // the scroll bar (leading to a scroll event on the scroll bar): the touch
    // must happen in the right place. (We still allow the scroll bar to not be
    // visible at the moment).
    is_scroll_bar_scrolled_ =
        (event.y() >= scroll_bar_.begin() && event.y() <= scroll_bar_.end());
    return is_scroll_bar_scrolled_ || scroll_bar_.isVisible();
  }
  // Don't intercept too aggressively, as it prevents the child to handle its
  // own scrolling (which may be in a different direction.) Instead, rely on the
  // child to relinquish control by returning false from onMove.
  // if (event.type() == TouchEvent::MOVE) {
  //   // If we detect drag motion in the scroll direction, intercept, and turn
  //   // into a scroll event.
  //   const GestureDetector& gd = getApplication()->gesture_detector();
  //   int16_t dx = (direction_ == VERTICAL) ? 0 : gd.xTotalMoveDelta();
  //   int16_t dy = (direction_ == HORIZONTAL) ? 0 : gd.yTotalMoveDelta();
  //   if (dx * dx + dy * dy > kTouchSlopSquare) {
  //     return true;
  //   }
  // }
  return false;
}

bool SimpleScrollablePanel::onDown(XDim x, YDim y) {
  // Stop any fling or spring-back animation.
  animation_ = ScrollAnimation::IDLE;
  cancelPendingUpdate();
  // Snap content to boundary (in case a spring-back was interrupted).
  if (contents() != nullptr) {
    scrollTo(contents()->offsetLeft(), contents()->offsetTop());
    // Initialize raw drag position from the snapped location.
    Margins m = contents()->getMargins();
    animation_ = ScrollAnimation::DRAGGING;
    anim_.drag.raw_x = contents()->offsetLeft() - m.left();
    anim_.drag.raw_y = contents()->offsetTop() - m.top();
  }
  return true;
}

bool SimpleScrollablePanel::onSingleTapUp(XDim x, YDim y) {
  // We generally swallow and ignore tap events, except when tapping on
  // a scrollbar area.
  if (!scroll_bar_gesture_) return true;
  if (scroll_bar_.isVisible()) {
    // Interpret as an immediate jump.
    // Calculate the difference in pixels between the minimum and maximum
    // position of the scroll bar (i.e., the possible range of begin_).
    YDim scroll_range =
        height() - (scroll_bar_.end() - scroll_bar_.begin() + 1);
    // Calculate the difference in pixels between the minimum and maximum
    // position of the scrolled content (i.e., the possible range of the
    // offset).
    YDim view_range = contents()->height() - height();
    if (scroll_range > 0) {
      YDim new_y = -(y - (scroll_bar_.end() - scroll_bar_.begin()) / 2) *
                   view_range / scroll_range;
      // The new y might be out of range, but that's ok - scrollTo will trim it.
      scrollTo(contents()->offsetLeft(), new_y);
    }
  } else {
    scroll_bar_.setVisibility(Visibility::kVisible);
  }
  deadline_hide_scrollbar_ = roo_time::Uptime::Now() + kDelayHideScrollbar;
  scheduleHideScrollBarUpdate();
  return true;
}

bool SimpleScrollablePanel::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  if (contents() == nullptr || contents()->height() <= height()) {
    // Nothing to scroll.
    return false;
  }
  if (scroll_bar_gesture_) {
    if (is_scroll_bar_scrolled_) {
      // Calculate the difference in pixels between the minimum and maximum
      // position of the scroll bar (i.e., the possible range of begin_).
      YDim scroll_range =
          height() - (scroll_bar_.end() - scroll_bar_.begin() + 1);
      // Calculate the difference in pixels between the minimum and maximum
      // position of the scrolled content (i.e., the possible range of the
      // offset).
      YDim view_range = contents()->height() - height();
      YDim y_shift = -dy * view_range / scroll_range;
      // The new y might be out of range, but that's ok - scrollBy will trim it.
      scrollBy(contents()->offsetLeft(), y_shift);
    }
  } else {
    Widget* c = contents();
    Margins m = c->getMargins();
    Rect inner_pane(m.left(), m.top(), width() - m.right() - 1,
                    height() - m.bottom() - 1);
    auto offset = ResolveAlignmentOffset(bounds(), c->bounds(), alignment_);
    // Lazily initialize raw drag position from the current content location.
    // This handles the case where onDown was not called on this panel (e.g.,
    // the scroll bubbled up from a child that couldn't handle it).
    if (animation_ != ScrollAnimation::DRAGGING) {
      animation_ = ScrollAnimation::DRAGGING;
      anim_.drag.raw_x = c->offsetLeft() - m.left();
      anim_.drag.raw_y = c->offsetTop() - m.top();
    }
    // Accumulate raw (unclamped) drag deltas.
    anim_.drag.raw_x += dx;
    anim_.drag.raw_y += dy;
    // Only overshoot in the panel's scrollable direction(s).
    const bool overshoot_x = (direction_ != Direction::kVertical);
    const bool overshoot_y = (direction_ != Direction::kHorizontal);
    // Compute target X.
    XDim target_x;
    if (c->width() >= inner_pane.width()) {
      if (overshoot_x) {
        XDim min_x = inner_pane.width() - c->width();
        XDim clamped_x = clamp(anim_.drag.raw_x, min_x, (XDim)0);
        XDim excess_x = anim_.drag.raw_x - clamped_x;
        target_x = clamped_x + DampedOvershoot(excess_x);
      } else {
        // Scrollable in X but overshoot disabled for this direction: clamp.
        target_x = clamp(anim_.drag.raw_x,
                         (XDim)(inner_pane.width() - c->width()), (XDim)0);
        anim_.drag.raw_x = target_x;
      }
    } else {
      target_x = (XDim)offset.first;
      anim_.drag.raw_x = target_x;
    }
    // Compute target Y.
    YDim target_y;
    if (c->height() >= inner_pane.height()) {
      if (overshoot_y) {
        YDim min_y = inner_pane.height() - c->height();
        YDim clamped_y = clamp(anim_.drag.raw_y, min_y, (YDim)0);
        YDim excess_y = anim_.drag.raw_y - clamped_y;
        target_y = clamped_y + DampedOvershoot(excess_y);
      } else {
        // Scrollable in Y but overshoot disabled for this direction: clamp.
        target_y = clamp(anim_.drag.raw_y,
                         (YDim)(inner_pane.height() - c->height()), (YDim)0);
        anim_.drag.raw_y = target_y;
      }
    } else {
      target_y = (YDim)offset.second;
      anim_.drag.raw_y = target_y;
    }
    scrollToRaw(target_x, target_y);
  }
  if (scroll_bar_presence_ ==
      VerticalScrollBar::Presence::kShownWhenScrolling) {
    scroll_bar_.setVisibility(Visibility::kVisible);
    deadline_hide_scrollbar_ = roo_time::Uptime::Now() + roo_time::Hours(1);
  }
  return true;
}

bool SimpleScrollablePanel::onFling(XDim x, YDim y, XDim vx, YDim vy) {
  if (contents() == nullptr) return true;
  if (scroll_bar_gesture_) {
    // No fling-animate on the scrollbar.
    return true;
  }
  // If the content is currently in overshoot (drag pushed past boundary),
  // ignore the fling and let spring-back handle the return instead.
  if (isInOvershoot()) {
    startSpringBack();
    return true;
  }
  Widget* c = contents();
  animation_ = ScrollAnimation::FLINGING;
  anim_.fling.start_time = millis();
  anim_.fling.start_vx = vx;
  anim_.fling.start_vy = vy;
  anim_.fling.dx_start = c->offsetLeft();
  anim_.fling.dy_start = c->offsetTop();
  // If we're already on the boundary and swiping outside of the bounded
  // region, cap the horizontal and/or vertical component of the scroll.
  if (anim_.fling.start_vx < 0 && c->offsetLeft() == 0) {
    anim_.fling.start_vx = 0;
  }
  if (anim_.fling.start_vx > 0 && -c->offsetLeft() + width() == c->width()) {
    anim_.fling.start_vx = 0;
  }
  if (anim_.fling.start_vy < 0 && c->offsetTop() == 0) {
    anim_.fling.start_vy = 0;
  }
  if (anim_.fling.start_vy > 0 && -c->offsetTop() + height() == c->height()) {
    anim_.fling.start_vy = 0;
  }
  // If the swipe is completely in the outside direction, ignore it.
  if (anim_.fling.start_vx == 0 && anim_.fling.start_vy == 0) {
    animation_ = ScrollAnimation::IDLE;
    return true;
  }
  // Calculate the length of the initial velocity vector.
  float v_abs = sqrtf(anim_.fling.start_vx * anim_.fling.start_vx +
                      anim_.fling.start_vy * anim_.fling.start_vy);
  // Cap that scroll velocity if too large.
  if (v_abs > kMaxVel) {
    float mult = kMaxVel / v_abs;
    anim_.fling.start_vx *= mult;
    anim_.fling.start_vy *= mult;
    v_abs = kMaxVel;
  }
  // Scroll is constantly deccelerating (with configured constant
  // decceleration). Calculate the total scroll time until it stops on
  // its own if uninterrupted.
  unsigned long scroll_duration =
      (unsigned long)(1000.0 * v_abs / kDecceleration);
  anim_.fling.end_time = anim_.fling.start_time + scroll_duration;
  // Capture horizontal and vertical components of the decceleration.
  anim_.fling.decel_x = -kDecceleration * anim_.fling.start_vx / v_abs;
  anim_.fling.decel_y = -kDecceleration * anim_.fling.start_vy / v_abs;
  // Apply a small initial scroll step immediately, so that the render
  // happening in the same tick() already shows the beginning of the fling.
  // Without this, the first animation frame is delayed until after the
  // render completes, causing a visible hesitation.
  // The physics in execute() self-corrects via (offsetTop - dy_start).
  int16_t kick_dx = (int16_t)(anim_.fling.start_vx * 0.02f);
  int16_t kick_dy = (int16_t)(anim_.fling.start_vy * 0.02f);
  if (kick_dx != 0 || kick_dy != 0) {
    scrollBy(kick_dx, kick_dy);
  }
  scheduleScrollAnimationUpdate();
  return true;
}

bool SimpleScrollablePanel::onTouchUp(XDim vx, YDim vy) {
  bool result = Widget::onTouchUp(vx, vy);
  scroll_bar_gesture_ = false;
  is_scroll_bar_scrolled_ = false;
  if (animation_ != ScrollAnimation::FLINGING) {
    // No fling was started; spring back if the drag left us in overshoot.
    animation_ = ScrollAnimation::IDLE;
    startSpringBack();
    if (animation_ != ScrollAnimation::SPRING_BACK) {
      if (scroll_bar_presence_ ==
          VerticalScrollBar::Presence::kShownWhenScrolling) {
        deadline_hide_scrollbar_ =
            roo_time::Uptime::Now() + kDelayHideScrollbar;
        scheduleHideScrollBarUpdate();
      }
    }
  }
  return result;
}

}  // namespace roo_windows
