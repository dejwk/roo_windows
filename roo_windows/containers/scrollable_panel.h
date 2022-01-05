#pragma once

#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

static const float decceleration = 1000.0;
static const float maxVel = 1200.0;

class ScrollablePanel : public Panel {
 public:
  ScrollablePanel(const Environment& env, Widget* contents)
      : Panel(env), scroll_start_vx_(0.0), scroll_start_vy_(0.0) {
    add(contents);
  }

  Widget* swapContents(Widget* contents) { return swap(0, contents); }

  void setSize(int16_t width, int16_t height) {
    width = std::max(width, this->width());
    height = std::max(height, this->height());
    Widget* c = contents();
    Box bounds(0, 0, width - 1, height - 1);
    c->moveTo(bounds.translate(c->xOffset(), c->yOffset()));
  }

  Widget* contents() { return children_[0].get(); }
  const Widget& contents() const { return *children_[0]; }

  // Sets the relative position of the underlying content, relative to the the
  // visible rectangle.
  void scrollTo(int16_t x, int16_t y);

  // Adjusts the relative position of the underlying content by the specified
  // offset.
  void scrollBy(int16_t dx, int16_t dy) {
    scrollTo(dx + contents()->xOffset(), dy + contents()->yOffset());
  }

  void paintWidgetContents(const Surface& s, Clipper& clipper) override;

  bool onTouch(const TouchEvent& event) override;

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    measured_ = contents()->measure(MeasureSpec::Unspecified(width.value()),
                                    MeasureSpec::Unspecified(height.value()));
    return Dimensions(width.resolveSize(measured_.width()),
                      height.resolveSize(measured_.height()));
  }

  void onLayout(bool changed, const roo_display::Box& box) override {
    Widget* c = contents();
    Box bounds(0, 0, measured_.width() - 1, measured_.height() - 1);
    bounds = bounds.translate(c->xOffset(), c->yOffset());
    c->layout(bounds);
  }

 private:
  Dimensions measured_;

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
