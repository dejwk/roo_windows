#include "widget.h"

#include "roo_display/filter/foreground.h"
#include "roo_windows/main_window.h"
#include "roo_windows/panel.h"

namespace roo_windows {

Widget::Widget(Panel* parent, const Box& parent_bounds)
    : dirty_(true),
      needs_repaint_(true),
      parent_(parent),
      parent_bounds_(parent_bounds),
      state_(kWidgetEnabled),
      on_clicked_([]{}) {
  if (parent != nullptr) {
    parent->addChild(this);
  }
}

MainWindow* Widget::getMainWindow() { return parent_->getMainWindow(); }

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

void Widget::markDirty() {
  dirty_ = true;
  Panel* c = parent_;
  while (c != nullptr) {
    c->markDirty();
    c = c->parent_;
  }
}

void Widget::invalidate() {
  markDirty();
  needs_repaint_ = true;
}

void Widget::setVisible(bool visible) {
  if (visible == isVisible()) return;
  state_ ^= kWidgetHidden;
  if (visible) {
    if (parent_ != nullptr) {
      parent_->markDirty();
    }
    markDirty();
    needs_repaint_ = true;
  } else {
    if (parent_ != nullptr) {
      parent_->markDirty();
      parent_->needs_repaint_ = true;
    }
  }
}

void Widget::setEnabled(bool enabled) {
  if (isEnabled() == enabled) return;
  state_ ^= kWidgetEnabled;
  if (!isVisible()) return;
  markDirty();
  needs_repaint_ = true;
}

void Widget::setSelected(bool selected) {
  if (selected == isSelected()) return;
  state_ ^= kWidgetSelected;
  if (!isVisible()) return;
  markDirty();
  needs_repaint_ = true;
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
  markDirty();
  needs_repaint_ = true;
}

void Widget::setDragged(bool dragged) {
  if (dragged == isDragged()) return;
  state_ ^= kWidgetDragged;
  if (!isVisible()) return;
  markDirty();
  needs_repaint_ = true;
}

Color Widget::getOverlayColor() const {
  Color bgcolor = background();
  const Theme& theme = parent()->theme();
  uint16_t overlay_opacity = 0;
  if (isHover()) overlay_opacity += theme.hoverOpacity(bgcolor);
  if (isFocused()) overlay_opacity += theme.focusOpacity(bgcolor);
  if (isSelected()) overlay_opacity += theme.selectedOpacity(bgcolor);
  if (isActivated() && useOverlayOnActivation()) {
    overlay_opacity += theme.activatedOpacity(bgcolor);
  }
  if (isDragged()) overlay_opacity += theme.draggedOpacity(bgcolor);
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity == 0) return roo_display::color::Transparent;
  Color overlay = theme.color.primary;
  overlay.set_a(overlay_opacity);
  return overlay;
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
    Color overlay = getOverlayColor();
    if (overlay.a() > 0) {
      roo_display::OverlayFilter filter(s.out(), overlay, s.bgcolor());
      news.set_out(&filter);
      defaultPaint(news);
    } else {
      defaultPaint(news);
    }
  } else {
    const Theme& theme = parent()->theme();
    roo_display::TranslucencyFilter disablement_filter(
        s.out(), theme.state.disabled, s.bgcolor());
    news.set_out(&disablement_filter);
    defaultPaint(news);
  }
  needs_repaint_ = false;
  dirty_ = false;
}

bool Widget::onTouch(const TouchEvent& event) {
  if (event.type() == TouchEvent::PRESSED) {
    if (!isPressed() && isClickable() && getMainWindow()->animateClicked(this)) {
      setPressed(true);
      return true;
    }
  } else if (event.type() == TouchEvent::RELEASED) {
    if (isPressed()) {
      setPressed(false);
      if (bounds().contains(event.x(), event.y())) {
        on_clicked_();
        return true;
      }
    }
  }
  return false;
}

}  // namespace roo_windows