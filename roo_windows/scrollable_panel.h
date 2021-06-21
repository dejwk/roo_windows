#pragma once

namespace roo_windows {

static inline const Theme& getTheme(Panel* parent) {
  return parent == nullptr ? DefaultTheme() : parent->theme();
}

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
        dragged_x_(0),
        dragged_y_(0) {}

  void setWidth(int16_t width) { width_ = width; }
  void setHeight(int16_t height) { height_ = height; }

  void setOffset(int16_t dx, int16_t dy) {
    dx_ = dx;
    dy_ = dy;
  }

  void paint(const Surface& s) override {
    Surface news = s;
    news.set_dx(news.dx() - dx_);
    news.set_dy(news.dy() - dy_);
    Panel::paint(news);
  }

  void getAbsoluteBounds(Box* full, Box* visible) const override {
    Panel::getAbsoluteBounds(full, visible);
    *full =
        Box(full->xMin() - dx_, full->yMin() - dy_,
            full->xMin() - dx_ + width_ - 1, full->xMin() - dx_ + height_ - 1);
    *visible = Box::intersect(*full, *visible);
  }

  bool onTouch(const TouchEvent& event) override {
    TouchEvent shifted(
        event.type(), event.startTime(), event.startX() + dx_ + dragged_x_,
        event.startY() + dy_ + dragged_y_, event.x() + dx_, event.y() + dy_);
    if (event.type() == TouchEvent::RELEASED) {
      dragged_x_ = 0;
      dragged_y_ = 0;
    }
    if (Panel::onTouch(shifted)) return true;
    // Handle drag.
    if (shifted.type() == TouchEvent::DRAGGED) {
      int16_t drag_x_total = event.x() - event.startX();
      int16_t drag_y_total = event.y() - event.startY();
      int16_t drag_x_delta = drag_x_total - dragged_x_;
      int16_t drag_y_delta = drag_y_total - dragged_y_;
      if (dx_ - drag_x_delta + parent_bounds().width() > width_) {
        drag_x_delta = dx_ + parent_bounds().width() - width_;
      }
      if (dx_ - drag_x_delta < 0) {
        drag_x_delta = dx_;
      }
      if (dy_ - drag_y_delta + parent_bounds().height() > height_) {
        drag_y_delta = dy_ + parent_bounds().height() - height_;
      }
      if (dy_ - drag_y_delta < 0) {
        drag_y_delta = dy_;
      }
      if (drag_x_delta < 3 && drag_x_delta > -3 && drag_y_delta < 3 &&
          drag_y_delta > -3) {
        return true;
      }
      dx_ -= drag_x_delta;
      dy_ -= drag_y_delta;
      dragged_x_ += drag_x_delta;
      dragged_y_ += drag_y_delta;
      invalidate();
      return true;
    }
  }

 private:
  int16_t dx_, dy_;
  int16_t width_, height_;
  int16_t dragged_x_, dragged_y_;
};

}  // namespace roo_windows