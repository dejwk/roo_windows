#pragma once

#include <Arduino.h>

#include <functional>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"
#include "roo_display/filter/color_filter.h"

namespace roo_windows {

class Panel;
class MainWindow;

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

class TouchEvent {
 public:
  enum Type { PRESSED, RELEASED, MOVED, DRAGGED };

  TouchEvent(Type type, unsigned long start_time_ms, int16_t x_start,
             int16_t y_start, int16_t x_end, int16_t y_end)
      : type_(type),
        start_time_ms_(start_time_ms),
        x_start_(x_start),
        y_start_(y_start),
        x_end_(x_end),
        y_end_(y_end) {}

  Type type() const { return type_; }
  unsigned long startTime() const { return start_time_ms_; }
  int16_t startX() const { return x_start_; }
  int16_t startY() const { return y_start_; }
  int16_t x() const { return x_end_; }
  int16_t y() const { return y_end_; }

 private:
  Type type_;
  int16_t x_start_, y_start_, x_end_, y_end_;
  unsigned long start_time_ms_;
};

class Widget {
 public:
  Widget(Panel* parent, const Box& bounds);
  virtual ~Widget() {}

  // Causes the widget to request paint(). The widget decides which pixels
  // need re-drawing (it will receive FILL_MODE_VISIBLE).
  void markDirty();

  // Causes the widget to request paint(), replacing the entire ractangle area.
  void invalidate();

  int16_t width() const { return parent_bounds_.width(); }
  int16_t height() const { return parent_bounds_.height(); }
  Box bounds() const { return Box(0, 0, width() - 1, height() - 1); }

  const Box& parent_bounds() const { return parent_bounds_; }

  // Returns bounds in the device's coordinates.
  Box absolute_bounds() const;

  // Called as part of display update, for a visible, dirty widget.
  //  // If repaint=false, the widget has not been painted-over, and so, it can
  //  // update only the content that it knows has changed. Otherwise, it needs
  //  to
  //  // fully redraw itself.
  // The Surface's offset and clipbox has been pre-initialized so that this
  // widget's top-left corner is painted at (0, 0), and the clip box is
  // constrained to this widget's bounds (and non-empty).
  virtual void paint(const Surface& s);

  virtual void defaultPaint(const Surface& s) {}

  virtual MainWindow* getMainWindow();

  virtual Color background() const;

  virtual bool onTouch(const TouchEvent& event);

  void setOnClicked(std::function<void()> on_clicked) {
    on_clicked_ = on_clicked;
  }

  // virtual bool onClick(int16_t x, int16_t y) { return false; }

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
  void setPressed(bool pressed);
  void setDragged(bool dragged);

  virtual bool useOverlayOnActivation() const { return true; }
  virtual bool isClickable() const { return false; }
  virtual bool showClickAnimation() const { return true; }

  virtual roo_display::Color getOverlayColor() const;

  const Panel* parent() const { return parent_; }
  bool isDirty() const { return dirty_; }

 protected:
  // The widget wants its paint() method to be called.
  bool dirty_;

  // The entire widget rectangle needs to be redrawn.
  bool needs_repaint_;

 private:
  Panel* parent_;
  Box parent_bounds_;
  uint16_t state_;

  std::function<void()> on_clicked_;
};

// TODO: adjust for different screen densities.
static const int gridSize = 4;

}  // namespace roo_windows