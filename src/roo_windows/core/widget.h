#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "roo_display.h"
#include "roo_display/filter/color_filter.h"
#include "roo_display/image/image.h"
#include "roo_display/shape/point.h"
#include "roo_logging.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/padding.h"
#include "roo_windows/core/preferred_size.h"
#include "roo_windows/core/rect.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/touch_event.h"

namespace roo_windows {

class ClickAnimation;
class Container;
class MainWindow;
class GestureDetector;
class Task;
class Application;

typedef roo_display::RleImage4bppxBiased<roo_display::Alpha4,
                                         roo_display::ProgMemPtr>
    MonoIcon;

using roo_display::Color;

static const uint32_t kPressAnimationMillis = 200;

static const uint16_t kWidgetOwnedByParent = 0x0001;
static const uint16_t kWidgetClippedInParent = 0x0002;

static const uint16_t kWidgetEnabled = 0x0004;
// static const uint16_t kWidgetDisabled = 0x0002;
static const uint16_t kWidgetHover = 0x0008;
static const uint16_t kWidgetFocused = 0x0010;
static const uint16_t kWidgetSelected = 0x0020;
static const uint16_t kWidgetActivated = 0x0040;
static const uint16_t kWidgetPressed = 0x0080;
static const uint16_t kWidgetDragged = 0x0100;
static const uint16_t kWidgetOn = 0x0200;
static const uint16_t kWidgetOff = 0x0400;
static const uint16_t kWidgetError = 0x0800;

// Widget is undergoing click animation.
static const uint16_t kWidgetClicking = 0x1000;

static const uint16_t kWidgetHidden = 0x4000;
static const uint16_t kWidgetGone = 0x8000;

static const uint8_t kDirty = 0x01;
static const uint8_t kInvalidated = 0x02;
static const uint8_t kLayoutRequested = 0x04;
static const uint8_t kLayoutRequired = 0x08;

class Widget {
 public:
  enum ParentClipMode { CLIPPED, UNCLIPPED };
  enum OnOffState { OFF, INDETERMINATE, ON };
  enum Visibility { VISIBLE, INVISIBLE, GONE };

  Widget(const Environment& env);
  Widget(const Widget& w);
  virtual ~Widget() {}

  // Causes the widget to request paint(). The widget decides which pixels
  // need re-drawing.
  void setDirty() { setDirty(maxBounds()); }

  // Causes the widget to request paint(). The widget decides which pixels
  // need re-drawing. Provides a hint to the framework as to which pixels do
  // need to be redrawn.
  void setDirty(const Rect& bounds);

  // Causes the widget to request paint(), replacing the entire rectangle area.
  void invalidateInterior();

  // Causes the widget to request paint(), replacing the specified area.
  void invalidateInterior(const Rect& rect);

  XDim width() const { return parent_bounds_.width(); }
  YDim height() const { return parent_bounds_.height(); }

  // How many pixels from the left side of the parent to the left side of this
  // widget. Can be negative, if the widget does not fit within the parent.
  XDim offsetLeft() const { return parent_bounds_.xMin(); }

  // How many pixels from the top of the parent to the top side of this widget.
  // Can be negative, if the widget does not fit within the parent.
  YDim offsetTop() const { return parent_bounds_.yMin(); }

  // How many pixels from the left side of this widget to the right side of the
  // parent. Can be negative, if the widget does not fit within the parent.
  XDim offsetRight() const;

  // How many pixels from the bottom of this widget to the bottom of its parent.
  // Can be negative, if the widget does not fit within the parent.
  YDim offsetBottom() const;

  Rect bounds() const { return Rect(0, 0, width() - 1, height() - 1); }

  // Returns bounds of the shadow, in the parent's coordinates. When no shadow
  // (elevation = 0), equivalent to parent_bounds().
  Rect getParentBoundsOfShadow() const;

  Rect getSloppyTouchBounds() const;

  // Returns the rectangle that covers all of this widget and its descendants.
  virtual Rect maxBounds() const { return bounds(); }

  // Returns the rectangle that covers all of this widget and its descendants,
  // in the parent's coordinates.
  virtual Rect maxParentBounds() const { return parent_bounds(); }

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
  // .onBackground. NOTE: during the call to paint(const Canvas& canvas), the
  // default color can also be determined by calling
  // theme().color.defaultColor(s.bgcolor()).
  roo_display::Color defaultColor() const {
    return theme().color.defaultColor(effectiveBackground());
  }

  const Rect& parent_bounds() const { return parent_bounds_; }

