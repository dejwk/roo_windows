#pragma once

#include "roo_display/ui/alignment.h"
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
        alignment_(roo_display::kLeft | roo_display::kTop),
        scroll_start_vx_(0.0),
        scroll_start_vy_(0.0) {
    add(std::move(contents));
  }

  void setAlign(roo_display::Alignment alignment) {
    alignment_ = alignment;
    update();
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

  void scrollToTop() { scrollTo(contents()->xOffset(), 0); }

  void scrollToBottom();

  void update() { scrollBy(0, 0); }

  void paintWidgetContents(const Surface& s, Clipper& clipper) override;

  bool onInterceptTouchEvent(const TouchEvent& event) override;

  bool onDown(int16_t x, int16_t y) override;
  bool onScroll(int16_t dx, int16_t dy) override;
  bool onFling(int16_t vx, int16_t vy) override;

  bool supportsScrolling() const override { return true; }

 protected:
  PreferredSize getPreferredSize() const override {
    return PreferredSize(direction_ != VERTICAL ? PreferredSize::WrapContent()
                                                : PreferredSize::MatchParent(),
                         direction_ != HORIZONTAL
                             ? PreferredSize::WrapContent()
                             : PreferredSize::MatchParent());
  }

  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override;

  void onLayout(bool changed, const roo_display::Box& box) override;

 private:
  Direction direction_;
  roo_display::Alignment alignment_;

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
