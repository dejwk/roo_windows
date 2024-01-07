#include "roo_windows/core/panel.h"

#include "roo_display/core/box.h"
#include "roo_display/core/device.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/shape/basic.h"
#include "roo_windows/core/main_window.h"

namespace roo_windows {

using roo_display::Color;
using roo_display::DisplayOutput;

Panel::Panel(const Environment& env, roo_display::Color bgcolor)
    : Container(env, bgcolor) {}

Panel::~Panel() {
  for (auto* c : children_) {
    if (c->isOwnedByParent()) delete c;
  }
}

void Panel::add(WidgetRef ref, const Rect& bounds) {
  Widget* child = ref.get();
  children_.push_back(child);
  attachChild(std::move(ref), bounds);
}

void Panel::removeAll() {
  for (auto* c : children_) {
    detachChild(c);
  }
  children_.clear();
}

void Panel::removeLast() {
  Widget* w = children_.back();
  children_.pop_back();
  detachChild(w);
}

}  // namespace roo_windows
