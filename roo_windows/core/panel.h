#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

using roo_display::Color;

static const int16_t kTouchMargin = 8;

// A type of a smart pointer to a widget, with optional ownership.
class WidgetRef {
 public:
  // Creates a WidgetRef that owns the widget.
  template <typename T>
  WidgetRef(std::unique_ptr<T> w) : ptr_(w.release()), is_owned_(true) {}

  // Creates a WidgetRef that does not own the widget.
  WidgetRef(Widget& w) : ptr_(&w), is_owned_(false) {}

  WidgetRef(const WidgetRef& w) = delete;

  WidgetRef(WidgetRef&& w) {
    ptr_ = w.ptr_;
    is_owned_ = w.is_owned_;
    w.ptr_ = nullptr;
  }

  ~WidgetRef() {
    if (is_owned_) delete ptr_;
  }

  Widget* operator->() { return ptr_; }
  Widget& operator*() { return *ptr_; }

  const Widget* operator->() const { return ptr_; }
  const Widget& operator*() const { return *ptr_; }

  Widget* release() {
    Widget* result = ptr_;
    ptr_ = nullptr;
    return result;
  }

 private:
  friend class Panel;

  Widget* ptr_;
  bool is_owned_;
};

class Panel : public Widget {
 public:
  Panel(const Environment& env) : Panel(env, env.theme().color.background) {}

  Panel(const Environment& env, Color bgcolor);

  ~Panel();

  void setBackground(Color bgcolor) { bgcolor_ = bgcolor; }
  Color background() const override { return bgcolor_; }

  const Theme& theme() const override { return parent()->theme(); }

  // Paints the panel with all its children. If the panel isn't invalidated,
  // omits drawing the surface area; otherwise, draws the surface area over the
  // invalidated region.
  //
  // Calls paintChildren() to actually paint the children. The default
  // implementation calls child->paintWidget(), thus omitting drawing of
  // children that aren't dirty.
  //
  // Calls paint() to actually paint the surface area (with the surface object
  // clipped to the invalidated region, and with the background color pre-set
  // to the panel's background).
  void paintWidgetContents(const Surface& s, Clipper& clipper) override;

  // Draws the surface area of this panel. The default implementation draws
  // a transparent rectangle. (Effectively, the rectangle is drawn in the
  // panel's background color, which is pre-set in the specified surface).
  bool paint(const Surface& s) override;

  virtual bool onTouch(const TouchEvent& event);

  // Returns the minimum bounding box, in this widget's coordinates, that covers
  // all visible descendants, including those with parent clip mode = UNCLIPPED,
  // i.e. sticking out of the normal bounds.
  // In the common case, when there are no 'unclipped' descendants, equivalent
  // to bounds().
  Box maxBounds() const override;

  Dimensions getSuggestedMinimumDimensions() const override {
    // Panels are expected to override onMeasure, so this is rarely relevant
    // anyway.
    return Dimensions(0, 0);
  }

  Padding getDefaultPadding() const override { return Padding(0); }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContent(),
                         PreferredSize::WrapContent());
  }

  const std::vector<Widget*>& children() const { return children_; }

  Widget& child_at(int idx) { return *children_[idx]; }

  const Widget& child_at(int idx) const { return *children_[idx]; }

 protected:
  Panel(const Panel& other)
      : Widget(other),
        bgcolor_(other.bgcolor_),
        invalid_region_(0, 0, -1, -1),
        cached_max_bounds_(0, 0, -1, -1) {}

  // Adds a child to this panel.
  void add(WidgetRef child, const Box& bounds = Box(0, 0, -1, -1));

  // Removes all children.
  void removeAll();

  virtual void paintChildren(const Surface& s, Clipper& clipper);

  void invalidateDescending() override;
  void invalidateDescending(const Box& box) override;
  bool invalidateBeneathDescending(const Box& box,
                                   const Widget* subject) override;
  void markCleanDescending() override;

  // Called when a previously obstructed region has been uncovered, because
  // the 'widget' descendant has been hidden or moved. Calls itself recursively
  // to determine the nearest ancestor that fully covers the invalidated region
  // (which is the direct parent in the common case when 'widget' has parent
  // clipping mode == UNCLIPPED). Then, calls invalidateBeneathDescending to
  // iterate over and invalidate all widgets with smaller 'Z' that intersect the
  // invalidated region.
  virtual void invalidateBeneath(const Box& box, const Widget* widget,
                                 bool clip);

  // Called by markDirty(), to propagate the dirtiness through the ancestry.
  virtual void propagateDirty(const Widget* child, const Box& box);

  // Called on panel when its specified child gets hidden (or moved).
  virtual void childHidden(const Widget* child);

  // Called on panel when its specified child gets shown (or moved).
  virtual void childShown(const Widget* child);

  // Called recursively upwards, to extend the cached_max_bounds if needed,
  // when a descendant has been shown.
  void unclippedChildRectShown(const Box& box);

  // Called recursively upwards, to invalidate the cached_max_bounds if needed,
  // when a descendant has been hidden.
  void unclippedChildRectHidden(const Box& box);

  // Overrides onMeasure in the Widget, to propagate measurement requests to the
  // children. The default implementation is that of a static layout: it ignores
  // child measurements and simply returns the panel's current dimensions.
  //
  // Subclasses implementing dynamic layout need to overwrite this method. The
  // overwritten implementation can call measure() on its children as needed to
  // calculate its own dimensions, and it must call it on the children (even the
  // invisible ones) that have the 'isLayoutRequested()' flag set. If the
  // subclass wishes to cache child dimensions (e.g. to use them later in
  // onLayout()), it can use the CachedMeasure helper class.
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override;

  // Overrides onLayout in the Widget, to propagate the layout requests to the
  // children. The default ipmlementation is that of a static layout: it
  // only calls layout() on the children that have 'isLayoutReuired()' flag set,
  // and it calls it with the child's current position. In other words, the
  // positions of the children do not change, although the children get the
  // oppportunity to re-layout themselves internally.
  //
  // Subclasses implementing dynamic layout need to overwrite this method. THe
  // overwritten implementation can call layout() on its children as needed to
  // implement its layout, and it must call it o the children (even the
  // invisible ones, or the ones whose position do not change) that have
  // 'isLayoutRequired()' flag set.
  void onLayout(bool changed, const Box& box) override;

  void moveTo(const Box& parent_bounds) override;

 private:
  void invalidateCachedMaxBounds() { cached_max_bounds_ = Box(0, 0, -1, -1); }

  friend class Widget;
  friend class ScrollablePanel;

  void addChild(Widget* child);

  std::vector<Widget*> children_;

  Color bgcolor_;
  Box invalid_region_;
  mutable Box cached_max_bounds_;
};

}  // namespace roo_windows
