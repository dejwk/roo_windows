#include "application.h"

#include "roo_display.h"
#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

using roo_display::Display;

static constexpr roo_time::Interval kMinRefreshDuration = roo_time::Millis(30);

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;

Application::Application(const Environment* env, Display& display)
    : display_(display),
      env_(env),
      root_window_(*this, display.extents()),
      gesture_detector_(root_window_, display),
      keyboard_(*env, kbEngUS()),
      text_field_editor_(env->scheduler(), keyboard_),
      ticker_(env->scheduler(), [this]() { tick(); }),
      paint_interval_(kMinRefreshDuration) {
  roo_windows::Task* kb_task = addTaskFloating();
  kb_task->enterActivity(&keyboard_);
}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  root_window_.add(std::move(child), box);
}

void Application::start() { ticker_.scheduleNow(); }

void Application::tick() {
  unsigned long now = millis();
  root_window_.refreshClickAnimation();
  bool is_click_animating =
      root_window_.getClickAnimation()->isClickAnimating();
  bool gesture_dispatched = gesture_detector_.tick();
  bool redraw_timeout = false;
  if (gesture_dispatched ||
      (now - last_time_refreshed_ms_) >= kMinRefreshTimeDeltaMs) {
    bool completed = refresh(roo_time::Uptime::Now() + paint_interval_);
    if (!completed) {
      paint_interval_ = kMinRefreshDuration;
      redraw_timeout = true;
    } else {
      paint_interval_ = paint_interval_ * 2;
    }
  }
  roo_scheduler::Priority priority =
      is_click_animating || gesture_detector_.isTouchDown()
          ? roo_scheduler::PRIORITY_ELEVATED
          : roo_scheduler::PRIORITY_NORMAL;
  roo_time::Interval delay = gesture_detector_.isTouchDown() || redraw_timeout
                                 ? roo_time::Millis(0)
                                 : roo_time::Millis(20);
  ticker_.scheduleAfter(delay, priority);
}

namespace {

class Adapter : public roo_display::Drawable {
 public:
  Adapter(MainWindow& window, roo_time::Uptime deadline)
      : window_(window), deadline_(deadline), completed_(false) {}

  roo_display::Box extents() const override { return window_.bounds().asBox(); }

  void drawTo(const roo_display::Surface& s) const override {
    if (window_.paintWindow(s, deadline_)) {
      completed_ = true;
    }
  }

  bool completed() const { return completed_; }

 private:
  MainWindow& window_;
  roo_time::Uptime deadline_;
  mutable bool completed_;
};

}  // namespace

bool Application::refresh(roo_time::Uptime deadline) {
  root_window_.updateLayout();
  last_time_refreshed_ms_ = millis();
  roo_display::DrawingContext dc(display_);
  dc.setFillMode(roo_display::FILL_MODE_RECTANGLE);
  Adapter adapter(root_window_, deadline);
  dc.draw(adapter);
  return adapter.completed();
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
