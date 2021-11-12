#pragma once

#include <Arduino.h>

#include <functional>
#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"
#include "roo_display/filter/color_filter.h"
#include "roo_display/image/image.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

class Panel;
class MainWindow;

typedef roo_display::RleImage4bppxBiased<roo_display::Alpha4,
                                         roo_display::PrgMemResource>
    MonoIcon;

using roo_display::Box;
using roo_display::Color;
using roo_display::Surface;

static const uint32_t kPressAnimationMillis = 200;

static const uint16_t kWidgetClippedInParent = 0x0001;

static const uint16_t kWidgetEnabled = 0x0002;
// static const uint16_t kWidgetDisabled = 0x0002;
static const uint16_t kWidgetHover = 0x0004;
static const uint16_t kWidgetFocused = 0x0008;
static const uint16_t kWidgetSelected = 0x0010;
static const uint16_t kWidgetActivated = 0x0020;
static const uint16_t kWidgetPressed = 0x0040;
static const uint16_t kWidgetDragged = 0x0080;
static const uint16_t kWidgetOn = 0x0100;
static const uint16_t kWidgetOff = 0x0200;
static const uint16_t kWidgetError = 0x0400;

// Widget is undergoing click animation.
static const uint16_t kWidgetClicking = 0x0800;

// The click has been released on top of the widget during click animation.
// We are registering this as a 'deferred click', to be delivered immediately
// when the click animation finishes.
static const uint16_t kWidgetClicked = 0x1000;

static const uint16_t kWidgetHidden = 0x8000;

