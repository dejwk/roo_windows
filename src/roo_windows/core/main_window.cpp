#include "main_window.h"

#include <Arduino.h>

#include "roo_display/color/color.h"
#include "roo_display/color/color_set.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

namespace {

void maybeAddColor(roo_display::internal::ColorSet& palette, Color color) {
  if (palette.size() >= 15) return;
  palette.insert(color);
}

Rect UnionNonEmpty(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

}  // namespace

MainWindow::MainWindow(Application& app, const roo_display::Box& bounds)
    : Container(app.context()),
      app_(app),
      redraw_bounds_(bounds),
      scrim_(app.context()) {
  parent_bounds_ = Rect(bounds);
  invalidateDescending();
  const ApplicationContext& context = app.context();
  roo_display::internal::ColorSet color_set;
  const FrameworkTheme& framework = context.theme().framework;
  maybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kCanvas));
  maybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kSurface));
  maybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kEmphasis));
  maybeAddColor(color_set, context.keyboardColorTheme().background);
  {
    Color c = framework.interaction.resolve(FrameworkColorRole::kSurface,
                                            InteractionState::kPressed);
    c = AlphaBlend(framework.color.resolve(FrameworkColorRole::kSurface), c);
    maybeAddColor(color_set, c);
  }
  {
    Color c = framework.color.resolve(FrameworkColorRole::kEmphasis);
    c.set_a(framework.interaction.disabledContentOpacity);
    c = AlphaBlend(framework.color.resolve(FrameworkColorRole::kSurface), c);
    maybeAddColor(color_set, c);
  }

  maybeAddColor(color_set, context.keyboardColorTheme().normalButton);
  maybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kCritical));
  maybeAddColor(color_set, context.keyboardColorTheme().modifierButton);
  Color palette[color_set.size()];
  std::copy(color_set.begin(), color_set.end(), palette);
  // background_fill_buffer_.setPalette(palette, color_set.size());
  // background_fill_buffer_.setPrefilled(
  //     framework.color.resolve(FrameworkColorRole::kCanvas));
}

MainWindow::~MainWindow() {
  cancelPaintContinuation();
  while (active_pins_ != nullptr) {
    active_pins_ = std::move(active_pins_->next_);
  }
  while (!popups_.empty()) {
    removeLastFromLayer(popups_);
  }
  while (!tasks_.empty()) {
    removeLastFromLayer(tasks_);
  }
}

void MainWindow::cancelPaintContinuation() {
  paint_continuation_ = false;
  continuation_invalid_bounds_ = Rect(0, 0, -1, -1);
}

Application& MainWindow::app() const { return app_; }
const Theme& MainWindow::theme() const { return app().context().theme(); }

void MainWindow::addToLayer(std::vector<Widget*>& layer, WidgetRef child,
                            const Rect& rect) {
  Widget* widget = child.get();
  layer.push_back(widget);
  attachChild(std::move(child), rect);
}

void MainWindow::addTask(WidgetRef child, const Rect& rect) {
  addToLayer(tasks_, std::move(child), rect);
}

void MainWindow::addPopup(WidgetRef child, const Rect& rect) {
  addToLayer(popups_, std::move(child), rect);
}

void MainWindow::removeTask(Widget& child) { removeFromLayer(tasks_, child); }

void MainWindow::removePopup(Widget& child) { removeFromLayer(popups_, child); }

void MainWindow::refreshClickAnimation() { click_animation_.tick(); }

void MainWindow::updateLayout() {
  if (isLayoutRequested()) {
    measure(WidthSpec::Exactly(width()), HeightSpec::Exactly(height()));
    layout(bounds());
  }
}

bool MainWindow::paintWindow(const roo_display::Surface& s,
                             roo_time::Uptime deadline) {
  preparePresentationPinsForPaint();
  if (paint_continuation_ && !continuation_invalid_bounds_.empty()) {
    // Preserve the completed prefix outside the newly invalidated area. Inside
    // it, remove stale exclusions/overlays and reopen just the affected widget
    // subtrees for this continuation.
    clipper_state_.invalidate(continuation_invalid_bounds_.asBox());
    invalidateDescending(continuation_invalid_bounds_);
    continuation_invalid_bounds_ = Rect(0, 0, -1, -1);
  }
  if (!initialized_) {
    initialized_ = true;
    s.drawObject(roo_display::Fill(
        theme().framework.color.resolve(FrameworkColorRole::kCanvas)));
  }
  if (pending_scrim_blit_) {
    pending_scrim_blit_ = false;
    if (s.out().getCapabilities().supportsBlending()) {
      s.out().fillRect(roo_display::BlendingMode::kSourceOverOpaque,
                       bounds().asBox(), scrim_.color());
    } else {
      invalidateBeneath(bounds(), &scrim_, /*clip=*/true);
    }
  }
  if (!isDirty()) {
    paint_continuation_ = false;
    continuation_invalid_bounds_ = Rect(0, 0, -1, -1);
    return true;
  }
  Canvas canvas(&s);
  canvas.clipToExtents(redraw_bounds_);
  Rect old_redraw_bounds = redraw_bounds_;
  redraw_bounds_ = Rect(0, 0, -1, -1);
  Clipper clipper(clipper_state_, s.out(), deadline, paint_continuation_);
  canvas.set_out(clipper.out());
  paintWidget(canvas, clipper);
  if (clipper.isDeadlineExceeded()) {
    // Keep exclusions and clipper-owned overlays from the completed foreground
    // prefix. Clean children remain skipped on retry while their preserved
    // exclusions protect them from the remaining lower-z paint.
    paint_continuation_ = true;
    redraw_bounds_ = UnionNonEmpty(old_redraw_bounds, redraw_bounds_);
    return false;
  }
  paint_continuation_ = false;
  continuation_invalid_bounds_ = Rect(0, 0, -1, -1);
  commitPresentationPinBounds();
  return true;
}

