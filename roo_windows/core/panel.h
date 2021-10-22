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
  // implementation calls child->paintWidget(), thus omitting drawing of children
  // that aren't dirty.
  //
  // Calls paint() to actually paint the surface area (with the surface object
  // clipped to the invalidated region, and with the background color pre-set
  // to the panel's background).
  void paintWidgetContents(const Surface& s, Clipper& clipper) override;

  // Draws the surface area of this panel. The default implementation draws
  // a transparent rectangle. (Effectively, the rectangle is drawn in the panel's
  // background color, which is pre-set in the specified surface).
  void paint(const Surface& s) override;

  virtual bool onTouch(const TouchEvent& event);

 protected:
  const std::vector<std::unique_ptr<Widget>>& children() const {
    return children_;
  }

  virtual void paintChildren(const Surface& s, Clipper& clipper);

  void invalidateDescending() override;
  void invalidateDescending(const Box& box) override;
  bool invalidateBeneath(const Box& box, const Widget* subject) override;
  void markCleanDescending() override;

  virtual void propagateDirty(const Widget* child, const Box& box);
  virtual void childHidden(const Widget* child);

 private:
  friend class Widget;
  friend class ScrollablePanel;

  void addChild(Widget* child);

  std::vector<std::unique_ptr<Widget>> children_;
  Color bgcolor_;
  Box invalid_region_;
};

}  // namespace roo_windows
