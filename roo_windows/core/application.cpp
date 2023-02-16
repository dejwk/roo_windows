#include "application.h"

#include <Arduino.h>

#include "roo_display.h"

namespace roo_windows {

using roo_display::Display;

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;

Application::Application(const Environment* env, Display& display,
                         roo_scheduler::Scheduler& scheduler)
    : display_(display),
      scheduler_(scheduler),
      env_(env),
      root_window_(*this, display.extents()),
      gesture_detector_(root_window_, display) {}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  root_window_.add(std::move(child), box);
}

void Application::tick() {
  root_window_.tick();
  if (!gesture_detector_.tick()) return;
  {
    unsigned long now = millis();
    if ((long)(now - last_time_refreshed_ms_) < kMinRefreshTimeDeltaMs) return;
  }
  refresh();
}

namespace {

class Adapter : public roo_display::Drawable {
 public:
  Adapter(MainWindow& window) : window_(window) {}

  roo_display::Box extents() const override { return window_.bounds().asBox(); }

  void drawTo(const roo_display::Surface& s) const override {
    window_.paintWindow(s);
  }

 private:
  MainWindow& window_;
};

}  // namespace

void Application::refresh() {
  root_window_.updateLayout();
  last_time_refreshed_ms_ = millis();
  roo_display::DrawingContext dc(display_);
  dc.setFillMode(roo_display::FILL_MODE_RECTANGLE);
  dc.draw(Adapter(root_window_));
}

Task* Application::addTask(const roo_display::Box& bounds) {
  auto task = std::unique_ptr<Task>(new Task());
  auto task_panel = std::unique_ptr<TaskPanel>(new TaskPanel(*env_, *task));
  task->init(task_panel.get());
  root_window_.add(*task_panel, bounds);
  tasks_.push_back(std::move(task));
  task_panels_.push_back(std::move(task_panel));
  return tasks_.back().get();
}

}  // namespace roo_windows
