#include "roo_windows/core/display_window.h"

#include <Arduino.h>

#include "roo_windows/core/application.h"

namespace roo_windows {
namespace {

constexpr roo_time::Duration kMinRefreshDuration = roo_time::Millis(200);
constexpr long kMinRefreshTimeDeltaMs = 20;

/// Adapts the root widget's bounded paint operation to DrawingContext.
class Adapter final : public roo_display::Drawable {
 public:
  Adapter(MainWindow& root, roo_time::Uptime deadline)
      : root_(root), deadline_(deadline) {}

  roo_display::Box extents() const override { return root_.bounds().asBox(); }

  void drawTo(const roo_display::Surface& surface) const override {
    completed_ = root_.paintWindow(surface, deadline_);
  }

  bool completed() const { return completed_; }

 private:
  MainWindow& root_;
  roo_time::Uptime deadline_;
  mutable bool completed_ = false;
};

}  // namespace

DisplayWindow::DisplayWindow(Application& app, roo_display::Display& display,
                             bool touch_enabled)
    : display_(display),
      root_(app, display.extents()),
      touch_sensor_(display),
      gesture_detector_(root_, touch_sensor_),
      touch_enabled_(touch_enabled) {}

DisplayWindow::~DisplayWindow() { stop(); }

void DisplayWindow::start() {
  if (touch_enabled_) touch_sensor_.start();
}

void DisplayWindow::stop() {
  if (touch_enabled_) touch_sensor_.stop();
  gesture_detector_.cancel();
  root_.click_animation().cancelForWindowTeardown();
  root_.cancelPaintContinuation();
}

void DisplayWindow::advanceFrameState() {
  if (!root_.hasPaintContinuation()) root_.refreshClickAnimation();
}

bool DisplayWindow::servicePointerInput(bool& touch_active) {
#if defined(ROO_THREADS_SINGLETHREADED)
  if (touch_enabled_) touch_sensor_.pollOnce();
#endif
  bool dispatched = touch_enabled_ && gesture_detector_.tick();
  touch_active = touch_enabled_ && gesture_detector_.isTouchDown();
  return dispatched;
}

bool DisplayWindow::refreshIfDue(bool& redraw_timeout) {
  redraw_timeout = false;
  unsigned long now = millis();
  if ((now - last_time_refreshed_ms_) < kMinRefreshTimeDeltaMs) return true;
  bool completed = refresh(roo_time::Uptime::Now() + paint_interval_);
  if (!completed) {
    paint_interval_ = paint_interval_ * 2;
    redraw_timeout = true;
  } else {
    paint_interval_ = kMinRefreshDuration;
  }
  return completed;
}

bool DisplayWindow::refresh(roo_time::Uptime deadline) {
  root_.updateLayout();
  last_time_refreshed_ms_ = millis();
  ClickAnimation& click_animation = root_.click_animation();
  if (!root_.hasPaintContinuation()) click_animation.sampleFrameTime();
  bool completed;
  {
    roo_display::DrawingContext context(display_);
    context.setFillMode(roo_display::FillMode::kExtents);
    Adapter adapter(root_, deadline);
    context.draw(adapter);
    completed = adapter.completed();
  }
  if (completed) click_animation.notifyRefreshCompleted();
  return completed;
}

void DisplayWindow::requestRefresh() { root_.invalidateInterior(); }

}  // namespace roo_windows
