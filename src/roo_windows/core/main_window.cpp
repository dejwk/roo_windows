#include "main_window.h"

#include <Arduino.h>

#include "roo_display/color/color.h"
#include "roo_display/color/color_set.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

namespace {

void MaybeAddColor(roo_display::internal::ColorSet& palette, Color color) {
  if (palette.size() >= 15) return;
  palette.insert(color);
}

Rect UnionNonEmpty(const Rect& a, const Rect& b) {
  if (a.empty()) return b;
  if (b.empty()) return a;
  return Rect::Extent(a, b);
}

bool IsInSubtree(const Widget& candidate, const Widget& subtree) {
  for (const Widget* current = &candidate; current != nullptr;
       current = current->parent()) {
    if (current == &subtree) return true;
  }
  return false;
}

}  // namespace

MainWindow::MainWindow(Application& app, const roo_display::Box& bounds)
    : Container(app.context()),
      app_(app),
      redraw_bounds_(bounds),
      scrim_(app.context()),
      host_layer_(app.context()),
      transient_surface_host_(*this),
      transient_presentation_slot_(*this) {
  parent_bounds_ = Rect(bounds);
  invalidateDescending();
  const ApplicationContext& context = app.context();
  roo_display::internal::ColorSet color_set;
  const FrameworkTheme& framework = context.theme().framework;
  MaybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kCanvas));
  MaybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kSurface));
  MaybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kEmphasis));
  MaybeAddColor(color_set, context.keyboardColorTheme().background);
  {
    Color c = framework.interaction.resolve(FrameworkColorRole::kSurface,
                                            InteractionState::kPressed);
    c = AlphaBlend(framework.color.resolve(FrameworkColorRole::kSurface), c);
    MaybeAddColor(color_set, c);
  }
  {
    Color c = framework.color.resolve(FrameworkColorRole::kEmphasis);
    c.set_a(framework.interaction.disabledContentOpacity);
    c = AlphaBlend(framework.color.resolve(FrameworkColorRole::kSurface), c);
    MaybeAddColor(color_set, c);
  }

  MaybeAddColor(color_set, context.keyboardColorTheme().normalButton);
  MaybeAddColor(color_set,
                framework.color.resolve(FrameworkColorRole::kCritical));
  MaybeAddColor(color_set, context.keyboardColorTheme().modifierButton);
  Color palette[color_set.size()];
  std::copy(color_set.begin(), color_set.end(), palette);
  // background_fill_buffer_.setPalette(palette, color_set.size());
  // background_fill_buffer_.setPrefilled(
  //     framework.color.resolve(FrameworkColorRole::kCanvas));
}

bool MainWindow::fillTouchTargetPath(XDim x, YDim y,
                                     std::vector<Widget*>& path) {
  const size_t begin = path.size();
  if (!Container::fillTouchTargetPath(x, y, path)) return false;
  if (!app_.keyboard().getContents().isVisible()) return true;
  const Widget* editor = app_.activeTextEditorWidget();
  for (size_t i = begin; i < path.size(); ++i) {
    // Hosted surfaces retain their own outside-interaction policy.
    if (path[i] == &host_layer_ ||
        path[i] == app_.keyboard().getContents().parent() ||
        path[i] == editor) {
      return true;
    }
  }
  // Consume the gesture so closing the keyboard cannot also activate a
  // control underneath it or move the form during an in-flight gesture.
  path.resize(begin + 1);
  return true;
}

bool MainWindow::supportsTap() const {
  return app_.keyboard().getContents().isVisible();
}

void MainWindow::onSingleTapUp(XDim x, YDim y) {
  if (!app_.keyboard().getContents().isVisible()) return;
  app_.dismissTextEditor();
}

void MainWindow::transientActivityObserverSubtreeDetaching(Widget& subtree) {
  transient_presentation_slot_.activityObserverSubtreeDetaching(subtree);
}

MainWindow::~MainWindow() {
  beginShutdown();
  prepareForDestruction();
}

void MainWindow::prepareForDestruction() {
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

void MainWindow::beginShutdown() {
  transient_presentation_slot_.shutdown(
      PresentationFinishReason::kHostDestroyed);
}

void MainWindow::attachTransientHostLayer() {
  CHECK(host_layer_.parent() == nullptr);
  attachChild(WidgetRef(host_layer_), bounds());
}

void MainWindow::detachTransientHostLayer() {
  if (host_layer_.parent() == this) detachChild(&host_layer_);
}

void MainWindow::cancelTaskKeyActivationForDisplayCoverage() {
  for (Widget* child : tasks_) {
    if (Task* task = child->getTask(); task != nullptr) {
      task->cancelKeyActivation();
    }
  }
  for (Widget* child : popups_) {
    if (Task* task = child->getTask(); task != nullptr) {
      task->cancelKeyActivation();
    }
  }
}

void MainWindow::cancelGesturesInSubtree(Widget& subtree) {
  click_animation_.cancelInSubtree(subtree);
  app_.window().cancelGestureTargetsInSubtree(subtree);
}

void MainWindow::flushPendingOutsideInteraction() {
  transient_surface_host_.flushPendingOutsideInteraction();
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

void MainWindow::paintWindow(const roo_display::Surface& s) {
  preparePresentationPinsForPaint();
  if (!initialized_) {
    initialized_ = true;
    s.drawObject(roo_display::Fill(
        theme().framework.color.resolve(FrameworkColorRole::kCanvas)));
  }
  if (!isDirty()) return;
  Canvas canvas(&s);
  canvas.clipToExtents(redraw_bounds_);
  // New invalidations raised during painting belong to the next refresh.
  redraw_bounds_ = Rect(0, 0, -1, -1);
  Clipper clipper(clipper_state_, s.out());
  canvas.set_out(clipper.out());
  paintWidget(canvas, clipper);
  commitPresentationPinBounds();
}

namespace {

bool IsEffectivelyVisible(const Widget& widget) {
  for (const Widget* current = &widget; current != nullptr;
       current = current->parent()) {
    if (!current->isVisible()) return false;
  }
  return true;
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
    if (current->anchor_ != &anchor || !IsEffectivelyVisible(anchor)) {
      continue;
    }
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
    if (&current->effectiveZScopeRoot() != &root ||
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
  const bool grouped = mayHaveUnclippedChildren();
  const int group_count = grouped ? 2 : 1;
  for (int group = 0; group < group_count; ++group) {
    const ParentClipMode selected =
        group == 0 ? ParentClipMode::kUnclipped : ParentClipMode::kClipped;
    for (int i = getChildrenCount() - 1; i >= 0; --i) {
      Widget& child = getChild(i);
      if (grouped && child.getParentClipMode() != selected) continue;
      if (!grouped) {
        DCHECK(child.getParentClipMode() == ParentClipMode::kClipped);
      }
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
}

void MainWindow::propagateDirty(const Widget* child, const Rect& rect) {
  Rect clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = Rect::Intersect(rect, bounds());
  }
  setDirty(clipped);
  if (!clipped.empty()) {
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

}  // namespace roo_windows
