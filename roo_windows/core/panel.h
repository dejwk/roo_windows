#pragma once

#include <memory>
#include <vector>

#include "press_overlay.h"
#include "roo_display.h"
#include "roo_display/core/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

using roo_display::Color;

static const int16_t kTouchMargin = 8;

class Panel : public Widget {
 public:
  Panel(Panel* parent, const Box& bounds);

  Panel(Panel* parent, const Box& bounds, roo_display::Color bgcolor);

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

  void moveTo(const Box& parent_bounds) override;

 protected:
  const std::vector<std::unique_ptr<Widget>>& children() const {
    return children_;
  }

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

 private:
  void invalidateCachedMaxBounds() { cached_max_bounds_ = Box(0, 0, -1, -1); }

  friend class Widget;
  friend class ScrollablePanel;

  void addChild(Widget* child);

  std::vector<std::unique_ptr<Widget>> children_;

  Color bgcolor_;
  Box invalid_region_;
  mutable Box cached_max_bounds_;
};

}  // namespace roo_windows
