#include "widget.h"

#include "roo_windows/panel.h"

namespace roo_windows {

Widget::Widget(Panel* parent, const Box& parent_bounds)
    : dirty_(true),
      needs_repaint_(true),
      parent_(parent),
      parent_bounds_(parent_bounds),
      visible_(true) {
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
  if (visible == visible_) return;
  visible_ = visible;
  if (parent_ != nullptr) {
    parent_->markDirty();
    if (!visible) {
      parent_->needs_repaint_ = true;
    }
  }
  if (visible_) {
    markDirty();
    needs_repaint_ = true;
  }
}

}  // namespace roo_windows