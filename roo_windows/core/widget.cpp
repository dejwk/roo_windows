#include "widget.h"

#include "roo_display/filter/foreground.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/panel.h"

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
    : parent_(parent),
      parent_bounds_(parent_bounds),
      state_(kWidgetEnabled),
      redraw_status_(kDirty | kInvalidated),
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

void Widget::setVisible(bool visible) {
  if (visible == isVisible()) return;
  state_ ^= kWidgetHidden;
  invalidateInterior();
  if (!isVisible()) {
    parent()->childHidden(this);
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

void Widget::clearClicked() { state_ &= ~kWidgetClicked; }

void Widget::moveTo(const Box& parent_bounds) {
  if (parent_bounds == parent_bounds_) return;
  if (!isVisible()) {
    parent_bounds_ = parent_bounds;
    return;
  }
  setVisible(false);
  parent_bounds_ = parent_bounds;
  setVisible(true);
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
  } else if (isPressed()) {
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
  return (int16_t)(sqrt(max) * progress + 1);
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
          getMainWindow()->getClick(this, &click_progress, &click_x,
                                    &click_y) &&
          click_progress < 1.0;
      if (click_animation_continues) {
        // Need to draw the circular overlay. Combine the regular rect overlay
        // into it.
        Color animation_color = getClickAnimationColor(*this, s);
        PressOverlay press_overlay(
            click_x, click_y,
            animation_radius(bounds(), click_x - news.dx(), click_y - news.dy(),
                             click_progress),
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
    paint(s);
    markClean();
  }
  clipper.addExclusion(absolute_bounds);
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
          getMainWindow()->clickWidget(this);
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
  parent()->invalidateInterior();
}

}  // namespace roo_windows
