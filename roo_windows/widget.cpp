#include "widget.h"

#include "roo_windows/panel.h"

namespace roo_windows {

Widget::Widget(Panel* parent, roo_display::Box bounds)
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

}  // namespace roo_windows