namespace {

bool IsEffectivelyVisible(const Widget& widget) {
  for (const Widget* current = &widget; current != nullptr;
       current = current->parent()) {
    if (!current->isVisible()) return false;
  }
  return true;
}

bool IsInSubtree(const Widget& candidate, const Widget& subtree) {
  for (const Widget* current = &candidate; current != nullptr;
       current = current->parent()) {
    if (current == &subtree) return true;
  }
  return false;
}

}  // namespace

PresentationPinShowResult MainWindow::showPresentationPin(
    Widget& anchor, std::unique_ptr<PresentationPin> pin) {
  if (pin == nullptr) return PresentationPinShowResult::kAllocationFailed;
  if (&anchor == this || anchor.parent() == nullptr ||
      anchor.getMainWindow() != this) {
    return PresentationPinShowResult::kAnchorUnavailable;
  }
  for (PresentationPin* current = active_pins_.get(); current != nullptr;
       current = current->next_.get()) {
    if (current->anchor_ == &anchor) {
      return PresentationPinShowResult::kAlreadyRegistered;
    }
  }

  Widget* root = &anchor;
  while (root->parent() != this) {
    root = root->parent();
    if (root == nullptr) return PresentationPinShowResult::kAnchorUnavailable;
  }
  pin->anchor_ = &anchor;
  pin->z_scope_root_ = root;
  pin->next_ = std::move(active_pins_);
  active_pins_ = std::move(pin);
  if (IsEffectivelyVisible(anchor)) {
    PresentationPin& shown = *active_pins_;
    invalidatePresentationRegion(Rect::Intersect(
        Rect::Intersect(shown.boundsInWindow(), shown.clipBoundsInWindow()),
        bounds()));
  }
  return PresentationPinShowResult::kShown;
}

bool MainWindow::hasPresentationPin(const Widget& anchor) const {
  for (const PresentationPin* current = active_pins_.get(); current != nullptr;
       current = current->next_.get()) {
    if (current->anchor_ == &anchor) return true;
  }
  return false;
}

void MainWindow::invalidatePresentationRegion(const Rect& rect) {
  const Rect clipped = Rect::Intersect(rect, bounds());
  if (clipped.empty()) return;
  setDirty(clipped);
  invalidateDescending(clipped);
  redraw_bounds_ = UnionNonEmpty(redraw_bounds_, clipped);
}

void MainWindow::setPresentationPinDirty(const Widget& anchor) {
  for (PresentationPin* current = active_pins_.get(); current != nullptr;
       current = current->next_.get()) {
    if (current->anchor_ != &anchor || !IsEffectivelyVisible(anchor)) continue;
    invalidatePresentationRegion(Rect::Intersect(
        current->dirtyBoundsInWindow(), current->clipBoundsInWindow()));
    return;
  }
}

void MainWindow::hidePresentationPin(const Widget& anchor) {
  std::unique_ptr<PresentationPin>* link = &active_pins_;
  while (*link != nullptr) {
    if ((*link)->anchor_ == &anchor) {
      invalidatePresentationRegion((*link)->presented_bounds_);
      *link = std::move((*link)->next_);
      return;
    }
    link = &((*link)->next_);
  }
}

void MainWindow::presentationAnchorSubtreeDetaching(Widget& subtree) {
  std::unique_ptr<PresentationPin>* link = &active_pins_;
  while (*link != nullptr) {
    if (IsInSubtree(*(*link)->anchor_, subtree)) {
      invalidatePresentationRegion((*link)->presented_bounds_);
      *link = std::move((*link)->next_);
    } else {
      link = &((*link)->next_);
    }
  }
}

void MainWindow::preparePresentationPinsForPaint() {
  for (PresentationPin* current = active_pins_.get(); current != nullptr;
       current = current->next_.get()) {
    Rect current_bounds(0, 0, -1, -1);
    if (IsEffectivelyVisible(*current->anchor_)) {
      current_bounds =
          Rect::Intersect(Rect::Intersect(current->boundsInWindow(),
                                          current->clipBoundsInWindow()),
                          bounds());
    }
    if (current_bounds != current->presented_bounds_) {
      invalidatePresentationRegion(
          UnionNonEmpty(current->presented_bounds_, current_bounds));
    }
  }
}

