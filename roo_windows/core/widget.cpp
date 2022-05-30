#include "roo_windows/core/widget.h"

#include <cmath>

#include "roo_display/filter/foreground.h"
#include "roo_glog/logging.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/core/rtti.h"

namespace roo_windows {

// If the end-to-end duration of touch it shorted than this threshold,
// it is always interpreted as a 'click' rather than 'drag'; i.e.
// it is registered in the target component even if the exit coordinates
// fall outside of the component. (It helps to overcome touch panel noise).
static const long int kClickDurationThresholdMs = 200;

// When the user presses on a clickable item and drags the finger within
// this radius, the drag events are ignored and considered 'random noise'.
// The item remains clicked, and when the press is released, the gesture
// is interpreted as 'click'. If the user drags beyond this thredhold,
// however, then the item gets 'unclicked' and the gesture gets interpreted
// as drag.
static const long int kClickStickinessRadius = 40;

Widget::Widget(const Environment& env)
    : parent_(nullptr),
      parent_bounds_(0, 0, -1, -1),
      state_(kWidgetEnabled),
      redraw_status_(kDirty | kInvalidated),
      on_clicked_([] {}) {}

Widget::Widget(const Widget& w)
    : parent_(nullptr),
      parent_bounds_(0, 0, -1, -1),
      state_(kWidgetEnabled),
      redraw_status_(kDirty | kInvalidated),
      on_clicked_([] {}) {}

MainWindow* Widget::getMainWindow() { return parent_->getMainWindow(); }

const MainWindow* Widget::getMainWindow() const {
  return parent_->getMainWindow();
}

Task* Widget::getTask() { return parent_->getTask(); }

ClickAnimation* Widget::getClickAnimation() {
  MainWindow* w = getMainWindow();
  return (w == nullptr) ? nullptr : &w->click_animation();
}

void Widget::getAbsoluteBounds(Box& full, Box& visible) const {
  if (parent() == nullptr) {
    full = visible = parent_bounds();
  } else {
    parent()->getAbsoluteBounds(full, visible);
    full = parent_bounds().translate(full.xMin(), full.yMin());
    visible = Box::intersect(visible.translate(full.xMin(), full.yMin()), full);
  }
}

void Widget::getAbsoluteOffset(int16_t& dx, int16_t& dy) const {
  if (parent() == nullptr) {
    dx = 0;
    dy = 0;
  } else {
    parent()->getAbsoluteOffset(dx, dy);
  }
  dx += xOffset();
  dy += yOffset();
}

Color Widget::effectiveBackground() const {
  roo_display::Color bgcolor = background();
  return bgcolor.opaque() || parent() == nullptr
             ? bgcolor
             : roo_display::alphaBlend(parent()->effectiveBackground(),
                                       bgcolor);
}

// Returns the theme used by this widget. Defaults to the parent's theme
// (and, ultimately, to DefaultTheme(), if not otherwise specified).
const Theme& Widget::theme() const { return parent_->theme(); }

void Widget::markDirty(const Box& bounds) {
  redraw_status_ |= kDirty;
  if (parent_ != nullptr) {
    parent_->propagateDirty(this, bounds.translate(xOffset(), yOffset()));
  }
}

void Widget::invalidateInterior() {
  invalidateDescending();
  markDirty();
}

void Widget::invalidateInterior(const Box& box) {
  invalidateDescending(box);
  markDirty(box);
}

void Widget::requestLayout() {
  if (isLayoutRequested()) return;
  onRequestLayout();
}

Dimensions Widget::measure(MeasureSpec width, MeasureSpec height) {
  Dimensions result = onMeasure(width, height);
  redraw_status_ |= kLayoutRequired;
  return result;
}

void Widget::layout(const roo_display::Box& box) {
  bool changed = (box != parent_bounds());
  if (changed || isLayoutRequired()) {
    moveTo(box);
    onLayout(changed, box);
  }
  redraw_status_ &= ~(kLayoutRequired | kLayoutRequested);
}

void Widget::onRequestLayout() {
  markLayoutRequested();
  if (parent() != nullptr) parent()->requestLayout();
}

namespace {

// Utility to return a default size. Uses the supplied size if the MeasureSpec
// imposed no constraints. Will get larger if allowed by the MeasureSpec.
static int16_t getDefaultSize(int16_t size, MeasureSpec spec) {
  switch (spec.kind()) {
    case MeasureSpec::UNSPECIFIED:
      return size;
    default:
      return spec.value();
  }
}

}  // namespace

Dimensions Widget::onMeasure(MeasureSpec width, MeasureSpec height) {
  Dimensions suggestedMin = getSuggestedMinimumDimensions();
  return Dimensions(getDefaultSize(suggestedMin.width(), width),
                    getDefaultSize(suggestedMin.height(), height));
}

void Widget::setParentClipMode(ParentClipMode mode) {
  if (mode == getParentClipMode()) return;
  bool visible = isVisible();
  if (visible) {
    setVisible(false);
  }
  state_ ^= kWidgetClippedInParent;
  if (visible) {
    setVisible(true);
  }
}

void Widget::setVisible(bool visible) {
  if (visible == isVisible()) return;
  state_ ^= kWidgetHidden;
  invalidateInterior();
  if (isVisible()) {
    requestLayout();
    if (parent() != nullptr) parent()->childShown(this);
  } else {
    setPressed(false);
    state_ &= ~kWidgetClicking;
    if (parent() != nullptr) parent()->childHidden(this);
  }
}

void Widget::setEnabled(bool enabled) {
  if (isEnabled() == enabled) return;
  state_ ^= kWidgetEnabled;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setSelected(bool selected) {
  if (selected == isSelected()) return;
  state_ ^= kWidgetSelected;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setActivated(bool activated) {
  if (activated == isActivated()) return;
  state_ ^= kWidgetActivated;
  if (!isVisible()) return;
  markDirty();
  if (useOverlayOnActivation()) {
    invalidateInterior();
  }
}

void Widget::setPressed(bool pressed) {
  if (pressed == isPressed()) return;
  state_ ^= kWidgetPressed;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setDragged(bool dragged) {
  if (dragged == isDragged()) return;
  state_ ^= kWidgetDragged;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setClicking() { state_ |= kWidgetClicking; }

void Widget::setParent(Panel* parent, bool owned) {
  CHECK(parent_ == nullptr || parent == nullptr)
      << "widget " << *this << " being added, but"
      << " it already has a parent";
  parent_ = parent;
  if (owned) {
    state_ |= kWidgetOwnedByParent;
  } else {
    state_ &= ~kWidgetOwnedByParent;
  }
}

void Widget::setParentBounds(const Box& parent_bounds) {
  if (parent_bounds == parent_bounds_) return;
  if (!isVisible() || parent() == nullptr) {
    parent_bounds_ = parent_bounds;
    return;
  }
  invalidateInterior();
  if (parent() != nullptr) parent()->childHidden(this);
  parent_bounds_ = parent_bounds;
  invalidateInterior();
  if (parent() != nullptr) parent()->childShown(this);
}

void Widget::moveTo(const Box& parent_bounds) {
  setParentBounds(parent_bounds);
}

uint8_t Widget::getOverlayOpacity() const {
  Color bgcolor = background();
  uint16_t overlay_opacity = 0;
  const Theme& myTheme = theme();
  if (isHover()) overlay_opacity += myTheme.hoverOpacity(bgcolor);
  if (isFocused()) overlay_opacity += myTheme.focusOpacity(bgcolor);
  if (isSelected()) overlay_opacity += myTheme.selectedOpacity(bgcolor);
  if (isActivated() && useOverlayOnActivation()) {
    overlay_opacity += myTheme.activatedOpacity(bgcolor);
  }
  if (isClicking()) {
    if (useOverlayOnPressAnimation()) {
      overlay_opacity += myTheme.pressedOpacity(bgcolor);
    }
  } else if (isPressed() && useOverlayOnPress()) {
    overlay_opacity += myTheme.pressedOpacity(bgcolor);
  }
  if (isDragged()) overlay_opacity += myTheme.draggedOpacity(bgcolor);
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity == 0) return 0;  // roo_display::color::Transparent;
  return overlay_opacity;
}

namespace {

Color getOverlayColor(const Widget& widget, const Surface& s) {
  uint8_t overlay_opacity = widget.getOverlayOpacity();
  if (overlay_opacity == 0) {
    return roo_display::color::Transparent;
  }
  const Theme& myTheme = widget.theme();
  Color overlay = widget.usesHighlighterColor()
                      ? myTheme.color.highlighterColor(s.bgcolor())
                      : myTheme.color.defaultColor(s.bgcolor());
  overlay.set_a(overlay_opacity);
  return overlay;
}

Color getClickAnimationColor(const Widget& widget, const Surface& s) {
  const Theme& myTheme = widget.theme();
  Color color = widget.usesHighlighterColor()
                    ? myTheme.color.highlighterColor(s.bgcolor())
                    : myTheme.color.defaultColor(s.bgcolor());
  color.set_a(myTheme.pressAnimationOpacity(s.bgcolor()));
  return color;
}

inline int32_t dsquare(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
  return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

inline int16_t animation_radius(const Box& bounds, int16_t x, int16_t y,
                                float progress) {
  int32_t ul = dsquare(x, y, bounds.xMin(), bounds.yMin());
  int32_t ur = dsquare(x, y, bounds.xMax(), bounds.yMin());
  int32_t dl = dsquare(x, y, bounds.xMin(), bounds.yMax());
  int32_t dr = dsquare(x, y, bounds.xMax(), bounds.yMax());
  int32_t max = 0;
  if (ul > max) max = ul;
  if (ur > max) max = ur;
  if (dl > max) max = dl;
  if (dr > max) max = dr;
  return (int16_t)(std::sqrt(max) * progress + 1);
}

}  // namespace

void Widget::paintWidget(const Surface& s, Clipper& clipper) {
  if (!isVisible()) {
    markCleanDescending();
    return;
  }
  Surface news(s);
  news.set_dx(s.dx() + xOffset());
  news.set_dy(s.dy() + yOffset());
  if (news.clipToExtents(maxBounds()) == Box::CLIP_RESULT_EMPTY) {
    markCleanDescending();
    return;
  }
  if (!isDirty()) {
    // Fast path to only include exclusion rect.
    paintWidgetContents(news, clipper);
  }
  news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), background()));
  if (isEnabled()) {
    Color overlay = getOverlayColor(*this, s);
    // If click_animation is true, we need to redraw the overlay.
    bool click_animation = ((state_ & kWidgetClicking) != 0);
    if (click_animation) {
      float click_progress;
      int16_t click_x, click_y;
      // If click_animation_continues is true, we need to invalidate ourselves
      // after redrawing, so that we receive a subsequent paint request shortly.
      bool click_animation_continues =
          click_animation &&
          getClickAnimation()->getProgress(this, &click_progress, &click_x,
                                           &click_y) &&
          click_progress < 1.0;
      if (click_animation_continues) {
        // Need to draw the circular overlay. Combine the regular rect overlay
        // into it.
        Color animation_color = getClickAnimationColor(*this, s);
        PressOverlay press_overlay(
            click_x + news.dx(), click_y + news.dy(),
            animation_radius(bounds(), click_x, click_y, click_progress),
            alphaBlend(overlay, animation_color), overlay);
        roo_display::ForegroundFilter filter(news.out(), &press_overlay);
        news.set_out(&filter);
        paintWidgetContents(news, clipper);
        // Do not clear dirtiness.
        invalidateInterior();
        return;
      }
      // Full rect click overlay - just apply on top of the overlay as
      // calculated so far.
      state_ &= ~kWidgetClicking;
      Color animation_color = getClickAnimationColor(*this, s);
      overlay = alphaBlend(overlay, animation_color);
    }
    if (overlay.a() > 0) {
      roo_display::OverlayFilter filter(news.out(), overlay, s.bgcolor());
      news.set_out(&filter);
      paintWidgetContents(news, clipper);
    } else {
      paintWidgetContents(news, clipper);
    }
    if (click_animation) {
      // Note that we have !click_animation_continues here. Make sure that the
      // next paint is scheduled promptly, but that it does not encounter
      // click_animation, so that it just draws the widget in its regular state
      // (possibly pressed, but not click-animating anymore).
      invalidateInterior();
      return;
    }
  } else {
    roo_display::TranslucencyFilter disablement_filter(
        s.out(), theme().state.disabled, s.bgcolor());
    news.set_out(&disablement_filter);
    paintWidgetContents(news, clipper);
  }
}

void Widget::paintWidgetContents(const Surface& s, Clipper& clipper) {
  Box absolute_bounds =
      Box::intersect(bounds().translate(s.dx(), s.dy()), s.clip_box());
  if (isDirty()) {
    clipper.setBounds(absolute_bounds);
    bool clean = paint(s);
    markClean();
    if (!clean) markDirty();
  }
  clipper.addExclusion(absolute_bounds);
}

Widget* Widget::dispatchTouchDownEvent(int16_t x, int16_t y,
                                       GestureDetector& gesture_detector) {
  return (onTouchDown(x, y, gesture_detector)) ? this : nullptr;
}

bool Widget::onTouchDown(int16_t x, int16_t y,
                         GestureDetector& gesture_detector) {
  return gesture_detector.onTouchDown(*this, x, y);
}

bool Widget::onTouchMove(int16_t x, int16_t y,
                         GestureDetector& gesture_detector) {
  return gesture_detector.onTouchMove(*this, x, y);
}

bool Widget::onTouchUp(int16_t x, int16_t y,
                       GestureDetector& gesture_detector) {
  return gesture_detector.onTouchUp(*this, x, y);
}

void Widget::onClicked() { on_clicked_(); }

bool Widget::onDown(int16_t x, int16_t y) { return isClickable(); }

void Widget::onShowPress(int16_t x, int16_t y) {
  if (!isClickable()) return;
  if (isPressed()) return;
  ClickAnimation* anim = getClickAnimation();
  if (anim->isClickAnimating() || anim->isClickConfirmed()) return;
  if (showClickAnimation()) {
    anim->start(this, x, y);
    setClicking();
  }
  setPressed(true);
}

bool Widget::onSingleTapUp(int16_t x, int16_t y) {
  if (!isClickable()) return false;
  if (isPressed()) {
    setPressed(false);
  } else {
    // Quick release (onShowPress not yet triggered).
    if (showClickAnimation()) {
      ClickAnimation* anim = getClickAnimation();
      setClicking();
      markDirty();
      anim->start(this, x, y);
    }
  }
  getClickAnimation()->confirmClick(this);
  return true;
}

bool Widget::onScroll(int16_t dx, int16_t dy) {
  setPressed(false);
  return false;
}

bool Widget::onFling(int16_t vx, int16_t vy) {
  setPressed(false);
  return false;
}

std::ostream& operator<<(std::ostream& os, const Widget& widget) {
  os << GetTypeName(widget) << "{ " << &widget << "}";
  return os;
}

}  // namespace roo_windows
