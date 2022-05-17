#include "application.h"

#include <Arduino.h>

#include "roo_display.h"

namespace roo_windows {

using roo_display::Display;

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;

Application::Application(const Environment* env, Display& display)
    : display_(display),
      env_(env),
      root_window_(*env, display.extents()),
      gesture_detector_(root_window_, display) {}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  root_window_.add(std::move(child), box);
}

void Application::tick() {
  root_window_.tick();
  if (!gesture_detector_.tick()) return;

  unsigned long now = millis();
  if ((long)(now - last_time_refreshed_ms_) < kMinRefreshTimeDeltaMs) return;
  last_time_refreshed_ms_ = now;
  roo_display::DrawingContext dc(display_);
  root_window_.update(dc);
}

void Application::addTask(Activity* activity, const roo_display::Box& bounds) {
  auto task = std::unique_ptr<Task>(new Task());
  auto task_panel = std::unique_ptr<TaskPanel>(new TaskPanel(*env_, *task));
  task->init(task_panel.get());
  root_window_.add(*task_panel, bounds);
  task->pushActivity(activity);
  tasks_.push_back(std::move(task));
  task_panels_.push_back(std::move(task_panel));
}

}  // namespace roo_windows
