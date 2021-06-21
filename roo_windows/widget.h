#pragma once

#include <Arduino.h>

#include <functional>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"
#include "roo_display/filter/color_filter.h"
#include "roo_windows/theme.h"

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
  enum Type { PRESSED, RELEASED, MOVED, DRAGGED, SWIPED };

  TouchEvent(Type type, unsigned long duration_ms, int16_t x_start,
             int16_t y_start, int16_t x_end, int16_t y_end)
      : type_(type),
        x_start_(x_start),
        y_start_(y_start),
        x_end_(x_end),
        y_end_(y_end),
        duration_ms_(duration_ms) {}

  Type type() const { return type_; }
  unsigned long duration() const { return duration_ms_; }
  int16_t startX() const { return x_start_; }
  int16_t startY() const { return y_start_; }
  int16_t x() const { return x_end_; }
  int16_t y() const { return y_end_; }

  int16_t dx() const { return x() - startX(); }
  int16_t dy() const { return y() - startY(); }

 private:
  Type type_;
  int16_t x_start_, y_start_, x_end_, y_end_;
  unsigned long duration_ms_;
};

class Widget {
 public:
  Widget(Panel* parent, const Box& bounds);
  virtual ~Widget() {}

  // Causes the widget to request paint(). The widget decides which pixels
  // need re-drawing (it will receive FILL_MODE_VISIBLE).
  void markDirty();

  // Causes the widget to request paint(), replacing the entire rectangle area.
  void invalidate();

  // Causes the widget to request paint(), replacing the specified area.
  void invalidate(const Box& box);

  int16_t width() const { return parent_bounds_.width(); }
  int16_t height() const { return parent_bounds_.height(); }
  Box bounds() const { return Box(0, 0, width() - 1, height() - 1); }
  const Theme& theme() const;

  const Box& parent_bounds() const { return parent_bounds_; }

  // Returns bounds in the device's coordinates.
  virtual void getAbsoluteBounds(Box* full, Box* visible) const;

  // Called as part of display update, for a visible, dirty widget.
  // The Surface's offset and clipbox has been pre-initialized so that this
  // widget's top-left corner is painted at (0, 0), and the clip box is
  // constrained to this widget's bounds (and non-empty).
  virtual void paint(const Surface& s);

  void clear(const Surface& s);

  virtual void defaultPaint(const Surface& s) {}

  virtual MainWindow* getMainWindow();
  virtual const MainWindow* getMainWindow() const;

  virtual Color background() const;

  virtual bool onTouch(const TouchEvent& event);

  virtual void onClicked() { on_clicked_(); }

  void setOnClicked(std::function<void()> on_clicked) {
    on_clicked_ = on_clicked;
  }

  void addOnClicked(std::function<void()> on_clicked) {
    if (on_clicked_ == nullptr) {
      on_clicked_ = on_clicked;
    } else {
      auto prev = on_clicked_;
      on_clicked_ = [prev, on_clicked]() {
        prev();
        on_clicked();
      };
    }
  }

  std::function<void()> getOnClicked() const {
    return on_clicked_;
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
  virtual bool useOverlayOnPressAnimation() const { return false; }
  virtual bool isClickable() const { return false; }
  virtual bool showClickAnimation() const { return true; }

  virtual bool usesHighlighterColor() const { return false; }

  virtual uint8_t getOverlayOpacity() const;

  const Panel* parent() const { return parent_; }
  Panel* parent() { return parent_; }
  bool isDirty() const { return dirty_; }
  bool isInvalidated() const { return needs_repaint_; }

  void updateBounds(const Box& bounds);

 protected:
  // The widget wants its paint() method to be called.
  bool dirty_;

  // The entire widget rectangle needs to be redrawn.
  bool needs_repaint_;

  virtual void invalidateDescending() { needs_repaint_ = true; }
  virtual void invalidateDescending(const Box& box) { needs_repaint_ = true; }

 private:
  friend class Panel;

  Panel* parent_;
  Box parent_bounds_;
  uint16_t state_;

  std::function<void()> on_clicked_;
};

// TODO: adjust for different screen densities.
static const int gridSize = 4;

}  // namespace roo_windows