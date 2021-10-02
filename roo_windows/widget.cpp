#include "widget.h"

#include "roo_display/filter/foreground.h"
#include "roo_windows/main_window.h"
#include "roo_windows/panel.h"

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

Widget::Widget(Panel* parent, const Box& parent_bounds)
    : dirty_(true),
      needs_repaint_(true),
      parent_(parent),
      parent_bounds_(parent_bounds),
      state_(kWidgetEnabled),
      on_clicked_([] {}) {
  if (parent != nullptr) {
    parent->addChild(this);
  }
}

MainWindow* Widget::getMainWindow() { return parent_->getMainWindow(); }
const MainWindow* Widget::getMainWindow() const {
  return parent_->getMainWindow();
}

void Widget::getAbsoluteBounds(Box* full, Box* visible) const {
  parent()->getAbsoluteBounds(full, visible);
  *full = parent_bounds().translate(full->xMin(), full->yMin());
  *visible = Box::intersect(*visible, *full);
}

Color Widget::background() const { return parent_->background(); }

const Theme& Widget::theme() const { return parent_->theme(); }

void Widget::markDirty() {
  dirty_ = true;
  if (parent_ != nullptr) {
    parent_->markDirty();
  }
}

void Widget::invalidate() {
  invalidateDescending();
  markDirty();
}

void Widget::invalidate(const Box& box) {
  invalidateDescending(box);
  markDirty();
}

void Widget::setVisible(bool visible) {
  if (visible == isVisible()) return;
  state_ ^= kWidgetHidden;
  invalidate();
}

void Widget::setEnabled(bool enabled) {
  if (isEnabled() == enabled) return;
  state_ ^= kWidgetEnabled;
  if (!isVisible()) return;
  invalidate();
}

void Widget::setSelected(bool selected) {
  if (selected == isSelected()) return;
  state_ ^= kWidgetSelected;
  if (!isVisible()) return;
  invalidate();
}

void Widget::setActivated(bool activated) {
  if (activated == isActivated()) return;
  state_ ^= kWidgetActivated;
  if (!isVisible()) return;
  markDirty();
  if (useOverlayOnActivation()) {
    needs_repaint_ = true;
  }
}

void Widget::setPressed(bool pressed) {
  if (pressed == isPressed()) return;
  state_ ^= kWidgetPressed;
  if (!isVisible()) return;
  invalidate();
}

void Widget::setDragged(bool dragged) {
  if (dragged == isDragged()) return;
  state_ ^= kWidgetDragged;
  if (!isVisible()) return;
  invalidate();
}

void Widget::clearClicked() { state_ &= ~kWidgetClicked; }

void Widget::moveTo(const Box& parent_bounds) {
  if (parent_bounds == parent_bounds_) return;
  Box affected = Box::extent(parent_bounds, parent_bounds_);
  parent_bounds_ = parent_bounds;
  if (isVisible()) {
    parent_->invalidate(affected);
  }
}

uint8_t Widget::getOverlayOpacity() const {
  Color bgcolor = background();
  uint16_t overlay_opacity = 0;
  if (isHover()) overlay_opacity += theme().hoverOpacity(bgcolor);
  if (isFocused()) overlay_opacity += theme().focusOpacity(bgcolor);
  if (isSelected()) overlay_opacity += theme().selectedOpacity(bgcolor);
  if (isActivated() && useOverlayOnActivation()) {
    overlay_opacity += theme().activatedOpacity(bgcolor);
  }
  if (isClicking()) {
    if (useOverlayOnPressAnimation()) {
      overlay_opacity += theme().pressedOpacity(bgcolor);
    }
  } else if (isPressed()) {
    overlay_opacity += theme().pressedOpacity(bgcolor);
  }
  if (isDragged()) overlay_opacity += theme().draggedOpacity(bgcolor);
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity == 0) return 0;  // roo_display::color::Transparent;
  return overlay_opacity;
}

void Widget::clear(const Surface& s) {
  if (!needs_repaint_) return;
  s.drawObject(roo_display::Clear());
  dirty_ = false;
  needs_repaint_ = false;
}

namespace {

Color getOverlayColor(const Widget& widget, const Surface& s) {
  uint8_t overlay_opacity = widget.getOverlayOpacity();
  if (overlay_opacity == 0) {
    return roo_display::color::Transparent;
  }
  Color overlay = widget.usesHighlighterColor()
                      ? widget.theme().color.highlighterColor(s.bgcolor())
                      : widget.theme().color.defaultColor(s.bgcolor());
  overlay.set_a(overlay_opacity);
  return overlay;
}

Color getClickAnimationColor(const Widget& widget, const Surface& s) {
  Color color = widget.usesHighlighterColor()
                    ? widget.theme().color.highlighterColor(s.bgcolor())
                    : widget.theme().color.defaultColor(s.bgcolor());
  color.set_a(widget.theme().pressAnimationOpacity(s.bgcolor()));
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
  return (int16_t)(sqrt(max) * progress + 1);
}

}  // namespace