  // The bounds (in parent's coordinates) extended, if needed, to some minimum
  // size that is a reasonable touch target. The bounds extended, if needed, to
  // some minimum size that is a reasonable touch target.
  Rect getSloppyTouchParentBounds() const;

  // Returns bounds in the device's coordinates.
  void getAbsoluteBounds(Rect& full, Rect& visible) const;

  // Returns the offset in the device's coordinates.
  void getAbsoluteOffset(XDim& dx, YDim& dy) const;

  // Called as part of display update, for a visible, dirty widget.
  // The Canvas's offset and clipbox has been pre-initialized so that this
  // widget's top-left corner is painted at (0, 0), and the clip box is
  // constrained to this widget's bounds (and non-empty).
  virtual void paint(const Canvas& s) const {}

  virtual MainWindow* getMainWindow();
  virtual const MainWindow* getMainWindow() const;

  virtual Task* getTask();

  Application* getApplication() const;

  ClickAnimation* getClickAnimation();

  const ClickAnimation* getClickAnimation() const;

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

  // Has no effect when getBorderStyle() reports outline_width = 0.
  virtual Color getOutlineColor() const { return theme().color.primary; }

  virtual Widget* dispatchTouchDownEvent(XDim x, YDim y);

  // Returns true if this widget is currently handling a touch gesture.
  bool isHandlingGesture() const;

  // Called by the framework when on the first touch event within the bounds of
  // this widget. If the method returns false, the event will be redirected to
  // this widget's parent, and this widget will not receive any further touch
  // events received as part of this gesture. Most widgets should not overwrite
  // this method. The default implementation calls the gesture detector which
  // may then call more specific widget methods.
  //
  // Returns true if the event is handled, false otherwise.
  virtual bool onTouchDown(XDim x, YDim y);

  // Called by the framework upon the 'move' event. If the method returns false,
  // the event will be redirected to this widget's parent, and this widget will
  // not receive any further touch events received as part of this gesture. Most
  // widgets should not overwrite this method. The default implementation calls
  // the gesture detector which may then call more specific widget methods.
  //
  // Returns true if the event is handled, false otherwise.
  virtual bool onTouchMove(XDim x, YDim y);

  // Called by the framework upon the 'up' event. If the method returns false,
  // the event will be redirected to this widget's parent. Most widgets should
  // not overwrite this method. The default implementation calls the gesture
  // detector which may then call more specific widget methods.
  //
  // Returns true if the event is handled, false otherwise.
  virtual bool onTouchUp(XDim x, YDim y);

  // Called by the framework when onTouchMove returns false, indicating that
  // this widget is no longer the target of the gesture.
  virtual void onTouchCanceled() {}

  // Called when a 'click' should be handled (either during or after
  // onSingleTapUp). For widgets that support click animation, the event is
  // triggered after the animation completes.
  virtual void onClicked();

  // Sets a handler to be called when the state of this widget changes due to
  // touch interaction. For simple clickable items, called by onClicked().
  void setOnInteractiveChange(std::function<void()> handler) {
    on_interactive_change_ = handler;
  }

  std::function<void()> getOnInteractiveChange() const {
    return on_interactive_change_;
  }

  // Called when the first 'down' event happens within the bounds of this
  // widget. Contains the touch coordinates relative to the widget. The widget
  // should return true if it handled the event. Otherwise, the event, and the
  // subsequent events, will be redirected to the parent.
  //
  // Most widgets should not override this method.
  virtual bool onDown(XDim x, YDim y);

  // Called when the gesture is recognized as press, rather than scroll.
  // Contains the coordinates of the original 'down' event, relative to the
  // widget. It happens with some delay since the onDown(), and only in case
  // there was no significant movement. The gesture can still turn into scroll,
  // even after this method is called.
  //
  // Most widgets should not override this method. The default implementation
  // handles state changes to show the widget as 'pressed', and to trigger click
  // animation.
  virtual void onShowPress(XDim x, YDim y);

  // Called when the gesture is recognized as a single tap, during the 'up'
  // event that ended the tap. Contains the coordinates of the 'up' event, in
  // the widget's coordinate system. The widget should return true if it handled
  // the event. Otherwise, the event, and the subsequent events, will be
  // redirected to the parent.
  //
  // Most widgets should not override this method, and override 'onClicked()'
  // instead. The default implementation handles state changes to show the
  // widget as no longer pressed, triggers click animation if it hasn't alrady
  // started, and schedules onClicked().
  //
  // If you would like to perform some additional action on the tap 'up', you
  // may want to override this method and call the superclass method at the
  // beginning of your implementation.
  virtual bool onSingleTapUp(XDim x, YDim y);

  // Should be overridden to return true by widgets that wish to handle the
  // onLongPoress events.
  virtual bool supportsLongPress() { return false; }

