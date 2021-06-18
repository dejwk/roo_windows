#include "widget.h"

#include "roo_display/filter/foreground.h"
#include "roo_windows/main_window.h"
#include "roo_windows/panel.h"

namespace roo_windows {

// If the end-to-end duration of touch it shorted than this threshold,
// it is always interpreted as a 'click' rather than 'drag'; i.e.
// it is registered in the target component even if the exit coordinates
// fall outside of the component. (It helps to overcome touch panel noise).
static const long int kClickDuratiohThresholdMs = 200;

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

Box Widget::absolute_bounds() const {
  int16_t dx = 0;
  int16_t dy = 0;
  const Widget* w = this;
  do {
    dx += w->parent_bounds().xMin();
    dy += w->parent_bounds().yMin();
    w = w->parent_;
  } while (w != nullptr);
  return bounds().translate(dx, dy);
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

uint8_t Widget::getOverlayOpacity() const {
  Color bgcolor = background();
  uint16_t overlay_opacity = 0;
  if (isHover()) overlay_opacity += theme().hoverOpacity(bgcolor);
  if (isFocused()) overlay_opacity += theme().focusOpacity(bgcolor);
  if (isSelected()) overlay_opacity += theme().selectedOpacity(bgcolor);
  if (isActivated() && useOverlayOnActivation()) {
    overlay_opacity += theme().activatedOpacity(bgcolor);
  }
  if (isPressed() && getMainWindow()->getClickAnimationTarget() == nullptr) {
    overlay_opacity += theme().pressedOpacity(bgcolor);
  }
  if (getMainWindow()->getClickAnimationTarget() == this &&
      useOverlayOnPressAnimation()) {
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

void Widget::paint(const Surface& s) {
  if (!isVisible()) return;
  if (state_ == kWidgetEnabled && !needs_repaint_) {
    // Fast path.
    defaultPaint(s);
    needs_repaint_ = false;
    dirty_ = false;
    return;
  }
  Surface news(s);
  if (needs_repaint_) {
    news.set_fill_mode(roo_display::FILL_MODE_RECTANGLE);
  }
  if (isEnabled()) {
    uint8_t overlay_opacity = getOverlayOpacity();
    if (overlay_opacity > 0) {
      Color overlay = usesHighlighterColor()
                          ? theme().color.highlighterColor(s.bgcolor())
                          : theme().color.defaultColor(s.bgcolor());
      overlay.set_a(overlay_opacity);
      roo_display::OverlayFilter filter(s.out(), overlay, s.bgcolor());
      news.set_out(&filter);
      defaultPaint(news);
    } else {
      defaultPaint(news);
    }
  } else {
    roo_display::TranslucencyFilter disablement_filter(
        s.out(), theme().state.disabled, s.bgcolor());
    news.set_out(&disablement_filter);
    defaultPaint(news);
  }
  needs_repaint_ = false;
  dirty_ = false;
}

bool Widget::onTouch(const TouchEvent& event) {
  if (event.type() == TouchEvent::PRESSED) {
    if (!isPressed() && isClickable() &&
        getMainWindow()->animateClicked(this)) {
      setPressed(true);
      return true;
    }
  } else if (event.type() == TouchEvent::RELEASED) {
    if (isPressed()) {
      setPressed(false);
      if (bounds().contains(event.x(), event.y()) ||
          millis() - event.startTime() < kClickDuratiohThresholdMs) {
        onClicked();
        return true;
      }
    }
  }
  return false;
}

void Widget::updateBounds(const Box& bounds) {
  parent_bounds_ = bounds;
  parent()->invalidate();
}

}  // namespace roo_windows