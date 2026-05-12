#include "roo_windows/core/widget.h"

#include "roo_display/filter/foreground.h"
#include "roo_display/shape/smooth.h"
#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/core/rtti.h"

#ifndef MLOG_roo_windows_layout
#define MLOG_roo_windows_layout 0
#endif

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
  dx += offsetLeft();
  dy += offsetTop();
}

namespace {

Rect Slopify(Rect bounds) {
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

XDim Widget::offsetRight() const {
  return parent()->width() - parent_bounds().xMax() - 1;
}

YDim Widget::offsetBottom() const {
  return parent()->height() - parent_bounds().yMax() - 1;
}

Rect Widget::getParentDecorationBounds() const { return parent_bounds(); }

Insets Widget::getInteractionInsets() const {
  if (getOverlayType() != OVERLAY_POINT) return Insets::Zero();

  const Rect logical_bounds = bounds();
  roo_display::FpPoint focus = getPointOverlayFocus();
  Rect overlay_bounds(focus.x - kPointOverlayDiameter * 0.5f,
                      focus.y - kPointOverlayDiameter * 0.5f,
                      focus.x + kPointOverlayDiameter * 0.5f,
                      focus.y + kPointOverlayDiameter * 0.5f);
  return Insets(
      std::min<int16_t>(0, overlay_bounds.xMin() - logical_bounds.xMin()),
      std::min<int16_t>(0, overlay_bounds.yMin() - logical_bounds.yMin()),
      std::min<int16_t>(0, logical_bounds.xMax() - overlay_bounds.xMax()),
      std::min<int16_t>(0, logical_bounds.yMax() - overlay_bounds.yMax()));
}

namespace {

roo_display::SmoothShape MakePointOverlay(const Widget& widget,
                                          const Canvas& canvas,
                                          roo_display::Color color) {
  roo_display::FpPoint focus = widget.getPointOverlayFocus();
  return roo_display::SmoothFilledCircle(
      {canvas.dx() + focus.x, canvas.dy() + focus.y},
      kPointOverlayDiameter * 0.5f - 0.5f, color);
}

bool MayHaveNonTransparentOverlayOutsideBounds(const Widget& widget) {
  if (widget.getOverlayType() != Widget::OVERLAY_POINT) return false;
  if (widget.isHover()) return true;
  if (widget.isFocused()) return true;
  if (widget.isSelected()) return true;
  if (widget.isDragged()) return true;
  if (widget.isActivated() && widget.useOverlayOnActivation()) return true;
  if ((widget.isPressed() || widget.isClicking()) &&
      widget.useOverlayOnPress()) {
    return true;
  }
  return false;
}

}  // namespace

bool Widget::hasTransientPaintOverflow() const {
  return getParentTransientPaintBounds() != parent_bounds();
}

Rect Widget::getParentTransientPaintBounds() const {
  if (parent() == nullptr ||
      !MayHaveNonTransparentOverlayOutsideBounds(*this)) {
    return parent_bounds();
  }
  return getParentInteractionBounds();
}

Rect Widget::getContentBounds() const {
  return getInkInsets().applyTo(bounds());
}

namespace {

Rect UnionRects(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

}  // namespace

Rect Widget::getVisualBounds() const {
  return UnionRects(UnionRects(getContentBounds(), getDecorationBounds()),
                    getTransientPaintBounds());
}

Rect Widget::getParentVisualBounds() const {
  return UnionRects(
      UnionRects(getParentContentBounds(), getParentDecorationBounds()),
      getParentTransientPaintBounds());
}

Rect Widget::getSloppyTouchParentBounds() const {
  return Slopify(parent_bounds());
}

Rect Widget::getSloppyTouchBounds() const { return Slopify(bounds()); }

ColorRole Widget::effectiveContainerRole() const {
  return parent() != nullptr ? parent()->effectiveContainerRole()
                             : ColorRole::kBackground;
}

// Returns the theme used by this widget's app. Must not be called outside of
// the paint() flow.
const Theme& Widget::theme() const {
  CHECK(parent_ != nullptr)
      << "Widget::theme() should only be called from paint()";
  return parent_->theme();
}

void Widget::setDirty(const Rect& bounds) {
  redraw_status_ |= kDirty;
  if (parent_ != nullptr) {
    parent_->propagateDirty(this, bounds.translate(offsetLeft(), offsetTop()));
  }
}

void Widget::invalidateInterior() {
  invalidateDescending();
  setDirty();
}

void Widget::invalidateInterior(const Rect& rect) {
  invalidateDescending(rect);
  setDirty(rect);
}

void Widget::requestLayout() {
  if (isLayoutRequested()) return;
  onRequestLayout();
}

#if MLOG_IS_ON(roo_windows_layout)
static int indent_level = 0;

const char* Indent() {
  static const char* spaces = "                                        ";
  int offset = strlen(spaces) - indent_level;
  if (offset < 0) offset = 0;
  return spaces + offset;
}
#endif

Dimensions Widget::measure(WidthSpec width, HeightSpec height) {
#if MLOG_IS_ON(roo_windows_layout)
  MLOG(roo_windows_layout) << Indent() << "Measuring " << *this << " (" << width
                           << ", " << height << ")";
  ++indent_level;
#endif

  Dimensions result = onMeasure(width, height);
#if MLOG_IS_ON(roo_windows_layout)
  --indent_level;
  MLOG(roo_windows_layout) << Indent() << "Measuring " << *this << " returned "
                           << result;
#endif

  redraw_status_ |= kLayoutRequired;
  return result;
}

void Widget::layout(const Rect& rect) {
  bool changed = (rect != parent_bounds());
  if (changed || isLayoutRequired()) {
#if MLOG_IS_ON(roo_windows_layout)
    XDim dx = 0;
    YDim dy = 0;
    if (parent() != nullptr) {
      parent()->getAbsoluteOffset(dx, dy);
    }
    MLOG(roo_windows_layout) << "Layout changed for " << *this << ": " << rect
                             << "; absolute: " << rect.translate(dx, dy);
#endif
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
static XDim GetDefaultWidth(XDim width, WidthSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return width;
    case AT_MOST:
      return width < spec.value() ? width : spec.value();
    default:
      return spec.value();
  }
}

// Utility to return a default height. Uses the supplied height if the
// HeightSpec imposed no constraints. Will get larger if allowed by the
// HeightSpec.
static YDim GetDefaultHeight(YDim height, HeightSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return height;
    case AT_MOST:
      return height < spec.value() ? height : spec.value();
    default:
      return spec.value();
  }
}

}  // namespace

Dimensions Widget::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions suggestedMin = getSuggestedMinimumDimensions();
  return Dimensions(GetDefaultWidth(suggestedMin.width(), width),
                    GetDefaultHeight(suggestedMin.height(), height));
}

void Widget::setParentClipMode(ParentClipMode mode) {
  if (mode == getParentClipMode()) return;
  bool visible = isVisible();
  if (visible) {
    setVisibility(Visibility::kInvisible);
  }
  state_ ^= kWidgetClippedInParent;
  if (visible) {
    setVisibility(Visibility::kVisible);
  }
}

void Widget::setVisibility(Visibility visibility) {
  Visibility previous = this->visibility();
  if (visibility == previous) return;
  state_ &= ~(kWidgetHidden | kWidgetGone);
  state_ |= (kWidgetHidden * (visibility == Visibility::kInvisible));
  state_ |= (kWidgetGone * (visibility == Visibility::kGone));
  if (previous == Visibility::kGone) {
    // Layout might have been skipped before, leaving the request pending.
    if (parent() != nullptr && isLayoutRequested()) parent()->requestLayout();
    requestLayout();
  } else if (visibility == Visibility::kGone) {
    if (parent() != nullptr) parent()->requestLayout();
  }
  if (visibility == Visibility::kVisible) {
    invalidateInterior();
    if (previous != Visibility::kVisible && parent() != nullptr)
      parent()->childShown(this);
  } else {
    setPressed(false);
    clearClicking();
    if (previous == Visibility::kVisible && parent() != nullptr)
      parent()->childHidden(this);
  }
}

void Widget::setEnabled(bool enabled) {
  if (isEnabled() == enabled) return;
  state_ ^= kWidgetEnabled;
  if (isVisible()) {
    invalidateInterior();
  }
  notifyStateChanged(kWidgetEnabled);
}

void Widget::setSelected(bool selected) {
  if (selected == isSelected()) return;
  Rect old_bounds = maxParentBounds();
  state_ ^= kWidgetSelected;
  if (isVisible()) {
    invalidateInterior();
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  notifyStateChanged(kWidgetSelected);
}

void Widget::setActivated(bool activated) {
  if (activated == isActivated()) return;
  Rect old_bounds = maxParentBounds();
  state_ ^= kWidgetActivated;
  if (isVisible()) {
    setDirty();
    if (useOverlayOnActivation()) {
      invalidateInterior();
    }
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  notifyStateChanged(kWidgetActivated);
}

void Widget::setPressed(bool pressed) {
  if (pressed == isPressed()) return;
  Rect old_bounds = maxParentBounds();
  state_ ^= kWidgetPressed;
  if (isVisible()) {
    setDirty();
    if (getOverlayType() != OVERLAY_NONE) {
      invalidateInterior();
      notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
    }
  }
  notifyStateChanged(kWidgetPressed);
}

void Widget::setDragged(bool dragged) {
  if (dragged == isDragged()) return;
  Rect old_bounds = maxParentBounds();
  state_ ^= kWidgetDragged;
  if (isVisible()) {
    invalidateInterior();
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  notifyStateChanged(kWidgetDragged);
}

OnOffState Widget::onOffState() const {
  return isOn() ? (!isOff() ? OnOffState::kOn : OnOffState::kIndeterminate)
                : (isOff() ? OnOffState::kOff : OnOffState::kIndeterminate);
}

void Widget::toggle() {
  OnOffState state = onOffState();
  if (state == OnOffState::kOn) {
    setOff();
  } else if (state == OnOffState::kOff) {
    setOn();
  }
}

void Widget::setOnOffState(OnOffState state) {
  if (onOffState() == state) return;
  uint16_t old_state = state_;
  state_ &= ~(kWidgetOn | kWidgetOff);
  if (state == OnOffState::kOn) {
    state_ |= kWidgetOn;
  } else if (state == OnOffState::kOff) {
    state_ |= kWidgetOff;
  }
  setDirty();
  notifyStateChanged(old_state ^ state_);
}

void Widget::triggerInteractiveChange() {
  if (on_interactive_change_ == nullptr) return;
  on_interactive_change_();
}

void Widget::notifyParentInvalidatedRegion(const Rect& rect) {
  if (parent() == nullptr) return;
  parent()->childInvalidatedRegion(this, rect);
}

void Widget::setClicking() {
  if (isClicking()) return;
  Rect old_bounds = maxParentBounds();
  state_ |= kWidgetClicking;
  if (isVisible()) {
    invalidateInterior();
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  notifyStateChanged(kWidgetClicking);
}

void Widget::clearClicking() {
  if (!isClicking()) return;
  Rect old_bounds = maxParentBounds();
  state_ &= ~kWidgetClicking;
  if (isVisible()) {
    invalidateInterior();
    notifyParentInvalidatedRegion(Rect::Extent(old_bounds, maxParentBounds()));
  }
  notifyStateChanged(kWidgetClicking);
}

void Widget::setParent(Container* parent, bool owned) {
  CHECK(parent_ == nullptr || parent == nullptr || parent == parent_)
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

ColorRole Widget::effectiveOverlayColorRole() const {
  if (getOverlayType() == Widget::OVERLAY_POINT && parent() != nullptr) {
    return parent()->effectiveContainerRole();
  }
  return effectiveContainerRole();
}

uint8_t Widget::getOverlayOpacity() const {
  if (getOverlayType() == OVERLAY_NONE) return 0;
  ColorRole bg_role = effectiveOverlayColorRole();
  uint16_t overlay_opacity = 0;
  const Theme& myTheme = theme();
  if (isHover()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kHover);
  }
  if (isFocused()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kFocus);
  }
  if (isSelected()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kSelected);
  }
  if (isActivated() && useOverlayOnActivation()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kActivated);
  }
  if (isClicking() && useOverlayOnPress()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kPressed);
  } else if (isPressed() && useOverlayOnPress()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kPressed);
  }
  if (isDragged()) {
    overlay_opacity += myTheme.opacity(bg_role, InteractionState::kDragged);
  }
  if (overlay_opacity > 255) overlay_opacity = 255;
  if (overlay_opacity == 0) return 0;  // roo_display::color::Transparent;
  return overlay_opacity;
}

