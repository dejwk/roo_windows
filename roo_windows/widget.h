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

  virtual void drawContent(const Surface& s) const {}

  void markDirty();

  const Box& bounds() const { return bounds_; }

  virtual void update(const Surface& s) {
    if (!visible_) return;
    Surface cs = s;
    if (cs.clipToExtents(bounds()) == Box::CLIP_RESULT_EMPTY) {
      return;
    }
    drawContent(cs);
    dirty_ = false;
  }

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

 protected:
  bool dirty_;

 private:
  Panel* parent_;
  Box bounds_;
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