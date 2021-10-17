#pragma once

#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

static inline const Theme& getTheme(Panel* parent) {
  return parent == nullptr ? DefaultTheme() : parent->theme();
}

static const float decceleration = 1000.0;
static const float maxVel = 1200.0;

class ScrollablePanel : public Panel {
 public:
  ScrollablePanel(Panel* parent, const Box& bounds)
      : ScrollablePanel(parent, bounds, getTheme(parent),
                        getTheme(parent).color.background) {}

  ScrollablePanel(Panel* parent, const Box& bounds, Color bgcolor)
      : ScrollablePanel(parent, bounds, getTheme(parent), bgcolor) {}

  ScrollablePanel(Panel* parent, const Box& bounds, const Theme& theme,
                  Color bgcolor)
      : Panel(parent, bounds, theme, bgcolor),
        width_(bounds.width()),
        height_(bounds.height()),
        dx_(0),
        dy_(0),
        scroll_start_vx_(0.0),
        scroll_start_vy_(0.0) {}

  void setWidth(int16_t width) {
    width = std::max(width, parent_bounds().width());
    width_ = width;
  }
  void setHeight(int16_t height) {
    height = std::max(height, parent_bounds().height());
    height_ = height;
  }

  // Sets the relative position of the underlying content, relative to the the
  // visible rectangle.
  void setOffset(int16_t dx, int16_t dy);

  void paintWidgetContents(const Surface& s, Clipper& clipper) override;

  void paintChildren(const Surface& s, Clipper& clipper) override;

  void getAbsoluteBounds(Box* full, Box* visible) const override;

  bool onTouch(const TouchEvent& event) override;

  void invalidateDescending(const Box& box) override {
    markInvalidated();
    if (invalid_region_.empty()) {
      invalid_region_ = box;
    } else {
      invalid_region_ = Box::extent(invalid_region_, box);
    }
    for (auto& child : children_) {
      Box cb = Box::intersect(invalid_region_, child->parent_bounds().translate(dx_, dy_));
      if (cb.empty()) continue;
      cb = cb.translate(-dx_ - child->parent_bounds().xMin(),
                        -dy_ - child->parent_bounds().yMin());
      child->invalidateDescending(cb);
    }
  }

  void childHidden(const Widget* child) override {
    invalidateBeneath(child->parent_bounds().translate(dx_, dy_), child);
  }

  bool invalidateBeneath(const Box& box, const Widget* subject) override {
    Box clipped = Box::intersect(box, maxBounds());
    if (clipped.empty()) return false;
    markInvalidated();
    if (invalid_region_.empty()) {
      invalid_region_ = clipped;
    } else {
      invalid_region_ = Box::extent(invalid_region_, clipped);
    }
    for (auto& child : children_) {
      if (child.get() == subject) return true;
      if (child->isVisible()) {
        Box adjusted =
            clipped.translate(-dx_ - child->xOffset(), -dy_ - child->yOffset());
        if (child->invalidateBeneath(adjusted, subject)) return true;
      }
    }
    return false;
  }

  void propagateDirty(const Widget* child, const Box& box) override {
    Panel::propagateDirty(child, box.translate(dx_, dy_));
  }

 private:
  // The current size of the virtual canvas. Always at least as
  // large as the bounded viewport.
  int16_t width_, height_;

  // The current offset of the virtual canvas, relative to the bounded
  // viewport, Always non-positive.
  int16_t dx_, dy_;

  // Captured dx_ and dy_ during drag and scroll animations.
  int16_t dxStart_, dyStart_;

  unsigned long scroll_start_time_;
  unsigned long scroll_end_time_;
  float scroll_start_vx_;  // initial scroll velocity in pixels/s.
  float scroll_start_vy_;  // initial scroll velocity in pixels/s.
  float scroll_decel_x_;
  float scroll_decel_y_;
};

}  // namespace roo_windows
