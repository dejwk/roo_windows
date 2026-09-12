#include "roo_windows/core/display_window.h"

#include <Arduino.h>

#include <algorithm>

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

DisplayWindow::~DisplayWindow() {
  stop();
  // Root detachment can consult display-local gesture state. Empty the root
  // here, before automatic member destruction reaches the detector.
  root_.prepareForDestruction();
}

void DisplayWindow::start() {
  if (!touch_enabled_) return;
  Application& app = root_.app();
  touch_sensor_.setReadinessHandler([&app]() { app.requestInputTick(); });
  touch_sensor_.start(app.env().scheduler());
}

void DisplayWindow::stop() {
  if (touch_enabled_) {
    touch_sensor_.stop();
    touch_sensor_.setReadinessHandler(nullptr);
  }
  // Close admission before callbacks or task teardown can attempt to reopen a
  // presentation into a window whose input and paint services are stopping.
  root_.beginShutdown();
  gesture_detector_.cancel();
  root_.click_animation().cancelForWindowTeardown();
  root_.cancelPaintContinuation();
}

void DisplayWindow::servicePointerInput() {
  if (touch_enabled_) gesture_detector_.tick();
  root_.flushPendingOutsideInteraction();
}

roo_time::Uptime DisplayWindow::nextPaintDeadline() const {
  roo_time::Uptime now = roo_time::Uptime::Now();
  if (root_.hasPaintContinuation()) return now;
  roo_time::Uptime desired =
      std::min(root_.app().context().animations().nextFrameDeadline(),
               root_.click_animation().nextFrameDeadline());
  if (root_.isDirty() || root_.isLayoutRequested() ||
      root_.click_animation().needsSettlementRefresh())
    desired = now;
  if (desired == roo_time::Uptime::Max()) return desired;
  // Keep the existing wrapping millisecond refresh clock. Queries between
  // millisecond boundaries must not postpone the eligible paint time.
  uint32_t elapsed = static_cast<uint32_t>(now.inMillis()) -
                     static_cast<uint32_t>(last_time_refreshed_ms_);
  roo_time::Uptime eligible = now;
  if (elapsed < kMinRefreshTimeDeltaMs) {
    eligible += roo_time::Millis(kMinRefreshTimeDeltaMs - elapsed) -
                roo_time::Micros(now.inMicros() % 1000);
  }
  return std::max(desired, eligible);
}

roo_time::Uptime DisplayWindow::nextWorkDeadline() const {
  if (root_.transient_presentation_slot().hasPendingFrameworkWork()) {
    return roo_time::Uptime::Now();
  }
  return nextPaintDeadline();
}

void DisplayWindow::refreshIfDue() {
  if (serviceDeferredWork()) return;
  if (nextPaintDeadline() > roo_time::Uptime::Now()) return;
  bool completed = refreshPaint(roo_time::Uptime::Now() + paint_interval_);
  if (!completed) {
    paint_interval_ = paint_interval_ * 2;
  } else {
    paint_interval_ = kMinRefreshDuration;
  }
}

void DisplayWindow::cancelGestureTargetsInSubtree(Widget& subtree) {
  gesture_detector_.cancelTargetsInSubtree(subtree);
}

bool DisplayWindow::serviceDeferredWork() {
  root_.app().context().presentations().deliverPendingChanges();
  root_.transient_presentation_slot().deliverPendingActivityChanges();
  // Terminal delivery stays on a fresh framework entry after click settlement.
  // It can destroy the owner, so make it the last member access on this path.
  if (!root_.transient_presentation_slot().isDeferredFinishReady())
    return false;
  return root_.transient_presentation_slot().finishDeferredIfReady();
}

bool DisplayWindow::refresh(roo_time::Uptime deadline) {
  if (serviceDeferredWork()) return true;
  return refreshPaint(deadline);
}

bool DisplayWindow::refreshPaint(roo_time::Uptime deadline) {
  refreshing_ = true;
  AnimationRegistry& animations = root_.app().context().animations();
  if (!root_.hasPaintContinuation()) {
    root_.refreshClickAnimation();
    animations.beginFrame(roo_time::Uptime::Now());
    while (animations.dispatchNext()) {
      root_.app().context().presentations().deliverPendingChanges();
    }
    animations.endFrame();
  }
  root_.updateLayout();
  // Layout can change effective presentation. Deliver that state before paint,
  // but do not run a second animation pass in the same logical frame.
  root_.app().context().presentations().deliverPendingChanges();
  last_time_refreshed_ms_ = millis();
  ClickAnimation& click_animation = root_.click_animation();
  bool completed;
  {
    roo_display::DrawingContext context(display_);
    context.setFillMode(roo_display::FillMode::kExtents);
    Adapter adapter(root_, deadline);
    context.draw(adapter);
    completed = adapter.completed();
  }
  refreshing_ = false;
  root_.app().requestAnimationFrameAt(nextWorkDeadline());
  // Semantic delivery can destroy the application; do not touch members after
  // it.
  if (completed) click_animation.notifyRefreshCompleted();
  return completed;
}

void DisplayWindow::requestRefresh() { root_.invalidateInterior(); }

}  // namespace roo_windows