Canvas Widget::prepareCanvas(const Canvas& in) {
  // NOTE: keeping this in a separate method sheds 16 bytes from the stack.
  Canvas canvas(in);
  canvas.shift(offsetLeft(), offsetTop());
  canvas.clipToExtents(maxBounds());
  return canvas;
}

void Widget::paintWidget(const Canvas& canvas, Clipper& clipper) {
  if (!isVisible()) {
    markCleanDescending();
    return;
  }
  Canvas my_canvas = prepareCanvas(canvas);
  bool empty = my_canvas.clip_box().empty();
  if (empty) {
    // Nothing remains inside logical bounds. If persistent decoration paint
    // extends farther, finalizePaintWidget() may still need to repaint or
    // exclude that decoration overflow region (currently surface shadow).
    // Transient overflow is modeled separately and is not consumed here yet.
    markCleanDescending();
    if (!hasDecorationOverflow()) return;
  }
  OverlaySpec overlay_spec(*this, my_canvas);
  if (!empty) {
    if (!overlay_spec.is_modded()) {
      paintWidgetContents(my_canvas, clipper);
    } else {
      paintWidgetModded(my_canvas, overlay_spec, clipper);
    }
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
        Rect repaint_bounds = maxParentBounds();
        if (getOverlayType() == OVERLAY_POINT) {
          clipper.addOverlay(overlay_spec.press_overlay(), canvas.clip_box());
        } else {
          roo_display::DisplayOutput& out = canvas.out();
          roo_display::ForegroundFilter filter(canvas.out(),
                                               overlay_spec.press_overlay());
          canvas.set_out(&filter);
          paintWidgetContents(canvas, clipper);
          canvas.set_out(&out);
          setDirty();
          notifyParentInvalidatedRegion(repaint_bounds);
          return;
        }
        paintWidgetContents(canvas, clipper);
        setDirty();
        notifyParentInvalidatedRegion(repaint_bounds);
      } else {
        clearClicking();
        // overlay_spec was computed before clearClicking(), so this frame may
        // still draw the full settled overlay even though the widget is no
        // longer marked clicking. ClickAnimation::tick() compensates by
        // invalidating the full interaction spill region before delivering the
        // deferred click, which refreshes any siblings we may have tinted.
      }
    }
    if (overlay_spec.is_click_animation_in_progress()) {
      // Already drawn.
    } else if (overlay_spec.base_overlay().a() > 0) {
      switch (getOverlayType()) {
        case OVERLAY_POINT: {
          clipper.addOverlayShape(
              MakePointOverlay(*this, canvas, overlay_spec.base_overlay()),
              canvas.clip_box());
          paintWidgetContents(canvas, clipper);
          break;
        }
        default: {
          roo_display::DisplayOutput& out = canvas.out();
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
  if (!isDirty() || clipper.isDeadlineExceeded()) return;
  Canvas my_canvas = prepareContentsCanvas(canvas);
  clipper.setBounds(my_canvas.clip_box());
  paint(my_canvas);
  markClean();
}

Canvas Widget::prepareContentsCanvas(const Canvas& in) {
  Canvas canvas = in;
  canvas.clipToExtents(getContentBounds());
  return canvas;
}

void Widget::finalizePaintWidget(const Canvas& canvas, Clipper& clipper,
                                 const OverlaySpec& overlay_spec) const {
  (void)overlay_spec;
  Rect exclusion = getDirectPaintExclusionBounds();
  roo_display::Box absolute_bounds(
      canvas.dx() + exclusion.xMin(), canvas.dy() + exclusion.yMin(),
      canvas.dx() + exclusion.xMax(), canvas.dy() + exclusion.yMax());
  clipper.addExclusion(
      roo_display::Box::Intersect(absolute_bounds, canvas.clip_box()));
}

Rect Widget::getDirectPaintExclusionBounds() const {
  return getContentBounds();
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

bool Widget::isClickable() const { return false; }

bool Widget::onDown(XDim x, YDim y) {
  if (!isClickable() || !isEnabled()) return false;
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
      setDirty();
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

roo_logging::Stream& operator<<(roo_logging::Stream& os, const Widget& widget) {
  os << GetTypeName(widget) << "{" << &widget << "}";
  return os;
}

}  // namespace roo_windows
