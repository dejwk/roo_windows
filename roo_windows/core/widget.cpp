#include "roo_windows/core/widget.h"

#include <cmath>

#include "roo_display/filter/foreground.h"
#include "roo_glog/logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/core/rtti.h"

namespace roo_windows {

static int16_t kMinSloppyTouchTargetSpan = 50;

// If the end-to-end duration of touch it shorted than this threshold,
// it is always interpreted as a 'click' rather than 'drag'; i.e.
// it is registered in the target component even if the exit coordinates
// fall outside of the component. (It helps to overcome touch panel noise).
static const long int kClickDurationThresholdMs = 200;

// When the user presses on a clickable item and drags the finger within
// this radius, the drag events are ignored and considered 'random noise'.
// The item remains clicked, and when the press is released, the gesture
// is interpreted as 'click'. If the user drags beyond this thredhold,
// however, then the item gets 'unclicked' and the gesture gets interpreted
// as drag.
static const long int kClickStickinessRadius = 40;

// If working with the fast display, set it to false, so that click animations
// finish gracefully. The default is true, because SPI displays are slow, and
// continuing to animate a moving widget (e.g. when its) container is scrolling)
// competes with the other display updates (e.g. updating the scrolled content),
// making it stutter.
static const bool kTerminateAnimationsOnCancel = true;

Widget::Widget(const Environment& env)
    : parent_(nullptr),
      parent_bounds_(0, 0, -1, -1),
      state_(kWidgetEnabled),
      redraw_status_(kDirty | kInvalidated),
      on_clicked_(nullptr) {}

Widget::Widget(const Widget& w)
    : parent_(nullptr),
      parent_bounds_(0, 0, -1, -1),
      state_(kWidgetEnabled),
      redraw_status_(kDirty | kInvalidated),
      on_clicked_(nullptr) {}

MainWindow* Widget::getMainWindow() {
  return parent_ == nullptr ? nullptr : parent_->getMainWindow();
}

Application* Widget::getApplication() {
  MainWindow* w = getMainWindow();
  return (w == nullptr) ? nullptr : &w->app();
}

const MainWindow* Widget::getMainWindow() const {
  return parent_->getMainWindow();
}

Task* Widget::getTask() { return parent_->getTask(); }

ClickAnimation* Widget::getClickAnimation() {
  MainWindow* w = getMainWindow();
  return (w == nullptr) ? nullptr : &w->click_animation();
}

void Widget::getAbsoluteBounds(Rect& full, Rect& visible) const {
  if (parent() == nullptr) {
    full = visible = parent_bounds();
  } else {
    parent()->getAbsoluteBounds(full, visible);
    full = parent_bounds().translate(full.xMin(), full.yMin());
    visible =
        Rect::Intersect(visible.translate(full.xMin(), full.yMin()), full);
  }
}

void Widget::getAbsoluteOffset(XDim& dx, YDim& dy) const {
  if (parent() == nullptr) {
    dx = 0;
    dy = 0;
  } else {
    parent()->getAbsoluteOffset(dx, dy);
  }
  dx += xOffset();
  dy += yOffset();
}

namespace {

Rect slopify(Rect bounds) {
  XDim w = bounds.width();
  YDim h = bounds.height();
  XDim xMin = bounds.xMin();
  YDim yMin = bounds.yMin();
  XDim xMax = bounds.xMax();
  YDim yMax = bounds.yMax();
  if (w < kMinSloppyTouchTargetSpan) {
    xMin -= (kMinSloppyTouchTargetSpan - w) / 2;
    xMax = xMin + kMinSloppyTouchTargetSpan - 1;
  }
  if (h < kMinSloppyTouchTargetSpan) {
    yMin -= (kMinSloppyTouchTargetSpan - h) / 2;
    yMax = yMin + kMinSloppyTouchTargetSpan - 1;
  }
  return Rect(xMin, yMin, xMax, yMax);
}

}  // namespace

Rect Widget::getSloppyTouchParentBounds() const {
  return slopify(parent_bounds());
}

Rect Widget::getSloppyTouchBounds() const { return slopify(bounds()); }

Color Widget::effectiveBackground() const {
  roo_display::Color bgcolor = background();
  return bgcolor.opaque() || parent() == nullptr
             ? bgcolor
             : roo_display::alphaBlend(parent()->effectiveBackground(),
                                       bgcolor);
}

// Returns the theme used by this widget. Defaults to the parent's theme
// (and, ultimately, to DefaultTheme(), if not otherwise specified).
const Theme& Widget::theme() const { return parent_->theme(); }

void Widget::markDirty(const Rect& bounds) {
  redraw_status_ |= kDirty;
  if (parent_ != nullptr) {
    parent_->propagateDirty(this, bounds.translate(xOffset(), yOffset()));
  }
}

void Widget::invalidateInterior() {
  invalidateDescending();
  markDirty();
}

void Widget::invalidateInterior(const Rect& rect) {
  invalidateDescending(rect);
  markDirty(rect);
}

void Widget::requestLayout() {
  if (isLayoutRequested()) return;
  onRequestLayout();
}

Dimensions Widget::measure(WidthSpec width, HeightSpec height) {
  Dimensions result = onMeasure(width, height);
  redraw_status_ |= kLayoutRequired;
  return result;
}

void Widget::layout(const Rect& rect) {
  bool changed = (rect != parent_bounds());
  if (changed || isLayoutRequired()) {
    moveTo(rect);
    onLayout(changed, rect);
  }
  redraw_status_ &= ~(kLayoutRequired | kLayoutRequested);
}

void Widget::onRequestLayout() {
  markLayoutRequested();
  if (parent() != nullptr) parent()->requestLayout();
}

namespace {

// Utility to return a default width. Uses the supplied width if the WidthSpec
// imposed no constraints. Will get larger if allowed by the WidthSpec.
static XDim getDefaultWidth(XDim width, WidthSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return width;
    default:
      return spec.value();
  }
}

// Utility to return a default height. Uses the supplied height if the
// HeightSpec imposed no constraints. Will get larger if allowed by the
// HeightSpec.
static YDim getDefaultHeight(YDim height, HeightSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return height;
    default:
      return spec.value();
  }
}

}  // namespace