  // Should be overridden to return true by widgets that wish to handle the
  // onScroll and onFling events.
  virtual bool supportsScrolling() const { return false; }

  // Called when the gesture is recognized as a long press. The widget must
  // have returned true from supportsLongPress() in order to receive this
  // notification. Contains the coordinates of the original 'down' event,
  // relative to the widget. It happens with some delay since the onShowPress(),
  // and only in case there was no significant movement. The gesture can no
  // longer turn into scroll.
  virtual void onLongPress(XDim x, YDim y);

  // Called when the long press has finished, and the widget is no longer
  // touched. Contains the coordinates of the 'up' event, in the widget's
  // coordinate system.  The widget must have returned true from
  // supportsLongPress() in order to receive this notification.
  virtual void onLongPressFinished(XDim x, YDim y);

  // Called during the 'move' event, when the gesture is recognized as 'scroll'.
  // Contains the latest coordinates along with a delta since the last time
  // onScroll was called as part of this gesture.
  //
  // Widgets that support scrolling should override this method. The default
  // implementation clears the 'press' state and returns false.
  virtual bool onScroll(XDim x, YDim y, XDim dx, YDim dy);

  // Called during the 'up' event, when the gesture is recognized as 'fling'.
  // Contains the latest coordinates along with the velocity of the fling.
  //
  // Widgets that support fling (swipe) should override this method. The default
  // implementation clears the 'press' state and returns false.
  virtual bool onFling(XDim x, YDim y, XDim vx, YDim vy);

  virtual void onCancel();

  ParentClipMode getParentClipMode() const {
    return (state_ & kWidgetClippedInParent) != 0 ? Widget::UNCLIPPED
                                                  : Widget::CLIPPED;
  }

  void setParentClipMode(ParentClipMode mode);

  bool isOwnedByParent() const { return (state_ & kWidgetOwnedByParent) != 0; }

  Visibility visibility() const {
    return isGone() ? GONE : isVisible() ? VISIBLE : INVISIBLE;
  }

  bool isVisible() const {
    return (state_ & (kWidgetHidden | kWidgetGone)) == 0;
  }

  bool isGone() const { return (state_ & kWidgetGone) != 0; }

  bool isEnabled() const { return (state_ & kWidgetEnabled) != 0; }

  bool isHover() const { return (state_ & kWidgetHover) != 0; }
  bool isFocused() const { return (state_ & kWidgetFocused) != 0; }
  bool isSelected() const { return (state_ & kWidgetSelected) != 0; }
  bool isActivated() const { return (state_ & kWidgetActivated) != 0; }
  bool isPressed() const { return (state_ & kWidgetPressed) != 0; }
  bool isDragged() const { return (state_ & kWidgetDragged) != 0; }

  bool isClicking() const { return (state_ & kWidgetClicking) != 0; }

  void setVisibility(Visibility visibility);

  void setEnabled(bool enabled);
  void setSelected(bool selected);
  void setActivated(bool activated);
  void setPressed(bool pressed);
  void setDragged(bool dragged);

  void setClicking();

  enum OverlayType { OVERLAY_AREA, OVERLAY_POINT };

  virtual OverlayType getOverlayType() const { return OVERLAY_AREA; }

  virtual roo_display::FpPoint getPointOverlayFocus() const {
    const Rect& r = parent_bounds();
    return roo_display::FpPoint{0.5f * (float)(r.xMax() - r.xMin()),
                                0.5f * (float)(r.yMax() - r.yMin())};
  }

  virtual bool useOverlayOnActivation() const { return true; }
  virtual bool useOverlayOnPress() const { return true; }

  virtual bool isClickable() const;

  virtual bool showClickAnimation() const { return true; }

  virtual bool usesHighlighterColor() const { return false; }

  virtual uint8_t getOverlayOpacity() const;

  const Container* parent() const { return parent_; }
  Container* parent() { return parent_; }

  bool isDirty() const {
    return (redraw_status_ & (kDirty | kInvalidated)) != 0;
  }

  bool isInvalidated() const { return (redraw_status_ & kInvalidated) != 0; }

  bool isLayoutRequested() const {
    return (redraw_status_ & kLayoutRequested) != 0;
  }

  bool isLayoutRequired() const {
    return (redraw_status_ & kLayoutRequired) != 0;
  }

  virtual Dimensions getSuggestedMinimumDimensions() const = 0;

  virtual Padding getPadding() const { return Padding(0); }
  virtual Margins getMargins() const { return Margins(0); }

