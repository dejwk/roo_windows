#include "roo_windows/core/widget.h"

#include "roo_display/filter/foreground.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/core/rtti.h"
#include "roo_windows/decoration/decoration.h"

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
      on_interactive_change_(nullptr) {}

Widget::Widget(const Widget& w)
    : parent_(nullptr),
      parent_bounds_(0, 0, -1, -1),
      state_(kWidgetEnabled),
      redraw_status_(kDirty | kInvalidated),
      on_interactive_change_(nullptr) {}

MainWindow* Widget::getMainWindow() {
  return parent_ == nullptr ? nullptr : parent_->getMainWindow();
}

Application* Widget::getApplication() const {
  const MainWindow* w = getMainWindow();
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

const ClickAnimation* Widget::getClickAnimation() const {
  const MainWindow* w = getMainWindow();
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

Rect Widget::getParentBoundsOfShadow() const {
  return CalculateShadowExtents(parent_bounds(), getElevation());
}

Rect Widget::getSloppyTouchParentBounds() const {
  return slopify(parent_bounds());
}

Rect Widget::getSloppyTouchBounds() const { return slopify(bounds()); }

Color Widget::effectiveBackground() const {
  roo_display::Color bgcolor = background();
  return bgcolor.isOpaque() || parent() == nullptr
             ? bgcolor
             : roo_display::AlphaBlend(parent()->effectiveBackground(),
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
  if (getBorderStyle().corner_radius() > 0 && parent() != nullptr) {
    parent()->childInvalidatedRegion(this, maxParentBounds());
  }
  markDirty();
}

void Widget::invalidateInterior(const Rect& rect) {
  invalidateDescending(rect);
  if (getBorderStyle().corner_radius() > 0 && parent() != nullptr) {
    parent()->childInvalidatedRegion(this,
                                     rect.translate(xOffset(), yOffset()));
  }
  markDirty(rect);
}

void Widget::elevationChanged(int higherElevation) {
  if (higherElevation > 0 && parent_ != nullptr) {
    // TODO: we can avoid marking dirty if we introduce a new flag specifically
    // for elevation changed. This way we can avoid content redraws except for
    // the bounds.
    if (getBorderStyle().corner_radius() > 0) markDirty();
    parent()->childInvalidatedRegion(
        this, CalculateShadowExtents(parent_bounds(), higherElevation));
  }
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
    clearClicking();
    if (previous == VISIBLE && parent() != nullptr) parent()->childHidden(this);
  }
}

void Widget::setEnabled(bool enabled) {
  if (isEnabled() == enabled) return;
  uint8_t old_elevation = getElevation();
  state_ ^= kWidgetEnabled;
  if (!isVisible()) return;
  invalidateInterior();
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Widget::setSelected(bool selected) {
  if (selected == isSelected()) return;
  uint8_t old_elevation = getElevation();
  state_ ^= kWidgetSelected;
  if (!isVisible()) return;
  invalidateInterior();
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Widget::setActivated(bool activated) {
  if (activated == isActivated()) return;
  uint8_t old_elevation = getElevation();
  state_ ^= kWidgetActivated;
  if (!isVisible()) return;
  markDirty();
  if (useOverlayOnActivation()) {
    invalidateInterior();
  }
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Widget::setPressed(bool pressed) {
  if (pressed == isPressed()) return;
  uint8_t old_elevation = getElevation();
  state_ ^= kWidgetPressed;
  if (!isVisible()) return;
  invalidateInterior();
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Widget::setDragged(bool dragged) {
  if (dragged == isDragged()) return;
  uint8_t old_elevation = getElevation();
  state_ ^= kWidgetDragged;
  if (!isVisible()) return;
  invalidateInterior();
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
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
  uint8_t old_elevation = getElevation();
  state_ &= ~(kWidgetOn | kWidgetOff);
  if (state == Widget::ON) {
    state_ |= kWidgetOn;
  } else if (state == Widget::OFF) {
    state_ |= kWidgetOff;
  }
  markDirty();
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Widget::triggerInteractiveChange() {
  if (on_interactive_change_ == nullptr) return;
  on_interactive_change_();
}

void Widget::setClicking() {
  uint8_t old_elevation = getElevation();
  state_ |= kWidgetClicking;
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

void Widget::clearClicking() {
  uint8_t old_elevation = getElevation();
  state_ &= ~kWidgetClicking;
  uint8_t new_elevation = getElevation();
  if (old_elevation != new_elevation) {
    elevationChanged(std::max(old_elevation, new_elevation));
  }
}

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
  if (isClicking() && useOverlayOnPress()) {
    overlay_opacity += myTheme.pressedOpacity(bgcolor);
  } else if (isPressed() && useOverlayOnPress()) {
    overlay_opacity += myTheme.pressedOpacity(bgcolor);
  }
  if (isDragged()) overlay_opacity += myTheme.draggedOpacity(bgcolor);
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity == 0) return 0;  // roo_display::color::Transparent;
  return overlay_opacity;
}

Canvas Widget::prepareCanvas(const Canvas& in) {
  // NOTE: keeping this in a separate method sheds 16 bytes from the stack.
  Canvas canvas(in);
  canvas.shift(xOffset(), yOffset());
  canvas.clipToExtents(maxBounds());
  {
    Color bg = background();
    if (!isEnabled()) {
      bg.set_a(bg.a() / 2);
    }
    canvas.set_bgcolor(roo_display::AlphaBlend(canvas.bgcolor(), bg));
  }
  return canvas;
}

void Widget::paintWidget(const Canvas& canvas, Clipper& clipper) {
  if (!isVisible()) {
    markCleanDescending();
    return;
  }
  Canvas my_canvas = prepareCanvas(canvas);
  if (my_canvas.clip_box().empty()) {
    markCleanDescending();
    return;
  }
  OverlaySpec overlay_spec(*this, my_canvas);
  if (!overlay_spec.is_modded()) {
    paintWidgetContents(my_canvas, clipper);
  } else {
    paintWidgetModded(my_canvas, overlay_spec, clipper);
  }

  my_canvas.set_clip_box(canvas.clip_box());
  finalizePaintWidget(my_canvas, clipper, overlay_spec);
}

void Widget::paintWidgetModded(Canvas& canvas, const OverlaySpec& overlay_spec,
                               Clipper& clipper) {
  // Keeping this in a separate methods sheds 32 bytes from the stack.
  if (overlay_spec.is_disabled()) {
    roo_display::DisplayOutput& out = canvas.out();
    roo_display::TranslucencyFilter disablement_filter(
        canvas.out(), theme().state.disabled, canvas.bgcolor());
    canvas.set_out(&disablement_filter);
    paintWidgetContents(canvas, clipper);
    canvas.set_out(&out);
  } else {
    // If click_animation is true, we need to redraw the overlay.
    bool click_animation = ((state_ & kWidgetClicking) != 0);
    if (click_animation) {
      // If click_animation_continues is true, we need to invalidate ourselves
      // after redrawing, so that we receive a subsequent paint request
      // shortly.
      if (overlay_spec.is_click_animation_in_progress()) {
        // Need to draw the circular overlay.
        roo_display::DisplayOutput& out = canvas.out();
        roo_display::ForegroundFilter filter(canvas.out(),
                                             overlay_spec.press_overlay());
        canvas.set_out(&filter);
        paintWidgetContents(canvas, clipper);
        canvas.set_out(&out);
      } else {
        clearClicking();
      }
    }
    if (overlay_spec.is_click_animation_in_progress()) {
      // Already drawn.
    } else if (overlay_spec.base_overlay().a() > 0) {
      roo_display::DisplayOutput& out = canvas.out();
      switch (getOverlayType()) {
        case OVERLAY_POINT: {
          roo_display::FpPoint focus = getPointOverlayFocus();
          XDim dx;
          YDim dy;
          getAbsoluteOffset(dx, dy);
          auto circle = roo_display::SmoothFilledCircle(
              {dx + focus.x, dy + focus.y}, kPointOverlayDiameter * 0.5f - 0.5f,
              overlay_spec.base_overlay());
          roo_display::ForegroundFilter filter(out, &circle);
          canvas.set_out(&filter);
          paintWidgetContents(canvas, clipper);
          canvas.set_out(&out);
        }
        default: {
          roo_display::OverlayFilter filter(canvas.out(),
                                            overlay_spec.base_overlay(),
                                            roo_display::color::Transparent);
          canvas.set_out(&filter);
          paintWidgetContents(canvas, clipper);
          canvas.set_out(&out);
        }
      }
    } else {
      paintWidgetContents(canvas, clipper);
    }
  }
}

void Widget::paintWidgetContents(const Canvas& canvas, Clipper& clipper) {
  if (!isDirty()) return;
  Canvas my_canvas = prepareContentsCanvas(canvas);
  clipper.setBounds(my_canvas.clip_box());
  paint(my_canvas);
  markClean();
}

Canvas Widget::prepareContentsCanvas(const Canvas& in) {
  Canvas canvas = in;
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  if (border_thickness > 0) {
    canvas.clip(roo_display::Box(
        border_thickness + canvas.dx(), border_thickness + canvas.dy(),
        width() - border_thickness - 1 + canvas.dx(),
        height() - border_thickness - 1 + canvas.dy()));
  }
  return canvas;
}

void Widget::finalizePaintWidget(const Canvas& canvas, Clipper& clipper,
                                 const OverlaySpec& overlay_spec) const {
  BorderStyle border_style = getBorderStyle().trim(width(), height());
  uint8_t border_thickness = border_style.getThickness();
  uint8_t elevation = getElevation();
  if (elevation != 0 || border_thickness != 0) {
    roo_display::Box absolute_bounds(canvas.dx(), canvas.dy(),
                                     width() - 1 + canvas.dx(),
                                     height() - 1 + canvas.dy());
    clipper.addDecoration(canvas.clip_box(), absolute_bounds, elevation,
                          overlay_spec, canvas.bgcolor(),
                          border_style.corner_radius(),
                          border_style.outline_width(),
                          AlphaBlend(canvas.bgcolor(), getOutlineColor()));
  }
  roo_display::Box inner_bounds(border_thickness + canvas.dx(),
                                border_thickness + canvas.dy(),
                                width() - border_thickness - 1 + canvas.dx(),
                                height() - border_thickness - 1 + canvas.dy());
  clipper.addExclusion(
      roo_display::Box::Intersect(inner_bounds, canvas.clip_box()));
}

Widget* Widget::dispatchTouchDownEvent(XDim x, YDim y) {
  return bounds().contains(x, y) && onTouchDown(x, y) ? this : nullptr;
}

bool Widget::isHandlingGesture() const {
  const Application* app = getApplication();
  return app == nullptr
             ? false
             : app->gesture_detector().currentGestureTarget() == this;
}

bool Widget::onTouchDown(XDim x, YDim y) {
  return getApplication()->gesture_detector().onTouchDown(*this, x, y);
}

bool Widget::onTouchMove(XDim x, YDim y) {
  return getApplication()->gesture_detector().onTouchMove(*this, x, y);
}

bool Widget::onTouchUp(XDim x, YDim y) {
  bool handled = getApplication()->gesture_detector().onTouchUp(*this, x, y);
  if (isPressed()) {
    setPressed(false);
  }
  return handled;
}

void Widget::onClicked() { triggerInteractiveChange(); }

bool Widget::isClickable() const { return on_interactive_change_ != nullptr; }

bool Widget::onDown(XDim x, YDim y) {
  if (!isClickable()) return false;
  const ClickAnimation* anim = getClickAnimation();
  return !anim->isClickAnimating() && !anim->isClickConfirmed();
}

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

bool Widget::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  setPressed(false);
  return false;
}

bool Widget::onFling(XDim x, YDim y, XDim vx, YDim vy) {
  setPressed(false);
  return false;
}

void Widget::onCancel() {
  setPressed(false);
  if (kTerminateAnimationsOnCancel) {
    clearClicking();
    getClickAnimation()->cancel();
  }
}

std::ostream& operator<<(std::ostream& os, const Widget& widget) {
  os << GetTypeName(widget) << "{ " << &widget << "}";
  return os;
}

}  // namespace roo_windows
