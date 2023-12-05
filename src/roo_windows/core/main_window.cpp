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

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;

namespace {

void maybeAddColor(roo_display::internal::ColorSet& palette, Color color) {
  if (palette.size() >= 15) return;
  palette.insert(color);
}

}  // namespace

MainWindow::MainWindow(Application& app, const roo_display::Box& bounds)
    : Panel(app.env(), app.env().theme().color.background),
      app_(app),
      redraw_bounds_(bounds),
      background_fill_buffer_(bounds.width(), bounds.height()) {
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
  background_fill_buffer_.setPalette(palette, color_set.size());
}

Application& MainWindow::app() const { return app_; }
const Theme& MainWindow::theme() const { return app().env().theme(); }

void MainWindow::tick() { click_animation_.tick(); }

void MainWindow::updateLayout() {
  if (isLayoutRequested()) {
    measure(WidthSpec::Exactly(width()), HeightSpec::Exactly(height()));
    layout(bounds());
  }
}

void MainWindow::paintWindow(const roo_display::Surface& s) {
  if (!isDirty()) return;
  Canvas canvas(&s);
  canvas.clipToExtents(redraw_bounds_);
  redraw_bounds_ = roo_display::Box(0, 0, -1, -1);
  roo_display::BackgroundFillOptimizer bg_optimizer(s.out(),
                                                    background_fill_buffer_);
  Clipper clipper(clipper_state_, bg_optimizer);
  canvas.set_out(clipper.out());
  paintWidget(canvas, clipper);
  // canvas.fillRect(canvas.clip_box(), roo_display::color::Black);
}

void MainWindow::propagateDirty(const Widget* child, const Rect& rect) {
  Rect clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = Rect::Intersect(rect, bounds());
  }
  markDirty(clipped);
  if (!clipped.empty()) {
    if (redraw_bounds_.empty()) {
      redraw_bounds_ = clipped;
    } else {
      redraw_bounds_ = Rect::Extent(redraw_bounds_, clipped);
    }
  }
}

}  // namespace roo_windows