Dimensions Widget::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions suggestedMin = getSuggestedMinimumDimensions();
  return Dimensions(getDefaultWidth(suggestedMin.width(), width),
                    getDefaultHeight(suggestedMin.height(), height));
}

void Widget::setParentClipMode(ParentClipMode mode) {
  if (mode == getParentClipMode()) return;
  bool visible = isVisible();
  if (visible) {
    setVisibility(INVISIBLE);
  }
  state_ ^= kWidgetClippedInParent;
  if (visible) {
    setVisibility(VISIBLE);
  }
}

void Widget::setVisibility(Visibility visibility) {
  Visibility previous = this->visibility();
  if (visibility == previous) return;
  state_ &= ~(kWidgetHidden | kWidgetGone);
  state_ |= (kWidgetHidden * (visibility == INVISIBLE));
  state_ |= (kWidgetGone * (visibility == GONE));
  if (previous == GONE) {
    // Layout might have been skipped before, leaving the request pending.
    if (parent() != nullptr && isLayoutRequested()) parent()->requestLayout();
    requestLayout();
  } else if (visibility == GONE) {
    if (parent() != nullptr) parent()->requestLayout();
  }
  if (visibility == VISIBLE) {
    invalidateInterior();
    if (previous != VISIBLE && parent() != nullptr) parent()->childShown(this);
  } else {
    setPressed(false);
    state_ &= ~kWidgetClicking;
    if (previous == VISIBLE && parent() != nullptr) parent()->childHidden(this);
  }
}

