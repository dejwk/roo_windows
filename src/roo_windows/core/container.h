#pragma once

#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/color/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows {

using roo_display::Color;

class Container : public Widget {
 public:
  Container(const Environment& env)
      : Container(env, roo_display::color::Transparent) {}

  Container(const Environment& env, Color bgcolor);

  void setBackground(Color bgcolor) {
    if (bgcolor_ == bgcolor) return;
    bgcolor_ = bgcolor;
    invalidateInterior();
  }

  Color background() const override { return bgcolor_; }

  const Theme& theme() const override { return parent()->theme(); }

  // Paints the panel with all its children. If the panel isn't invalidated,
  // omits drawing the canvas area; otherwise, draws the canvas area over the
  // invalidated region.
  //
  // Calls paintChildren() to actually paint the children. The default
  // implementation calls child->paintWidget(), thus omitting drawing of
  // children that aren't dirty.
  //
  // Calls paint() to actually paint the canvas area (with the canvas object
  // clipped to the invalidated region, and with the background color pre-set
  // to the panel's background).
  void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  // Draws the surface area of this panel. The default implementation draws
  // a transparent rectangle. (Effectively, the rectangle is drawn in the
  // panel's background color, which is pre-set in the specified canvas).
  void paint(const Canvas& s) const override;

  Widget* dispatchTouchDownEvent(XDim x, YDim y) override;

  // Returns the minimum bounding rect, in this widget's coordinates, that
  // covers all visible descendants, including those with parent clip mode =
  // UNCLIPPED, i.e. sticking out of the normal bounds. In the common case, when
  // there are no 'unclipped' descendants, equivalent to bounds().
  Rect maxBounds() const override;

  Rect maxParentBounds() const override;

  Dimensions getSuggestedMinimumDimensions() const override {
    // Containers are expected to override onMeasure, so this is rarely relevant
    // anyway.
    return Dimensions(0, 0);
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  // Should be overriden to return true by panels whose children, along with
  // their margins, are non-overlapping. In such case, decorations (borders and
  // shadows) which fit into the margins are rendered immediately after drawing
  // the child, which is faster than the general case where the rasterizables
  // need to be combined.
  virtual bool respectsChildrenBoundaries() const {
    return getChildrenCount() <= 1;
  }

  virtual bool onInterceptTouchEvent(const TouchEvent& event) { return false; }

  // Returns true if this container or any of its ancestors support scroll
  // events.
  bool isScrollable() const;

 protected:
  Container(const Container& other)
      : Widget(other),
        bgcolor_(other.bgcolor_),
        invalid_region_(0, 0, -1, -1),
        cached_max_bounds_(0, 0, -1, -1) {}

  void attachChild(WidgetRef child,
                   const Rect& initial_bounds = Rect(0, 0, -1, -1));

  void detachChild(Widget* child);

  virtual void paintChildren(const Canvas& canvas, Clipper& clipper);

  void invalidateDescending() override;
  void invalidateDescending(const Rect& rect) override;
  bool invalidateBeneathDescending(const Rect& rect,
                                   const Widget* subject) override;
  void markCleanDescending() override;

  // To be overwritten by scrolling containers, for which we want to delay the
  // recognition of touch down as 'press', to give the container a small delay
  // for properly recognizing drag and fling events (which get intercepted by
  // the container).
  virtual bool shouldDelayChildPressedState() { return false; }

  // Called when a previously obstructed region has been uncovered, because
  // the 'widget' descendant has been hidden or moved. Calls itself recursively
  // to determine the nearest ancestor that fully covers the invalidated region
  // (which is the direct parent in the common case when 'widget' has parent
  // clipping mode == UNCLIPPED). Then, calls invalidateBeneathDescending to
  // iterate over and invalidate all widgets with smaller 'Z' that intersect the
  // invalidated region.
  virtual void invalidateBeneath(const Rect& rect, const Widget* widget,
                                 bool clip);

  // Called by setDirty(), to propagate the dirtiness through the ancestry.
  virtual void propagateDirty(const Widget* child, const Rect& rect);

  // Called on panel when its specified child gets hidden (or moved).
  virtual void childHidden(const Widget* child);

  // Called on panel when its specified child gets shown (or moved).
  virtual void childShown(const Widget* child);

  // Called on panel when a specified region has been invalidated by the child,
  // and the child has some see-through border.
  virtual void childInvalidatedRegion(const Widget* child, Rect rect);

  // Called recursively upwards, to extend the cached_max_bounds if needed,
  // when a descendant has been shown.
  void unclippedChildRectShown(const Rect& rect);

  // Called recursively upwards, to invalidate the cached_max_bounds if needed,
  // when a descendant has been hidden.
  void unclippedChildRectHidden(const Rect& rect);

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
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

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
  void onLayout(bool changed, const Rect& rect) override;

  void moveTo(const Rect& parent_bounds) override;

  virtual int getChildrenCount() const = 0;
  virtual const Widget& getChild(int idx) const = 0;
  virtual Widget& getChild(int idx) = 0;

 private:
  void invalidateCachedMaxBounds() { cached_max_bounds_ = Rect(0, 0, -1, -1); }

  Canvas prepareCanvas(const Canvas& in, const Rect& invalid_region);

  void fastDrawChildShadow(Widget& child, const Canvas& canvas,
                           Clipper& clipper);

  friend class Widget;
  friend class ScrollableContainer;

  Color bgcolor_;
  Rect invalid_region_;
  mutable Rect cached_max_bounds_;
};

}  // namespace roo_windows