static const uint8_t kDirty = 0x01;
static const uint8_t kInvalidated = 0x02;
static const uint8_t kLayoutRequested = 0x04;

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
  enum ParentClipMode { CLIPPED, UNCLIPPED };

  Widget(const Environment& env);
  virtual ~Widget() {}

  // Causes the widget to request paint(). The widget decides which pixels
  // need re-drawing.
  void markDirty() { markDirty(maxBounds()); }

  // Causes the widget to request paint(). The widget decides which pixels
  // need re-drawing. Provides a hint to the framework as to which pixels do
  // need to be redrawn.
  void markDirty(const Box& bounds);

  // Causes the widget to request paint(), replacing the entire rectangle area.
  void invalidateInterior();

  // Causes the widget to request paint(), replacing the specified area.
  void invalidateInterior(const Box& box);

  int16_t width() const { return parent_bounds_.width(); }
  int16_t height() const { return parent_bounds_.height(); }
  int16_t xOffset() const { return parent_bounds_.xMin(); }
  int16_t yOffset() const { return parent_bounds_.yMin(); }

  Box bounds() const { return Box(0, 0, width() - 1, height() - 1); }

  // Returns the rectangle that covers all of this widget and its descendants.
  virtual Box maxBounds() const { return bounds(); }

  virtual const Theme& theme() const;

  // Returns default color that should be used by monochromatic content.
  // Calculated by determining the effective background color of this widget
  // (which is the result of layering the, possibly semi-transparent,
  // backgrounds of the ancestors), and consulting the theme to see if it has
  // any registered 'onX' colors that correspond to that background. Usually,
  // the background turns out to be one of: .surface, .background,
  // .primary[Variant*], .secondary[Variant*], or .error, in which case the
  // default color resolves to .onSurface, .onBackground, .onPrimary,
  // .onSecondary, or .onError, appropriately. If all else fails, returns
  // .onBackground. NOTE: during the call to paint(const Surface& s), the
  // default color can also be determined by calling
  // theme().color.defaultColor(s.bgcolor()).
  roo_display::Color defaultColor() const {
    return theme().color.defaultColor(effectiveBackground());
  }

  const Box& parent_bounds() const { return parent_bounds_; }

  // Moves the widget to the new position, specified in the parent's
  // coordinates.
  virtual void moveTo(const Box& parent_bounds);

  // Returns bounds in the device's coordinates.
  virtual void getAbsoluteBounds(Box* full, Box* visible) const;

  // Called as part of display update, for a visible, dirty widget.
  // The Surface's offset and clipbox has been pre-initialized so that this
  // widget's top-left corner is painted at (0, 0), and the clip box is
  // constrained to this widget's bounds (and non-empty). Should return true if
  // the painting is considered done, or false if the widget wants to be
  // repainted again (e.g. because it is undergoing state change resulting in an
  // animation).
  virtual bool paint(const Surface& s) { return true; }

  virtual MainWindow* getMainWindow();
  virtual const MainWindow* getMainWindow() const;

  // Returns this widget's background. Transparent by default. Normally
  // overridden by panels, which usually have opaque backgrounds.
  virtual Color background() const { return roo_display::color::Transparent; }

  // Returns the effective background color of this widget. If this widget has a
  // non-opaque background, it is returned. If this widget has a fully
  // transparent background, the parent's effective background is returned.
  // Otherwise, if this widget has a semi-transparent background, the returned
  // color is the alpha-blend of the parent's effective background and this
  // widget's background.
  Color effectiveBackground() const;

  virtual bool onTouch(const TouchEvent& event);

  virtual void onPressed() {}
  virtual void onReleased() {}

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

  std::function<void()> getOnClicked() const { return on_clicked_; }

  // virtual bool onClick(int16_t x, int16_t y) { return false; }

  ParentClipMode getParentClipMode() const {
    return (state_ & kWidgetClippedInParent) != 0 ? Widget::UNCLIPPED
                                                  : Widget::CLIPPED;
  }

  void setParentClipMode(ParentClipMode mode);

  bool isVisible() const { return (state_ & kWidgetHidden) == 0; }
  bool isEnabled() const { return (state_ & kWidgetEnabled) != 0; }

  bool isHover() const { return (state_ & kWidgetHover) != 0; }
  bool isFocused() const { return (state_ & kWidgetFocused) != 0; }
  bool isSelected() const { return (state_ & kWidgetSelected) != 0; }
  bool isActivated() const { return (state_ & kWidgetActivated) != 0; }
  bool isPressed() const { return (state_ & kWidgetPressed) != 0; }
  bool isDragged() const { return (state_ & kWidgetDragged) != 0; }

  bool isClicking() const { return (state_ & kWidgetClicking) != 0; }
  bool isClicked() const { return (state_ & kWidgetClicked) != 0; }

  void setVisible(bool visible);
  void setEnabled(bool enabled);
  void setSelected(bool selected);
  void setActivated(bool activated);
  void setPressed(bool pressed);
  void setDragged(bool dragged);

  void clearClicked();

  virtual bool useOverlayOnActivation() const { return true; }
  virtual bool useOverlayOnPressAnimation() const { return false; }
  virtual bool isClickable() const { return false; }
  virtual bool showClickAnimation() const { return true; }

  virtual bool usesHighlighterColor() const { return false; }

  virtual uint8_t getOverlayOpacity() const;

  const Panel* parent() const { return parent_; }
  Panel* parent() { return parent_; }

  bool isDirty() const {
    return (redraw_status_ & (kDirty | kInvalidated)) != 0;
  }

  bool isInvalidated() const { return (redraw_status_ & kInvalidated) != 0; }

  bool isLayoutRequested() const {
    return (redraw_status_ & kLayoutRequested) != 0;
  }

  // Call this when something has changed which has invalidated the layout of
  // this widget. This will schedule a layout pass of the tree.
  void requestLayout();

 protected:
  // Marks the entire area of this widget, and all its descendants, as
  // invalidated (needing full redraw).
  virtual void invalidateDescending() { markInvalidated(); }

  // Marks the specified sub-area of this widget, and all its descendants, as
  // invalidated (needing full redraw).
  virtual void invalidateDescending(const Box& box) {
    if (!box.intersects(bounds())) return;
    markInvalidated();
  }

  // Recursively iterates through all widgets to the 'left' of subject (i.e.,
  // with lower 'Z' than the subject), propagating the invalidation rectangle.
  // Returns true when reaching subject on the iterative descent, which signals
  // that the iteration should be stopped.
  virtual bool invalidateBeneathDescending(const Box& box,
                                           const Widget* subject) {
    if (subject == this) return true;
    invalidateDescending(box);
    return false;
  }

  // Mark this widget and its descendants as non-dirty.
  virtual void markCleanDescending() { markClean(); }

  // Responsible for drawing a widget to the specified surface, and updating the
  // clipper to exclude the region covered by the widget from the clipper to
  // prevent it from being over-drawn. If called on a non-dirty widget, does not
  // need to draw anything, but it should still exclude the widget's area from
  // the clipper. The default implementaion calls paint() (but only in case the)
  // widget is actually dirty) to actually request the content to be drawn.
  // Widgets should generally not override this method, and override paint()
  // instead.
  virtual void paintWidgetContents(const Surface& s, Clipper& clipper);

  virtual void setParent(Panel* parent);

  virtual void setParentBounds(const Box& parent_bounds);

 private:
  friend class Panel;
  friend class ScrollablePanel;
  friend class MainWindow;

  // Called by the framework. Receives the parent's surface. Determines
  // state-related overlays and filters (enabled/disabled, clicking, etc.),
  // creates the new, offset surface with overlays and/or filters, and calls
  // paintWidgetContents().
  void paintWidget(const Surface& s, Clipper& clipper);

  void markClean() { redraw_status_ &= ~(kDirty | kInvalidated); }
  void markInvalidated() { redraw_status_ |= (kDirty | kInvalidated); }

  Panel* parent_;
  Box parent_bounds_;
  uint16_t state_;
  uint8_t redraw_status_;  // kDirty | kInvalidated.
  std::function<void()> on_clicked_;
};

// TODO: adjust for different screen densities.
static const int gridSize = 4;

// Debug support.
std::ostream& operator<<(std::ostream& os, const Widget& widget);

}  // namespace roo_windows
