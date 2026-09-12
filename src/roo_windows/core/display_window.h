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

  /// Performs one layout-and-paint pass on the borrowed display.
  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max());

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

  /// Services deferred work and attempts at most one eligible paint slice.
  void refreshIfDue();

  /// Collects immediate framework work and the next eligible frame deadline.
  roo_time::Uptime nextWorkDeadline() const;

  /// Returns the next frame opportunity, applying cadence only to new frames.
  roo_time::Uptime nextPaintDeadline() const;

  /// Delivers one notification batch; returns true after terminal delivery.
  bool serviceDeferredWork();

  /// Samples and paints one logical frame or resumes its retained continuation.
  bool refreshPaint(roo_time::Uptime deadline);

  /// Clears gesture references into a subtree before it loses parent links.
  void cancelGestureTargetsInSubtree(Widget& subtree);

  roo_display::Display& display_;
  MainWindow root_;
  TouchSensor touch_sensor_;
  GestureDetector gesture_detector_;
  bool touch_enabled_;
  bool refreshing_ = false;
  unsigned long last_time_refreshed_ms_ = 0;
  roo_time::Duration paint_interval_ = roo_time::Millis(200);
};

}  // namespace roo_windows
