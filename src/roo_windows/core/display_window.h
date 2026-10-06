#pragma once

#include "roo_display.h"
#include "roo_time.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/touch_sensor.h"

namespace roo_windows {

class Application;
namespace test {
struct ApplicationWorkTestAccess;
}

/// Owns display-local rendering and pointer-input runtime state for one
/// application. The display itself is borrowed and must outlive this window.
class DisplayWindow {
 public:
  /// Stops display-local input and rendering state before destruction.
  ~DisplayWindow();

  DisplayWindow(const DisplayWindow&) = delete;
  DisplayWindow& operator=(const DisplayWindow&) = delete;
  DisplayWindow(DisplayWindow&&) = delete;
  DisplayWindow& operator=(DisplayWindow&&) = delete;

  /// Returns the borrowed display bound for this window's lifetime.
  roo_display::Display& display() { return display_; }

  /// Returns the borrowed display bound for this window's lifetime.
  const roo_display::Display& display() const { return display_; }

  /// Returns the window's root widget tree.
  MainWindow& root() { return root_; }

  /// Returns the window's root widget tree.
  const MainWindow& root() const { return root_; }

  /// Returns the display-local gesture dispatcher.
  GestureDetector& gestureDetector() { return gesture_detector_; }

  /// Returns the display-local gesture dispatcher.
  const GestureDetector& gestureDetector() const { return gesture_detector_; }

  /// Completes layout and painting on the borrowed display, then delivers
  /// pending click settlement after the drawing context closes.
  void refresh();

  /// Limits optional background work in the next refresh. Zero is unlimited;
  /// negative budgets fail a checked precondition. Painting still completes.
  void setAdvisoryPaintBudget(roo_time::Duration budget);

  /// Invalidates the full root and requests application work asynchronously.
  void requestRefresh();

 private:
  friend class Application;
  friend struct test::ApplicationWorkTestAccess;
  friend class MainWindow;

  /// Constructs the mandatory window for its owning application.
  DisplayWindow(Application& app, roo_display::Display& display,
                bool touch_enabled);

  /// Starts touch acquisition when this window has touch enabled.
  void start();

  /// Stops acquisition and clears transient gesture and paint state.
  void stop();

  /// Drains pointer input and dispatches gestures and due transitions.
  void servicePointerInput();

  /// Services deferred work and completes at most one eligible refresh.
  void refreshIfDue();

  /// Collects immediate framework work and the next eligible frame deadline.
  roo_time::Uptime nextWorkDeadline() const;

  /// Returns the next frame opportunity, respecting the refresh cadence.
  roo_time::Uptime nextPaintDeadline() const;

  /// Delivers one notification batch; returns true after terminal delivery.
  bool serviceDeferredWork();

  /// Samples animation and completes one paint before settling click feedback.
  void refreshPaint();

  /// Clears gesture references into a subtree before it loses parent links.
  void cancelGestureTargetsInSubtree(Widget& subtree);

  roo_display::Display& display_;
  MainWindow root_;
  TouchSensor touch_sensor_;
  GestureDetector gesture_detector_;
  roo_time::Duration advisory_paint_budget_;
  bool touch_enabled_;
  bool refreshing_ = false;
  unsigned long last_time_refreshed_ms_ = 0;
};

}  // namespace roo_windows