void Widget::setEnabled(bool enabled) {
  if (isEnabled() == enabled) return;
  state_ ^= kWidgetEnabled;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setSelected(bool selected) {
  if (selected == isSelected()) return;
  state_ ^= kWidgetSelected;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setActivated(bool activated) {
  if (activated == isActivated()) return;
  state_ ^= kWidgetActivated;
  if (!isVisible()) return;
  markDirty();
  if (useOverlayOnActivation()) {
    invalidateInterior();
  }
}

void Widget::setPressed(bool pressed) {
  if (pressed == isPressed()) return;
  state_ ^= kWidgetPressed;
  if (!isVisible()) return;
  invalidateInterior();
}

void Widget::setDragged(bool dragged) {
  if (dragged == isDragged()) return;
  state_ ^= kWidgetDragged;
  if (!isVisible()) return;
  invalidateInterior();
}

Widget::OnOffState Widget::onOffState() const {
  return isOn() ? (!isOff() ? Widget::ON : Widget::INDETERMINATE)
                : (isOff() ? Widget::OFF : Widget::INDETERMINATE);
}

void Widget::toggle() {
  OnOffState state = onOffState();
  if (state == Widget::ON) {
    setOff();
  } else if (state == Widget::OFF) {
    setOn();
  }
}

void Widget::setOnOffState(OnOffState state) {
  if (onOffState() == state) return;
  state_ &= ~(kWidgetOn | kWidgetOff);
  if (state == Widget::ON) {
    state_ |= kWidgetOn;
  } else if (state == Widget::OFF) {
    state_ |= kWidgetOff;
  }
  markDirty();
}

void Widget::setClicking() { state_ |= kWidgetClicking; }

void Widget::setParent(Panel* parent, bool owned) {
  CHECK(parent_ == nullptr || parent == nullptr)
      << "widget " << *this << " being added, but"
      << " it already has a parent";
  parent_ = parent;
  if (owned) {
    state_ |= kWidgetOwnedByParent;
  } else {
    state_ &= ~kWidgetOwnedByParent;
  }
}

void Widget::setParentBounds(const Rect& parent_bounds) {
  if (parent_bounds == parent_bounds_) return;
  if (isGone() || parent() == nullptr) {
    parent_bounds_ = parent_bounds;
    return;
  }
  invalidateInterior();
  if (parent() != nullptr) parent()->childHidden(this);
  parent_bounds_ = parent_bounds;
  invalidateInterior();
  if (parent() != nullptr) parent()->childShown(this);
}

void Widget::moveTo(const Rect& parent_bounds) {
  setParentBounds(parent_bounds);
}

uint8_t Widget::getOverlayOpacity() const {
  Color bgcolor = background();
  uint16_t overlay_opacity = 0;
  const Theme& myTheme = theme();
  if (isHover()) overlay_opacity += myTheme.hoverOpacity(bgcolor);
  if (isFocused()) overlay_opacity += myTheme.focusOpacity(bgcolor);
  if (isSelected()) overlay_opacity += myTheme.selectedOpacity(bgcolor);
  if (isActivated() && useOverlayOnActivation()) {
    overlay_opacity += myTheme.activatedOpacity(bgcolor);
  }
  if (isClicking()) {
    if (useOverlayOnPressAnimation()) {
      overlay_opacity += myTheme.pressedOpacity(bgcolor);
    }
  } else if (isPressed() && useOverlayOnPress()) {
    overlay_opacity += myTheme.pressedOpacity(bgcolor);
  }
  if (isDragged()) overlay_opacity += myTheme.draggedOpacity(bgcolor);
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity == 0) return 0;  // roo_display::color::Transparent;
  return overlay_opacity;
}

namespace {

Color getOverlayColor(const Widget& widget, const Canvas& canvas) {
  uint8_t overlay_opacity = widget.getOverlayOpacity();
  if (overlay_opacity == 0) {
    return roo_display::color::Transparent;
  }
  const Theme& myTheme = widget.theme();
  Color overlay = widget.usesHighlighterColor()
                      ? myTheme.color.highlighterColor(canvas.bgcolor())
                      : myTheme.color.defaultColor(canvas.bgcolor());
  overlay.set_a(overlay_opacity);
  return overlay;
}

Color getClickAnimationColor(const Widget& widget, const Canvas& canvas) {
  const Theme& myTheme = widget.theme();
  Color color = widget.usesHighlighterColor()
                    ? myTheme.color.highlighterColor(canvas.bgcolor())
                    : myTheme.color.defaultColor(canvas.bgcolor());
  color.set_a(myTheme.pressAnimationOpacity(canvas.bgcolor()));
  return color;
}

static constexpr int64_t kMaxClickAnimRadius = 8192;

inline int64_t dsquare(XDim x0, YDim y0, XDim x1, YDim y1) {
  return (x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0);
}

inline int16_t animation_radius(const Rect& bounds, XDim x, XDim y,
                                float progress) {
  int64_t ul = dsquare(x, y, bounds.xMin(), bounds.yMin());
  int64_t ur = dsquare(x, y, bounds.xMax(), bounds.yMin());
  int64_t dl = dsquare(x, y, bounds.xMin(), bounds.yMax());
  int64_t dr = dsquare(x, y, bounds.xMax(), bounds.yMax());
  int64_t max = 0;
  if (ul > max) max = ul;
  if (ur > max) max = ur;
  if (dl > max) max = dl;
  if (dr > max) max = dr;
  int32_t result = (int32_t)(std::sqrt(max) * progress + 1);
  if (result > kMaxClickAnimRadius) {
    result = kMaxClickAnimRadius;
  }
  return (int16_t)result;
}

}  // namespace

void Widget::paintWidget(const Canvas& canvas, Clipper& clipper) {
  if (!isVisible()) {
    markCleanDescending();
    return;
  }
  Canvas my_canvas(canvas);
  my_canvas.shift(xOffset(), yOffset());
  my_canvas.clipToExtents(maxBounds());
  if (my_canvas.clip_box().empty()) {
    markCleanDescending();
    return;
  }
  if (!isDirty()) {
    // Fast path to only include exclusion rect.
    paintWidgetContents(my_canvas, clipper);
  }
  my_canvas.set_bgcolor(
      roo_display::alphaBlend(canvas.bgcolor(), background()));
  bool animation_continues = false;
  if (isEnabled()) {
    Color overlay = getOverlayColor(*this, canvas);
    // If click_animation is true, we need to redraw the overlay.
    bool click_animation = ((state_ & kWidgetClicking) != 0);
    if (click_animation) {
      float click_progress;
      int16_t click_x, click_y;
      // If click_animation_continues is true, we need to invalidate ourselves
      // after redrawing, so that we receive a subsequent paint request shortly.
      animation_continues = click_animation &&
                            getClickAnimation()->getProgress(
                                this, &click_progress, &click_x, &click_y) &&
                            click_progress < 1.0;
      if (animation_continues) {
        // Note that dx,dy might have changed since the click event, moving dim
        // out of the int16_t horizon.
        XDim cx = click_x + my_canvas.dx();
        YDim cy = click_y + my_canvas.dy();
        // See if the overlay would be visible.
        int16_t r =
            animation_radius(bounds(), click_x, click_y, click_progress);
        Rect rect(cx - r, cy - r, cx + r, cy + r);
        if (rect.intersects(my_canvas.clip_box())) {
          // Need to draw the circular overlay. Combine the regular rect overlay
          // into it.
          Color animation_color = getClickAnimationColor(*this, my_canvas);
          PressOverlay press_overlay(
              cx, cy, r, alphaBlend(overlay, animation_color), overlay);
          roo_display::ForegroundFilter filter(my_canvas.out(), &press_overlay);
          my_canvas.set_out(&filter);
          paintWidgetContents(my_canvas, clipper);
          // Do not clear dirtiness.
          invalidateInterior();
          return;
        }
      } else {
        // Full rect click overlay - just apply on top of the overlay as
        // calculated so far.
        state_ &= ~kWidgetClicking;
        Color animation_color = getClickAnimationColor(*this, my_canvas);
        overlay = alphaBlend(overlay, animation_color);
      }
    }
    if (overlay.a() > 0) {
      roo_display::OverlayFilter filter(canvas.out(), overlay,
                                        canvas.bgcolor());
      my_canvas.set_out(&filter);
      paintWidgetContents(my_canvas, clipper);
    } else {
      paintWidgetContents(my_canvas, clipper);
    }
    if (click_animation && !animation_continues) {
      // Make sure that the next paint is scheduled promptly, but that it does
      // not encounter click_animation, so that it just draws the widget in its
      // regular state (possibly pressed, but not click-animating anymore).
      invalidateInterior();
      return;
    }
  } else {
    roo_display::TranslucencyFilter disablement_filter(
        canvas.out(), theme().state.disabled, canvas.bgcolor());
    my_canvas.set_out(&disablement_filter);
    paintWidgetContents(my_canvas, clipper);
  }
}

void Widget::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  roo_display::Box absolute_bounds =
      bounds().translate(canvas.dx(), canvas.dy()).clip(canvas.clip_box());
  if (isDirty()) {
    clipper.setBounds(absolute_bounds);
    paint(canvas);
    markClean();
  }
  clipper.addExclusion(absolute_bounds);
}

