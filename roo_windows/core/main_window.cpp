#include "main_window.h"

#include <Arduino.h>

#include "roo_display/core/color.h"
#include "roo_display/filter/clip_exclude_rects.h"
#include "roo_display/filter/foreground.h"
#include "roo_display/internal/hashtable.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/press_overlay.h"

namespace roo_windows {

using roo_display::Display;

static constexpr unsigned long kClickAnimationMs = 200;

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;

namespace {

void maybeAddColor(roo_display::internal::ColorSet& palette, Color color) {
  if (palette.size() >= 15) return;
  palette.insert(color);
}

}  // namespace

MainWindow::MainWindow(Application& app, const Box& bounds)
    : Panel(app.env(), app.env().theme().color.background),
      app_(app),
      redraw_bounds_(bounds),
      background_fill_buffer_(bounds.width(), bounds.height()) {
  parent_bounds_ = bounds;
  invalidateDescending();
  const Environment& env = app.env();
  roo_display::internal::ColorSet color_set;
  maybeAddColor(color_set, env.theme().color.background);
  maybeAddColor(color_set, env.theme().color.surface);
  maybeAddColor(color_set, env.theme().color.primary);
  maybeAddColor(color_set, env.theme().color.primaryVariant);
  maybeAddColor(color_set, env.theme().color.secondary);
  maybeAddColor(color_set, env.keyboardColorTheme().background);

  {
    Color c = env.theme().color.defaultColor(env.theme().color.surface);
    c.set_a(env.theme().pressAnimationOpacity(env.theme().color.surface));
    c = alphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }
  {
    Color c = env.theme().color.highlighterColor(env.theme().color.surface);
    c.set_a(env.theme().pressAnimationOpacity(env.theme().color.surface));
    c = alphaBlend(env.theme().color.surface, c);
    maybeAddColor(color_set, c);
  }

  maybeAddColor(color_set, env.keyboardColorTheme().normalButton);
  maybeAddColor(color_set, env.theme().color.error);
  maybeAddColor(color_set, env.keyboardColorTheme().modifierButton);
  Color palette[color_set.size()];
  std::copy(color_set.begin(), color_set.end(), palette);
  background_fill_buffer_.setPalette(palette, color_set.size());
}

Application& MainWindow::app() { return app_; }
const Application& MainWindow::app() const { return app_; }
const Theme& MainWindow::theme() const { return app().env().theme(); }

void MainWindow::tick() {
  click_animation_.tick();
}

void MainWindow::updateLayout() {
  if (isLayoutRequested()) {
    measure(MeasureSpec::Exactly(width()), MeasureSpec::Exactly(height()));
    layout(bounds());
  }
}

void MainWindow::paintWindow(const Surface& s) {
  if (!isDirty()) return;
  Surface news = s;
  news.clipToExtents(redraw_bounds_);
  redraw_bounds_ = Box(0, 0, -1, -1);
  news.set_fill_mode(roo_display::FILL_MODE_RECTANGLE);
  roo_display::BackgroundFillOptimizer bg_optimizer(s.out(),
                                                    background_fill_buffer_);
  Clipper clipper(clipper_state_, bg_optimizer);
  news.set_out(clipper.out());
  paintWidget(news, clipper);
}

void MainWindow::propagateDirty(const Widget* child, const Box& box) {
  Box clipped(0, 0, -1, -1);
  if (isVisible()) {
    clipped = Box::intersect(box, bounds());
  }
  markDirty(clipped);
  if (!clipped.empty()) {
    if (redraw_bounds_.empty()) {
      redraw_bounds_ = clipped;
    } else {
      redraw_bounds_ = Box::extent(redraw_bounds_, clipped);
    }
  }
}

}  // namespace roo_windows