void Widget::paintWidget(const Surface& s) {
  if (!isVisible()) return;
  if (state_ == kWidgetEnabled && !needs_repaint_) {
    // Fast path.
    paint(s);
    needs_repaint_ = false;
    dirty_ = false;
    return;
  }
  Surface news(s);
  if (needs_repaint_) {
    news.set_fill_mode(roo_display::FILL_MODE_RECTANGLE);
  }
  if (isEnabled()) {
    Color overlay = getOverlayColor(*this, s);
    // If click_animation is true, we need to redraw the overlay.
    bool click_animation = ((state_ & kWidgetClicking) != 0);
    float click_progress;
    int16_t click_x, click_y;
    // If click_animation_continues is true, we need to invalidate ourselves
    // after redrawing, so that we receive a subsequent paint request shortly.
    bool click_animation_continues =
        click_animation &&
        getMainWindow()->getClick(this, &click_progress, &click_x, &click_y) &&
        click_progress < 1.0;
    if (!click_animation_continues) {
      state_ &= ~kWidgetClicking;
    }
    if (click_animation_continues) {
      // Need to draw the circular overlay. Combine the regular rect overlay
      // into it.
      Color animation_color = getClickAnimationColor(*this, s);
      PressOverlay press_overlay(
          click_x, click_y,
          animation_radius(bounds(), click_x - s.dx(), click_y - s.dy(),
                           click_progress),
          alphaBlend(overlay, animation_color), overlay);
      roo_display::ForegroundFilter filter(s.out(), &press_overlay);
      news.set_out(&filter);
      paint(news);
      // Do not clear dirtiness.
      invalidate();
      return;
    }
    if (click_animation) {
      // Full rect click overlay - just apply on top of the overlay as
      // calculated so far.
      Color animation_color = getClickAnimationColor(*this, s);
      overlay = alphaBlend(overlay, animation_color);
    }
    if (overlay.a() > 0) {
      roo_display::OverlayFilter filter(s.out(), overlay, s.bgcolor());
      news.set_out(&filter);
      paint(news);
    } else {
      paint(news);
    }
    if (click_animation) {
      // Note that we have !click_animation_continues here. Make sure that the
      // next paint is scheduled promptly, but that it does not encounter
      // click_animation, so that it just draws the widget in its regular state
      // (possibly pressed, but not click-animating anymore).
      invalidate();
      return;
    }
  } else {
    roo_display::TranslucencyFilter disablement_filter(
        s.out(), theme().state.disabled, s.bgcolor());
    news.set_out(&disablement_filter);
    paint(news);
  }
  needs_repaint_ = false;
  dirty_ = false;
}

bool Widget::onTouch(const TouchEvent& event) {
  if (event.type() == TouchEvent::PRESSED) {
    if (!isClickable()) return false;
    if (isPressed()) return false;
    MainWindow* mw = getMainWindow();
    if (mw->isClickAnimating()) return false;
    if (showClickAnimation()) {
      mw->startClickAnimation(this);
      state_ |= kWidgetClicking;
    }
    setPressed(true);
    onPressed();
    return true;
  } else if (event.type() == TouchEvent::RELEASED) {
    if (isPressed()) {
      setPressed(false);
      if (bounds().contains(event.x(), event.y()) ||
          event.duration() < kClickDurationThresholdMs) {
        if (isClicking()) {
          // Defer the delivery of the event until the animation finishes.
          state_ |= kWidgetClicked;
        } else {
          onClicked();
        }
        return true;
      }
    }
  } else if (event.type() == TouchEvent::DRAGGED && isPressed()) {
    // Dragging within the bounds, when pressed, is fair game.
    if (bounds().contains(event.x(), event.y())) return true;
    int16_t dx = event.x() - event.startX();
    int16_t dy = event.y() - event.startY();
    uint32_t delta = dx * dx + dy * dy;
    if (delta < kClickStickinessRadius * kClickStickinessRadius) return true;
    setPressed(false);
    // Leave unhandled, for parent to handle.
  }
  return false;
}

void Widget::updateBounds(const Box& bounds) {
  parent_bounds_ = bounds;
  parent()->invalidate();
}

}  // namespace roo_windows