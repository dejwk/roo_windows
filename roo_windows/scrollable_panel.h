#pragma once

#include "roo_windows/panel.h"
#include "roo_windows/widget.h"

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

  void paint(const Surface& s) override;
  void getAbsoluteBounds(Box* full, Box* visible) const override;

  bool onTouch(const TouchEvent& event) override;

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