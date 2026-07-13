#include "roo_windows/containers/scrollable_panel.h"

#include "roo_windows/core/application.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"

namespace roo_windows {

namespace {

static const roo_time::Duration kDelayHideScrollbar = roo_time::Millis(1200);

// The area on the side of the panel whose touch is interpreted as an
// interaction with the scroll bar.
static const XDim kScrollBarTouchWidth = Scaled(50);

// Scroll bar height is scaled on the basis of much much content there is to
// scroll, but it will never be smaller than this number of pixels.
static const YDim kScrollBarMinHeightPx = Scaled(20);

}  // namespace

void VerticalScrollBar::setRange(int16_t begin, int16_t end) {
  if (begin == begin_ && end == end_) return;
  begin_ = begin;
  end_ = end;
  setDirty();
}

void VerticalScrollBar::paint(PaintContext& ctx) const {
  Color s = theme().framework.color.resolve(FrameworkColorRole::kContent);
  s.set_a(0xC0);
  s = AlphaBlend(ctx.bgcolor(), s);
  Color semi = theme().framework.color.resolve(FrameworkColorRole::kContent);
  semi.set_a(0x40);
  semi = AlphaBlend(ctx.bgcolor(), semi);
  if (begin_ > 0) {
    ctx.fillRect(0, 0, width() - 1, begin_ - 1, semi);
  }
  ctx.fillRect(0, begin_, width() - 1, end_, s);
  if (end_ + 1 < height()) {
    ctx.fillRect(0, end_ + 1, width() - 1, height() - 1, semi);
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
  ScrollPosition current = currentScrollPosition();
  applyScrollResult(
      motion_.scrollTo(motionGeometry(), current.x, current.y, x, y));
}

void SimpleScrollablePanel::scrollToBottom() {
  if (contents() == nullptr) return;
  getMainWindow()->updateLayout();
  Margins m = contents()->getMargins();
  scrollTo(contents()->offsetLeft() - m.left(),
           height() - m.top() - m.bottom() - contents()->height());
}

bool SimpleScrollablePanel::containsDescendant(const Widget& descendant) const {
  for (const Widget* current = &descendant; current != nullptr;
       current = current->parent()) {
    if (current == this) return true;
  }
  return false;
}

bool SimpleScrollablePanel::revealFocusedDescendant(Widget& descendant) {
  if (contents() == nullptr || !containsDescendant(descendant)) return false;

  // Project the target's parent-space bounds into this panel's coordinates.
  Rect target = descendant.parent_bounds();
  for (Widget* current = descendant.parent(); current != nullptr &&
                                               current != this;
       current = current->parent()) {
    target = target.translate(current->offsetLeft(), current->offsetTop());
  }
  if (descendant.parent() == nullptr) return false;

  Margins margins = contents()->getMargins();
  Rect viewport(margins.left(), margins.top(), width() - margins.right() - 1,
                height() - margins.bottom() - 1);
  if (viewport.empty()) return false;

  ScrollPosition position = currentScrollPosition();
  XDim desired_x = position.x;
  YDim desired_y = position.y;
  if (target.width() > viewport.width()) {
    desired_x += viewport.xMin() - target.xMin();
  } else if (target.xMin() < viewport.xMin()) {
    desired_x += viewport.xMin() - target.xMin();
  } else if (target.xMax() > viewport.xMax()) {
    desired_x -= target.xMax() - viewport.xMax();
  }
  if (target.height() > viewport.height()) {
    desired_y += viewport.yMin() - target.yMin();
  } else if (target.yMin() < viewport.yMin()) {
    desired_y += viewport.yMin() - target.yMin();
  } else if (target.yMax() > viewport.yMax()) {
    desired_y -= target.yMax() - viewport.yMax();
  }
  scrollTo(desired_x, desired_y);
  return true;
}

bool SimpleScrollablePanel::onKeyEvent(const KeyEvent& event) {
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat) {
    return false;
  }
  if (contents() == nullptr) return false;
  const XDim page_x = std::max<XDim>(1, width() - Scaled(16));
  const YDim page_y = std::max<YDim>(1, height() - Scaled(16));
  switch (event.code) {
    case KeyCode::kLeft:
      scrollBy(page_x / 8, 0);
      return true;
    case KeyCode::kRight:
      scrollBy(-page_x / 8, 0);
      return true;
    case KeyCode::kUp:
      scrollBy(0, page_y / 8);
      return true;
    case KeyCode::kDown:
      scrollBy(0, -page_y / 8);
      return true;
    case KeyCode::kPageUp:
      scrollBy(0, page_y);
      return true;
    case KeyCode::kPageDown:
      scrollBy(0, -page_y);
      return true;
    case KeyCode::kHome:
      scrollToTop();
      return true;
    case KeyCode::kEnd:
      scrollToBottom();
      return true;
    default:
      return false;
  }
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
  (void)id;
  notification_id_ = -1;

  if (roo_time::Uptime::Now() >= deadline_hide_scrollbar_) {
    scroll_bar_.setVisibility(Visibility::kInvisible);
  }

  if (!motion_.isAnimating()) return;
  ScrollPosition current = currentScrollPosition();
  scroll_motion::Result result =
      motion_.tick(motionGeometry(), current.x, current.y, millis());
  applyScrollResult(result);
  if (result.needs_tick) {
    scheduleScrollAnimationUpdate();
  } else {
    if (scroll_bar_presence_ ==
        VerticalScrollBar::Presence::kShownWhenScrolling) {
      deadline_hide_scrollbar_ = roo_time::Uptime::Now() + kDelayHideScrollbar;
      scheduleHideScrollBarUpdate();
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
  notification_id_ = scheduler_.scheduleAfter(kDelayHideScrollbar, *this);
}

scroll_motion::Axis SimpleScrollablePanel::AxisForDirection(
    Direction direction) {
  switch (direction) {
    case Direction::kHorizontal:
      return scroll_motion::Axis::kHorizontal;
    case Direction::kBoth:
      return scroll_motion::Axis::kBoth;
    case Direction::kVertical:
    default:
      return scroll_motion::Axis::kVertical;
  }
}

scroll_motion::Geometry SimpleScrollablePanel::motionGeometry() const {
  const Widget* c = contents();
  if (c == nullptr) return {0, 0, 0, 0, AxisForDirection(direction_)};
  Margins m = c->getMargins();
  XDim viewport_w = std::max<XDim>(0, width() - m.left() - m.right());
  YDim viewport_h = std::max<YDim>(0, height() - m.top() - m.bottom());
  XDim min_x = c->width() >= viewport_w ? viewport_w - c->width() : 0;
  YDim min_y = c->height() >= viewport_h ? viewport_h - c->height() : 0;
  return {min_x, 0, min_y, 0, AxisForDirection(direction_)};
}

SimpleScrollablePanel::ScrollPosition
SimpleScrollablePanel::currentScrollPosition() const {
  const Widget* c = contents();
  if (c == nullptr) return {0, 0};
  Margins m = c->getMargins();
  return {static_cast<XDim>(c->offsetLeft() - m.left()),
          static_cast<YDim>(c->offsetTop() - m.top())};
}

void SimpleScrollablePanel::applyScrollResult(
    const scroll_motion::Result& result) {
  Widget* c = contents();
  if (c == nullptr) return;
  Margins m = c->getMargins();
  Rect inner_pane(m.left(), m.top(), width() - m.right() - 1,
                  height() - m.bottom() - 1);
  auto aligned = ResolveAlignmentOffset(bounds(), c->bounds(), alignment_);
  XDim visual_x = result.x;
  YDim visual_y = result.y;
  XDim clamped_y = result.y;
  if (c->width() < inner_pane.width()) {
    visual_x = static_cast<XDim>(aligned.first);
  }
  if (c->height() < inner_pane.height()) {
    visual_y = static_cast<YDim>(aligned.second);
    clamped_y = 0;
  } else {
    clamped_y = std::max<YDim>(inner_pane.height() - c->height(),
                               std::min<YDim>(result.y, 0));
  }

  if (c->height() <= height()) {
    scroll_bar_.setRange(0, height() - 1);
  } else {
    YDim scroll_pix_height =
        std::max(kScrollBarMinHeightPx, height() * height() / c->height());
    YDim scroll_pix_range = height() - scroll_pix_height;
    YDim scroll_pix_begin =
        -(scroll_pix_range) * (clamped_y + m.top()) / (c->height() - height());
    scroll_bar_.setRange(scroll_pix_begin,
                         scroll_pix_begin + scroll_pix_height - 1);
  }
  c->moveTo(c->bounds().translate(visual_x + m.left(), visual_y + m.top()));
  onScrollPositionChanged();
}

bool SimpleScrollablePanel::onInterceptTouchEvent(const TouchEvent& event) {
  if (motion_.isAnimating()) {
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

void SimpleScrollablePanel::onDragStart(XDim x, YDim y) {
  (void)x;
  (void)y;
  cancelPendingUpdate();
  if (contents() != nullptr) {
    ScrollPosition current = currentScrollPosition();
    applyScrollResult(motion_.onDown(motionGeometry(), current.x, current.y));
  }
}

void SimpleScrollablePanel::onSingleTapUp(XDim x, YDim y) {
  // We generally swallow and ignore tap events, except when tapping on
  // a scrollbar area.
  if (!scroll_bar_gesture_) return;
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
      ScrollPosition pos = currentScrollPosition();
      scrollTo(pos.x, new_y);
    }
  } else {
    scroll_bar_.setVisibility(Visibility::kVisible);
  }
  deadline_hide_scrollbar_ = roo_time::Uptime::Now() + kDelayHideScrollbar;
  scheduleHideScrollBarUpdate();
}

void SimpleScrollablePanel::onDrag(XDim x, YDim y, XDim dx, YDim dy) {
  (void)x;
  (void)y;
  if (contents() == nullptr) {
    // Nothing to scroll.
    return;
  }
  scroll_motion::Geometry geometry = motionGeometry();
  if (!geometry.canScroll()) return;
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
      scrollBy(0, y_shift);
    }
  } else {
    ScrollPosition current = currentScrollPosition();
    applyScrollResult(motion_.onDrag(geometry, current.x, current.y, dx, dy));
  }
  if (scroll_bar_presence_ ==
      VerticalScrollBar::Presence::kShownWhenScrolling) {
    scroll_bar_.setVisibility(Visibility::kVisible);
    deadline_hide_scrollbar_ = roo_time::Uptime::Now() + roo_time::Hours(1);
  }
}

void SimpleScrollablePanel::onFling(XDim x, YDim y, XDim vx, YDim vy) {
  (void)x;
  (void)y;
  if (contents() == nullptr) return;
  if (scroll_bar_gesture_) {
    // No fling-animate on the scrollbar.
    return;
  }
  ScrollPosition current = currentScrollPosition();
  scroll_motion::Result result =
      motion_.onFling(motionGeometry(), current.x, current.y, vx, vy, millis());
  applyScrollResult(result);
  if (result.needs_tick) scheduleScrollAnimationUpdate();
}

void SimpleScrollablePanel::onDragFinished(XDim vx, YDim vy) {
  (void)vx;
  (void)vy;
  scroll_bar_gesture_ = false;
  is_scroll_bar_scrolled_ = false;
  if (contents() != nullptr) {
    ScrollPosition current = currentScrollPosition();
    scroll_motion::Result scroll_result =
        motion_.onTouchUp(motionGeometry(), current.x, current.y, millis());
    applyScrollResult(scroll_result);
    if (scroll_result.needs_tick) {
      scheduleScrollAnimationUpdate();
    } else if (scroll_bar_presence_ ==
               VerticalScrollBar::Presence::kShownWhenScrolling) {
      deadline_hide_scrollbar_ = roo_time::Uptime::Now() + kDelayHideScrollbar;
      scheduleHideScrollBarUpdate();
    }
  }
}

}  // namespace roo_windows
