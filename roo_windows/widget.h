#pragma once

#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"

namespace roo_windows {

class Panel;

using roo_display::Box;
using roo_display::Color;
using roo_display::Surface;

class TouchEvent {
 public:
  enum Type {
    DOWN, MOVE, UP
  };

  TouchEvent(Type type, int16_t x, int16_t y) : type_(type), x_(x), y_(y) {}

  Type type() const { return type_; }
  int16_t x() const { return x_; }
  int16_t y() const { return y_; }

 private:
  Type type_;
  int16_t x_, y_;
};

class Widget {
 public:
  Widget(Panel* parent, const Box& bounds);
  virtual ~Widget() {}

  void markDirty();

  int16_t width() const { return parent_bounds_.width(); }
  int16_t height() const { return parent_bounds_.height(); }
  Box bounds() const { return Box(0, 0, width() - 1, height() - 1); }

  const Box& parent_bounds() const { return parent_bounds_; }

  // Called as part of display update, for a visible, dirty widget.
  // If repaint=false, the widget has not been painted-over, and so, it can
  // update only the content that it knows has changed. Otherwise, it needs to
  // fully redraw itself.
  // The Surface's offset and clipbox has been pre-initialized so that this
  // widget's top-left corner is painted at (0, 0), and the clip box is
  // constrained to this widget's bounds (and non-empty).
  virtual void paint(const Surface& s, bool repaint) {}

  virtual bool onTouch(const TouchEvent& event) {
    if (event.type() == TouchEvent::DOWN) {
      return onClick(event.x(), event.y());
    }
    return false;
  }

  virtual bool onClick(int16_t x, int16_t y) {
    return false;
  }

  bool isVisible() const { return visible_; }

  void setVisible(bool visible);

  const Panel* parent() const { return parent_; }
  bool isDirty() const { return dirty_; }

 protected:
  bool dirty_;
  bool needs_repaint_;

 private:
  void update(const Surface& s, bool repaint) {
    paint(s, repaint || needs_repaint_);
    dirty_ = false;
    needs_repaint_ = false;
  }

  friend class Panel;
  friend class WindowManager;

  Panel* parent_;
  Box parent_bounds_;
  bool visible_;
};


// enum State {
//   ENABLED,
//   DISABLED,
//   HOVER,
//   FOCUSED,
//   SELECTED,
//   ACTIVATED,
//   PRESSED,
//   DRAGGED,
//   ON,
//   OFF,
//   ERROR
// };

// TODO: adjust for different screen densities.
static const int gridSize = 4;

}  // namespace roo_windows