  // Returns the current elevation of the widget. Non-zero elevation causes a
  // shadow to be drawn. The elevation can be in the range [0-31]. By default,
  // widgets have elevation = 0. Subclasses can override. When the reported
  // elevation changes in the subclass, elevationChanged() must be called to
  // ensure that the screen is properly redrawn.
  virtual uint8_t getElevation() const { return 0; }

  // Returns the border style of this widget. By default, widgets have sharp
  // corners and no outline. Subclasses can override.
  virtual BorderStyle getBorderStyle() const { return BorderStyle(); }

  // Returns dimensions that make the widget 'look good'. Inclusive of padding.
  virtual Dimensions getNaturalDimensions() const {
    Dimensions d = getSuggestedMinimumDimensions();
    Padding p = getPadding();
    return Dimensions(d.width() + p.left() + p.right(),
                      d.height() + p.top() + p.bottom());
  }

  virtual PreferredSize getPreferredSize() const {
    Dimensions d = getNaturalDimensions();
    return PreferredSize(PreferredSize::ExactWidth(d.width()),
                         PreferredSize::ExactHeight(d.height()));
  }

  // Called to find out how big a widget should be. The parent supplies
  // constraint information in the width and height parameters.
  //
  // The actual measurement work of a view is performed in onMeasure(int, int),
  // called by this method.
  Dimensions measure(WidthSpec width, HeightSpec height);

  // Call this when something has changed which has invalidated the layout of
  // this widget. This will schedule a layout pass of the tree.
  void requestLayout();

  // Assign a size and position to a widget and all of its descendants.
  //
  // This is the second phase of the layout mechanism. (The first is measuring).
  // In this phase each parent calls layout on all of its children to position
  // them. This is typically done using the child measurements that were
  // calculated in the measure pass().
  //
  // Derived classes should not override this method. Derived classes with
  // children should override onLayout. In that method, they should call layout
  // on each of their children.
  //
  // This method should generally not be called outside of the context of
  // implementing onLayout() in a Container. Any position changes made by
  // calling this method directly may get invalidated by the next layout pass.
  // It is OK, however, to call it on widgets belonging to a StaticLayout, since
  // it does nothing interesting in its layout pass.
  void layout(const Rect& rect);

 protected:
  bool isOn() const { return (state_ & kWidgetOn) != 0; }
  bool isOff() const { return (state_ & kWidgetOff) != 0; }

  void setOn() { setOnOffState(Widget::ON); }
  void setOff() { setOnOffState(Widget::OFF); }
  void toggle();

  OnOffState onOffState() const;
  void setOnOffState(OnOffState state);

  void triggerInteractiveChange();

  // Should be called by a child whose elevation has changed, and the child
  // wants the shadow to be redrawn. The argument should indicate the higher of
  // {before, after} elevations.
  void elevationChanged(int higherElevation);

  // Marks the entire area of this widget, and all its descendants, as
  // invalidated (needing full redraw).
  virtual void invalidateDescending() { markInvalidated(); }

  // Marks the specified sub-area of this widget, and all its descendants, as
  // invalidated (needing full redraw).
  virtual void invalidateDescending(const Rect& rect) {
    if (!rect.intersects(bounds())) return;
    markInvalidated();
  }

  // Recursively iterates through all widgets to the 'left' of subject (i.e.,
  // with lower 'Z' than the subject), propagating the invalidation rectangle.
  // Returns true when reaching subject on the iterative descent, which signals
  // that the iteration should be stopped.
  virtual bool invalidateBeneathDescending(const Rect& rect,
                                           const Widget* subject) {
    if (subject == this) return true;
    invalidateDescending(rect);
    return false;
  }

  // Mark this widget and its descendants as non-dirty.
  virtual void markCleanDescending() { markClean(); }

  // Responsible for drawing a widget to the specified canvas, and updating the
  // clipper to exclude the region covered by the widget from the clipper to
  // prevent it from being over-drawn. If called on a non-dirty widget, does not
  // need to draw anything, but it should still exclude the widget's area from
  // the clipper. The default implementaion calls paint() (but only in case the)
  // widget is actually dirty) to actually request the content to be drawn.
  //
  // Widgets should generally not override this method, and override paint()
  // instead. Two common scenarios when you might want to override this method
  // include:
  // * Animating widgets. Call the superclass method, and then conditionally
  //   call setDirty() if the animation shall continue.
  // * Mutating state - e.g. caching some paint-related values. Call the
  //   superclass method, and then do the necessary caching.
  virtual void paintWidgetContents(const Canvas& s, Clipper& clipper);

  // Draws borders and shadows, and applies exclusions.
  virtual void finalizePaintWidget(const Canvas& s, Clipper& clipper,
                                   const OverlaySpec& overlay_spec) const;

