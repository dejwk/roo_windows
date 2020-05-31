#include "widget.h"

#include "roo_windows/panel.h"

namespace roo_windows {

Widget::Widget(Panel* parent, const Box& bounds)
    : dirty_(true), parent_(parent), bounds_(bounds), visible_(true) {
  if (parent != nullptr) {
    parent->addChild(this);
  }
}

void Widget::markDirty() {
  dirty_ = true;
  Panel* c = parent_;
  while (c != nullptr) {
    c->has_dirty_descendants_ = true;
    c = c->parent_;
  }
}

void Widget::setVisible(bool visible) {
  if (visible == visible_) return;
  visible_ = visible;
  if (parent_ != nullptr) {
    parent_->markDirty();
  }
  if (visible_) markDirty();
}

}  // namespace roo_windows