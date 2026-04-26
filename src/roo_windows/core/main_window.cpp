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

}  // namespace

MainWindow::MainWindow(Application& app, const roo_display::Box& bounds)
    : Container(app.env(), app.env().theme().color.background),
      app_(app),
      redraw_bounds_(bounds),
      scrim_(app.env()) {
  parent_bounds_ = Rect(bounds);
  invalidateDescending();
  const Environment& env = app.env();
  roo_display::internal::ColorSet color_set;
  maybeAddColor(color_set, env.theme().color.background);
  maybeAddColor(color_set, env.theme().color.surface);
  maybeAddColor(color_set, env.theme().color.primary);
  maybeAddColor(color_set, env.theme().color.primaryVariant);
  maybeAddColor(color_set, env.theme().color.secondary);
  maybeAddColor(color_set, env.keyboardColorTheme().background);
  maybeAddColor(color_set, env.theme().color.secondaryVariant);
  {
    Color c = env.theme().color.defaultColor(env.theme().color.surface);
    c.set_a(env.theme().pressAnimationOpacity(env.theme().color.surface));
    c = AlphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }
  {
    Color c = env.theme().color.highlighterColor(env.theme().color.surface);
    c.set_a(env.theme().pressAnimationOpacity(env.theme().color.surface));
    c = AlphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }
  {
    Color c = env.theme().color.primary;
    c.set_a(env.theme().state.disabled);
    c = AlphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }
  {
    Color c = env.theme().color.secondary;
    c.set_a(env.theme().state.disabled);
    c = AlphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }

  maybeAddColor(color_set, env.keyboardColorTheme().normalButton);
  maybeAddColor(color_set, env.theme().color.error);
  maybeAddColor(color_set, env.keyboardColorTheme().modifierButton);
  Color palette[color_set.size()];
  std::copy(color_set.begin(), color_set.end(), palette);
  // background_fill_buffer_.setPalette(palette, color_set.size());
  // background_fill_buffer_.setPrefilled(env.theme().color.background);
}

MainWindow::~MainWindow() {
  if (active_dialog_ != nullptr) {
    detachChild(active_dialog_);
    detachChild(&scrim_);
    active_dialog_ = nullptr;
  }
  while (!popups_.empty()) {
    removeLastFromLayer(popups_);
  }
  while (!tasks_.empty()) {
    removeLastFromLayer(tasks_);
  }
}

Application& MainWindow::app() const { return app_; }
const Theme& MainWindow::theme() const { return app().env().theme(); }

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

void MainWindow::refreshClickAnimation() { click_animation_.tick(); }

void MainWindow::updateLayout() {
  if (isLayoutRequested()) {
    measure(WidthSpec::Exactly(width()), HeightSpec::Exactly(height()));
    layout(bounds());
  }
}

bool MainWindow::paintWindow(const roo_display::Surface& s,
                             roo_time::Uptime deadline) {
  if (!initialized_) {
    initialized_ = true;
    s.drawObject(roo_display::Fill(theme().color.background));
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
  if (!isDirty()) return true;
  Canvas canvas(&s);
  canvas.clipToExtents(redraw_bounds_);
  Rect old_redraw_bounds = redraw_bounds_;
  redraw_bounds_ = Rect(0, 0, -1, -1);
  Clipper clipper(clipper_state_, s.out(), deadline);
  canvas.set_out(clipper.out());
  paintWidget(canvas, clipper);
  if (clipper.isDeadlineExceeded()) {
    // Preserve incremental timeout recovery: keep pending redraw scope and
    // continue from partially drawn state in the next frame.
    redraw_bounds_ = old_redraw_bounds;
    return false;
  }
  return true;
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

void MainWindow::removeLastFromLayer(std::vector<Widget*>& layer) {
  Widget* widget = layer.back();
  layer.pop_back();
  detachChild(widget);
}

void MainWindow::showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn) {
  CHECK(active_dialog_ == nullptr) << "Can't show two dialogs at the same time";
  active_dialog_ = &dialog;
  attachChild(scrim_, bounds());
  pending_scrim_blit_ = true;
  dialog.onEnter();
  dialog.setCallbackFn([this, callback_fn, &dialog](int id) {
    dialog.onExit(id);
    active_dialog_ = nullptr;
    pending_scrim_blit_ = false;
    detachChild(&dialog);
    detachChild(&scrim_);
    invalidateInterior();
    Dialog::CallbackFn fn = callback_fn;
    dialog.setCallbackFn(nullptr);
    fn(id);
  });
  Dimensions dims =
      dialog.measure(WidthSpec::AtMost(width()), HeightSpec::AtMost(height()));
  XDim offsetLeft = (width() - dims.width()) / 2;
  YDim offsetTop = (height() - dims.height()) / 2;
  attachChild(dialog, Rect(offsetLeft, offsetTop, offsetLeft + dims.width() - 1,
                           offsetTop + dims.height() - 1));
}

void MainWindow::clearDialog() {
  if (active_dialog_ != nullptr) {
    active_dialog_->close();
  }
}

}  // namespace roo_windows