  virtual void setParent(Container* parent, bool is_owned);

  // Called to retrieve measurement information from the widget.
  //
  // The default implementation used getSuggestedMinimumDimensions() to
  // determine the 'natural' dimensions of the widget, and then returns
  // Dimensions that also takes into account the measure spec: for
  // MeasureSpecKind::UNSPECIFIED, it returns the preferred minimum dimension,
  // and for MeasureSpecKind::AT_MOST and MeasureSpecKind::EXACTLY, uses the
  // value provided in the measure spec. (Consequently, the returned dimension
  // may be greater than the preferred minimum if the measure spec allows it).
  // For simple widgets, prefer overwriting getDefaultMinimumDimensions() and
  // getDefaultSize() to overriding this method, when possible.
  virtual Dimensions onMeasure(WidthSpec width, HeightSpec height);

  virtual void onRequestLayout();

  // Called from layout after the new position of this widget has been
  // established. This method is not invoked in case the position of the widget
  // did not change *and* the layout for this widget has not been explicitly
  // requested. When this method is called, the widget has already been moved to
  // the new position.
  virtual void onLayout(bool changed, const Rect& rect) {}

  // Moves the widget to the new position, specified in the parent's
  // coordinates.
  //
  // This method should not be called during paint().
  virtual void moveTo(const Rect& parent_bounds);

  void markClean() { redraw_status_ &= ~(kDirty | kInvalidated); }

  // Helper method that, given the alignment, adds this widget's padding
  // to that alignment, from the appropriate side. Centered and absolute
  // alignments are not affected.
  roo_display::Alignment adjustAlignment(
      roo_display::Alignment alignment = roo_display::kMiddle |
                                         roo_display::kCenter) const {
    if ((alignment.h().dst() == roo_display::ANCHOR_MID ||
         alignment.h().dst() == roo_display::ANCHOR_ORIGIN) &&
        (alignment.v().dst() == roo_display::ANCHOR_MID ||
         alignment.v().dst() == roo_display::ANCHOR_ORIGIN)) {
      return alignment;
    }
    roo_display::HAlign h = alignment.h();
    roo_display::VAlign v = alignment.v();
    Padding p = getPadding();
    switch (h.dst()) {
      case roo_display::ANCHOR_MIN: {
        h = h.shiftBy(p.left());
        break;
      }
      case roo_display::ANCHOR_MAX: {
        h = h.shiftBy(-p.right());
        break;
      }
      default: {
      }
    }
    switch (v.dst()) {
      case roo_display::ANCHOR_MIN: {
        v = v.shiftBy(p.top());
        break;
      }
      case roo_display::ANCHOR_MAX: {
        v = v.shiftBy(-p.bottom());
        break;
      }
      default: {
      }
    }
    return roo_display::Alignment(h, v);
  }

 private:
  friend class Container;
  friend class ScrollablePanel;
  friend class MainWindow;
  friend class OverlaySpec;
  friend class ClickAnimation;

  void clearClicking();

  // Called by the framework. Receives the parent's canvas. Determines
  // state-related overlays and filters (enabled/disabled, clicking, etc.),
  // creates the new, offset canvas with overlays and/or filters, and calls
  // paintWidgetContents().
  void paintWidget(const Canvas& s, Clipper& clipper);

  void paintWidgetModded(Canvas& canvas, const OverlaySpec& spec,
                         Clipper& clipper);

  Canvas prepareCanvas(const Canvas& in);
  Canvas prepareContentsCanvas(const Canvas& in);

  // Helper functions for paintWidget.
  void paintWidgetInteriorWithOverlays(Canvas& s, Clipper& clipper,
                                       const OverlaySpec& overlay_spec);

  void markDirty() { redraw_status_ |= kDirty; }
  void markInvalidated() { redraw_status_ |= (kDirty | kInvalidated); }

  void markLayoutRequested() { redraw_status_ |= kLayoutRequested; }

  void setParentBounds(const Rect& parent_bounds);

  Container* parent_;
  Rect parent_bounds_;
  uint16_t state_;

  // kDirty | kInvalidated | kLayout{Requested|Required}
  uint8_t redraw_status_;

  std::function<void()> on_interactive_change_;
};

// TODO: adjust for different screen densities.
static const int gridSize = 4;

using WidgetCreatorFn = std::function<std::unique_ptr<Widget>()>;

template <typename Src>
using WidgetSetterFn = std::function<void(const Src& src, Widget& dest)>;

}  // namespace roo_windows

roo_logging::Stream& operator<<(roo_logging::Stream& os,
                                const roo_windows::Widget& widget);