Widget* Widget::dispatchTouchDownEvent(XDim x, YDim y) {
  return onTouchDown(x, y) ? this : nullptr;
}

bool Widget::onTouchDown(XDim x, YDim y) {
  return getApplication()->gesture_detector().onTouchDown(*this, x, y);
}

bool Widget::onTouchMove(XDim x, YDim y) {
  return getApplication()->gesture_detector().onTouchMove(*this, x, y);
}

bool Widget::onTouchUp(XDim x, YDim y) {
  return getApplication()->gesture_detector().onTouchUp(*this, x, y);
}

void Widget::onClicked() {
  if (on_clicked_ == nullptr) return;
  on_clicked_();
}

bool Widget::onDown(XDim x, YDim y) { return isClickable(); }

void Widget::onShowPress(XDim x, YDim y) {
  if (!isClickable()) return;
  if (isPressed()) return;
  ClickAnimation* anim = getClickAnimation();
  if (anim->isClickAnimating() || anim->isClickConfirmed()) return;
  if (showClickAnimation()) {
    anim->start(this, x, y);
    setClicking();
  }
  setPressed(true);
}

bool Widget::onSingleTapUp(XDim x, YDim y) {
  if (!isClickable()) return false;
  if (isPressed()) {
    setPressed(false);
  } else {
    // Quick release (onShowPress not yet triggered).
    if (showClickAnimation()) {
      ClickAnimation* anim = getClickAnimation();
      setClicking();
      markDirty();
      anim->start(this, x, y);
    }
  }
  getClickAnimation()->confirmClick(this);
  return true;
}

void Widget::onLongPress(XDim dx, YDim dy) {}

void Widget::onLongPressFinished(XDim dx, YDim dy) { setPressed(false); }

bool Widget::onScroll(XDim dx, YDim dy) {
  setPressed(false);
  return false;
}

bool Widget::onFling(XDim vx, YDim vy) {
  setPressed(false);
  return false;
}

void Widget::onCancel() {
  setPressed(false);
  if (kTerminateAnimationsOnCancel) {
    state_ &= ~kWidgetClicking;
    getClickAnimation()->cancel();
  }
}

std::ostream& operator<<(std::ostream& os, const Widget& widget) {
  os << GetTypeName(widget) << "{ " << &widget << "}";
  return os;
}

}  // namespace roo_windows
