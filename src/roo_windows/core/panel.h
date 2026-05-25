#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/color/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows {

using roo_display::Color;

/// Concrete container that stores its children in a `std::vector`.
///
/// Provides the default home for widgets that don't need a specialized child
/// collection. Subclasses are expected to add children through `add()` (or its
/// public wrappers) and to override `onMeasure()` / `onLayout()` when they need
/// a dynamic layout.
class Panel : public Container {
 public:
  Panel(ApplicationContext& context) : Container(context) {}

  ~Panel();

  /// Returns the panel's child vector.
  const std::vector<Widget*>& children() const { return children_; }

  Widget& child_at(int idx) { return *children_[idx]; }

  const Widget& child_at(int idx) const { return *children_[idx]; }

  /// Hook for subclasses that want to claim a touch event before it reaches
  /// children. The default implementation never intercepts.
  virtual bool onInterceptTouchEvent(const TouchEvent& event) { return false; }

 protected:
  /// Adds a child to this panel at optional initial bounds (`(0,0,-1,-1)`
  /// defers placement to the next layout pass).
  void add(WidgetRef child, const Rect& bounds = Rect(0, 0, -1, -1));

  /// Detaches and destroys every child.
  void removeAll();

  /// Detaches and destroys the last-added child.
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
  friend class SimpleScrollablePanel;
  friend class ScrollableBlitPanel;

  void addChild(Widget* child);
};

}  // namespace roo_windows