void MainWindow::paintPinsBeforeScopeRoot(Widget& root, PaintContext& ctx) {
  for (PresentationPin* current = active_pins_.get(); current != nullptr;
       current = current->next_.get()) {
    if (current->z_scope_root_ != &root ||
        !IsEffectivelyVisible(*current->anchor_)) {
      continue;
    }
    Rect pin_bounds =
        Rect::Intersect(Rect::Intersect(current->boundsInWindow(),
                                        current->clipBoundsInWindow()),
                        bounds());
    if (pin_bounds.empty()) continue;
    current->presented_bounds_ =
        UnionNonEmpty(current->presented_bounds_, pin_bounds);
    PaintContext pin_ctx = ctx.clipped(pin_bounds);
    if (!pin_ctx.empty()) current->paint(pin_ctx);
  }
}

void MainWindow::commitPresentationPinBounds() {
  for (PresentationPin* current = active_pins_.get(); current != nullptr;
       current = current->next_.get()) {
    current->presented_bounds_ =
        IsEffectivelyVisible(*current->anchor_)
            ? Rect::Intersect(Rect::Intersect(current->boundsInWindow(),
                                              current->clipBoundsInWindow()),
                              bounds())
            : Rect(0, 0, -1, -1);
  }
}

void MainWindow::paintChildren(PaintContext& ctx) {
  PaintContext clipped_ctx = ctx.clipped(bounds());
  bool fast_render = isDirty() && respectsChildrenBoundaries();
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    if (ctx.isDeadlineExceeded()) return;
    Widget& child = getChild(i);
    paintPinsBeforeScopeRoot(child, ctx);
    if (child.getParentClipMode() == ParentClipMode::kClipped) {
      child.paintWidget(clipped_ctx.canvas(),
                        clipped_ctx.clipperForFramework());
      if (fast_render) fastDrawChildShadow(child, clipped_ctx);
    } else {
      child.paintWidget(ctx.canvas(), ctx.clipperForFramework());
    }
  }
}

void MainWindow::propagateDirty(const Widget* child, const Rect& rect) {
  Rect clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = Rect::Intersect(rect, bounds());
  }
  setDirty(clipped);
  if (!clipped.empty()) {
    if (paint_continuation_) {
      continuation_invalid_bounds_ =
          continuation_invalid_bounds_.empty()
              ? clipped
              : Rect::Extent(continuation_invalid_bounds_, clipped);
    }
    if (redraw_bounds_.empty()) {
      redraw_bounds_ = clipped;
    } else {
      redraw_bounds_ = Rect::Extent(redraw_bounds_, clipped);
    }
  }
}

void MainWindow::childInvalidatedRegion(const Widget* child, Rect rect) {
  rect = Rect::Intersect(rect, bounds());
  if (!rect.empty()) {
    if (paint_continuation_) {
      continuation_invalid_bounds_ =
          continuation_invalid_bounds_.empty()
              ? rect
              : Rect::Extent(continuation_invalid_bounds_, rect);
    }
    if (redraw_bounds_.empty()) {
      redraw_bounds_ = rect;
    } else {
      redraw_bounds_ = Rect::Extent(redraw_bounds_, rect);
    }
  }
  Container::childInvalidatedRegion(child, rect);
}

void MainWindow::removeLastFromLayer(std::vector<Widget*>& layer) {
  Widget* widget = layer.back();
  layer.pop_back();
  detachChild(widget);
}

void MainWindow::removeFromLayer(std::vector<Widget*>& layer, Widget& child) {
  for (auto it = layer.begin(); it != layer.end(); ++it) {
    if (*it != &child) continue;
    layer.erase(it);
    detachChild(&child);
    return;
  }
}

PresentationStartResult MainWindow::showDialog(Dialog& dialog,
                                               Dialog::CallbackFn callback_fn) {
  PresentationStartResult result = transient_presentation_slot_.show(
      dialog.registration(), TransientPresentationPolicy(true, true));
  if (result != PresentationStartResult::kStarted) return result;

  active_dialog_ = &dialog;
  attachChild(scrim_, bounds());
  pending_scrim_blit_ = true;
  Dimensions dims =
      dialog.measure(WidthSpec::AtMost(width()), HeightSpec::AtMost(height()));
  XDim offsetLeft = (width() - dims.width()) / 2;
  YDim offsetTop = (height() - dims.height()) / 2;
  attachChild(dialog, Rect(offsetLeft, offsetTop, offsetLeft + dims.width() - 1,
                           offsetTop + dims.height() - 1));
  dialog.beginPresentation(std::move(callback_fn));
  return PresentationStartResult::kStarted;
}

void MainWindow::clearDialog() {
  if (active_dialog_ != nullptr) {
    active_dialog_->close();
  }
}

void MainWindow::detachDialog(Dialog& dialog) {
  if (active_dialog_ != &dialog) return;
  active_dialog_ = nullptr;
  pending_scrim_blit_ = false;
  detachChild(&dialog);
  detachChild(&scrim_);
  invalidateInterior();
}

}  // namespace roo_windows
