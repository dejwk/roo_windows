#pragma once

#include <vector>

#include <Arduino.h>

#include "roo_display.h"
#include "roo_display/core/box.h"
#include "roo_display/filter/color_filter.h"

namespace roo_windows {

class Panel;

using roo_display::Box;
using roo_display::Color;
using roo_display::Surface;

static const uint32_t kPressAnimationMillis = 200;

static const uint16_t kWidgetEnabled = 0x0001;
static const uint16_t kWidgetDisabled = 0x0002;
static const uint16_t kWidgetHover = 0x0004;
static const uint16_t kWidgetFocused = 0x0008;
static const uint16_t kWidgetSelected = 0x0010;
static const uint16_t kWidgetActivated = 0x0020;
static const uint16_t kWidgetPressed = 0x0040;
static const uint16_t kWidgetDragged = 0x0080;
static const uint16_t kWidgetOn = 0x0100;
static const uint16_t kWidgetOff = 0x0200;
static const uint16_t kWidgetError = 0x0400;

static const uint16_t kWidgetHidden = 0x8000;

// enum State {
//   WIDGET_ENABLED,
//   WIDGET_DISABLED,
//   WIDGET_HOVER,
//   WIDGET_FOCUSED,
//   WIDGET_SELECTED,
//   WIDGET_ACTIVATED,
//   WIDGET_PRESSED,
//   WIDGET_DRAGGED,
//   WIDGET_ON,
//   WIDGET_OFF,
//   WIDGET_ERROR
// };

class TouchEvent {
 public:
  enum Type { DOWN, MOVE, UP };

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
//  // If repaint=false, the widget has not been painted-over, and so, it can
//  // update only the content that it knows has changed. Otherwise, it needs to
//  // fully redraw itself.
  // The Surface's offset and clipbox has been pre-initialized so that this
  // widget's top-left corner is painted at (0, 0), and the clip box is
  // constrained to this widget's bounds (and non-empty).
  virtual void paint(const Surface& s);

  virtual void defaultPaint(const Surface& s) {}

  virtual bool onTouch(const TouchEvent& event) {
    if (event.type() == TouchEvent::DOWN) {
      //state_ = WIDGET_PRESSED;
      pressed_time_millis_ = millis();
      pressed_x_ = event.x();
      pressed_y_ = event.y();
      Serial.printf("Pressed: %d, %d\n", pressed_x_, pressed_y_);

      //return onClick(event.x(), event.y());
    }
    return false;
  }

  virtual bool onClick(int16_t x, int16_t y) { return false; }

  bool isVisible() const { return (state_ & kWidgetHidden) == 0; }
  bool isEnabled() const { return (state_ & kWidgetEnabled) != 0; }

  bool isHover() const { return (state_ & kWidgetHover) != 0; }
  bool isFocused() const { return (state_ & kWidgetFocused) != 0; }
  bool isSelected() const { return (state_ & kWidgetSelected) != 0; }
  bool isActivated() const { return (state_ & kWidgetActivated) != 0; }
  bool isPressed() const { return (state_ & kWidgetPressed) != 0; }
  bool isDragged() const { return (state_ & kWidgetDragged) != 0; }

  void setVisible(bool visible);
  void setEnabled(bool enabled);
  void setSelected(bool selected);
  void setActivated(bool activated);

  virtual bool useOverlayOnActivation() { return true; }

  const Panel* parent() const { return parent_; }
  bool isDirty() const { return dirty_; }
  //State state() const { return state_; }

  uint32_t pressed_millis() const {
    return millis() - pressed_time_millis_;
  }

  int16_t pressed_x() const {
    return pressed_x_;
  }

  int16_t pressed_y() const {
    return pressed_y_;
  }

 protected:
  // The widget wants its paint() method to be called.
  bool dirty_;

  // The entire widget rectangle needs to be redrawn.
  bool needs_repaint_;

 private:
  // void update(const Surface& s) {
  //   paint(s);
  //   dirty_ = false;
  //   needs_repaint_ = false;
  // }

  bool is_press_animating() {
    return //state_ == WIDGET_PRESSED &&
           millis() - pressed_time_millis_ < kPressAnimationMillis;
  }

  friend class Panel;
  friend class WindowManager;

  Panel* parent_;
  Box parent_bounds_;
  uint16_t state_;

  int16_t pressed_x_;
  int16_t pressed_y_;
  uint32_t pressed_time_millis_;
};

// TODO: adjust for different screen densities.
static const int gridSize = 4;

}  // namespace roo_windows