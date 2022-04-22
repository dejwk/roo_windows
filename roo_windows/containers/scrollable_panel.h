#pragma once

#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

static const float decceleration = 1000.0;
static const float maxVel = 1200.0;

class ScrollablePanel : public Panel {
 public:
  enum Direction { VERTICAL = 0, HORIZONTAL = 1, BOTH = 2 };

  ScrollablePanel(const Environment& env, WidgetRef contents,
                  Direction direction = VERTICAL)
      : Panel(env),
        direction_(direction),
        scroll_start_vx_(0.0),
        scroll_start_vy_(0.0) {
    add(std::move(contents));
  }

  void setSize(int16_t width, int16_t height) {
    width = std::max(width, this->width());
    height = std::max(height, this->height());
    Widget* c = contents();
    Box bounds(0, 0, width - 1, height - 1);
    c->moveTo(bounds.translate(c->xOffset(), c->yOffset()));
  }

  Widget* contents() { return children_[0]; }
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

  bool onDown(int16_t x, int16_t y) override;
  bool onScroll(int16_t dx, int16_t dy) override;
  bool onFling(int16_t vx, int16_t vy) override;

 protected:
  PreferredSize getPreferredSize() const override {
    return PreferredSize(direction_ != VERTICAL ? PreferredSize::WrapContent()
                                                : PreferredSize::MatchParent(),
                         direction_ != HORIZONTAL
                             ? PreferredSize::WrapContent()
                             : PreferredSize::MatchParent());
  }

  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override {
    if (direction_ != VERTICAL) {
      width = MeasureSpec::Unspecified(width.value());
    }
    if (direction_ != HORIZONTAL) {
      height = MeasureSpec::Unspecified(height.value());
    }
    measured_ = contents()->measure(width, height);
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
  Direction direction_;
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
