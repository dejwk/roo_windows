#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/color/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows {

using roo_display::Color;

class Panel : public Container {
 public:
  Panel(const Environment& env) : Panel(env, roo_display::color::Transparent) {}

  Panel(const Environment& env, Color bgcolor);

  ~Panel();

  const std::vector<Widget*>& children() const { return children_; }

  Widget& child_at(int idx) { return *children_[idx]; }

  const Widget& child_at(int idx) const { return *children_[idx]; }

  virtual bool onInterceptTouchEvent(const TouchEvent& event) { return false; }

 protected:
  // Adds a child to this panel.
  void add(WidgetRef child, const Rect& bounds = Rect(0, 0, -1, -1));

  // Removes all children.
  void removeAll();

  // Removes and returns the last child.
  void removeLast();

  virtual int getChildrenCount() const { return (int)children_.size(); }
  virtual const Widget& getChild(int idx) const { return *children_[idx]; }
  virtual Widget& getChild(int idx) { return *children_[idx]; }

 protected:
  // Subclasses should avoid mutating this vector directly, preferring accessor
  // methods that initialize the children properly. Direct access is sometimes
  // useful when a child object gets moved and its pointer needs to be updated.
  std::vector<Widget*> children_;

 private:
  friend class ScrollablePanel;

  void addChild(Widget* child);
};

}  // namespace roo_windows
