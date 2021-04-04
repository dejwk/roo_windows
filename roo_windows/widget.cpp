#include "widget.h"

#include "roo_windows/panel.h"
#include "roo_display/filter/foreground.h"

namespace roo_windows {

Widget::Widget(Panel* parent, const Box& parent_bounds)
    : dirty_(true),
      needs_repaint_(true),
      parent_(parent),
      parent_bounds_(parent_bounds),
      state_(kWidgetEnabled) {
  if (parent != nullptr) {
    parent->addChild(this);
  }
}

void Widget::markDirty() {
  dirty_ = true;
  Panel* c = parent_;
  while (c != nullptr) {
    c->markDirty();
    c = c->parent_;
  }
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
  const Theme& theme = parent()->theme();
  if (!isEnabled()) {
    roo_display::TranslucencyFilter disablement_filter(
      s.out(), theme.state.disabled, s.bgcolor());
    news.set_out(&disablement_filter);
    defaultPaint(news);
    needs_repaint_ = false;
    dirty_ = false;
    return;
  }

  uint16_t overlay_opacity = 0;
  if (isHover()) overlay_opacity += theme.hoverOpacity(s.bgcolor());
  if (isFocused()) overlay_opacity += theme.focusOpacity(s.bgcolor());
  if (isSelected()) overlay_opacity += theme.selectedOpacity(s.bgcolor());
  if (isActivated()) overlay_opacity += theme.activatedOpacity(s.bgcolor());
  if (isDragged()) overlay_opacity += theme.draggedOpacity(s.bgcolor());
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity > 0) {
    Color overlay = theme.color.primary;
    overlay.set_a(overlay_opacity);
    roo_display::OverlayFilter filter(s.out(), overlay, s.bgcolor());
    news.set_out(&filter);
    defaultPaint(news);
    needs_repaint_ = false;
    dirty_ = false;
    return;
  } else {
    defaultPaint(news);
    needs_repaint_ = false;
    dirty_ = false;
  }
}

// void Widget::setVisible(bool visible) {
//   if (visible == visible_) return;
//   visible_ = visible;
//   if (parent_ != nullptr) {
//     parent_->markDirty();
//     if (!visible) {
//       parent_->needs_repaint_ = true;
//     }
//   }
//   if (visible_) {
//     markDirty();
//     needs_repaint_ = true;
//   }
// }

}  // namespace roo_windows