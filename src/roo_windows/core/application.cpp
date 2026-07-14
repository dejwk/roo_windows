#include "application.h"

#include "roo_display.h"
#include "roo_logging.h"
#include "roo_threads.h"
#include "roo_threads/semaphore.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

using roo_display::Display;

static constexpr roo_time::Duration kMinRefreshDuration = roo_time::Millis(200);

// Do not refresh display more frequently than this (50 Hz).
static constexpr long kMinRefreshTimeDeltaMs = 20;
static constexpr int kKeyDrainBatchSize = 4;
static constexpr int kMaxKeyDrainBatchesPerTick = 4;

Application::Application(const Environment* env, Display& display)
    : display_(display),
      env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      text_field_editor_(env->scheduler(), keyboard_),
      root_window_(*this, display.extents()),
      touch_sensor_(display),
      gesture_detector_(root_window_, touch_sensor_),
      key_source_(nullptr),
      touch_enabled_(true),
      ticker_(env->scheduler(), [this]() { tick(); }),
      paint_interval_(kMinRefreshDuration) {
  roo_windows::Task* kb_task = addPopupTaskFloating();
  kb_task->enterActivity(&keyboard_);
}

Application::Application(const Environment* env, Display& display,
                         KeySource& keys, bool enable_touch)
    : display_(display),
      env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      text_field_editor_(env->scheduler(), keyboard_),
      root_window_(*this, display.extents()),
      touch_sensor_(display),
      gesture_detector_(root_window_, touch_sensor_),
      key_source_(&keys),
      touch_enabled_(enable_touch),
      ticker_(env->scheduler(), [this]() { tick(); }),
      paint_interval_(kMinRefreshDuration) {
  roo_windows::Task* kb_task = addPopupTaskFloating();
  kb_task->enterActivity(&keyboard_);
}

Application::~Application() {
  for (const std::unique_ptr<Task>& task : tasks_) {
    task->clear();
  }
}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  root_window_.addTask(std::move(child), box);
}

void Application::addPopup(WidgetRef child, const roo_display::Box& box) {
  root_window_.addPopup(std::move(child), box);
}

BackResult Application::requestBack(Task& target, BackSource source) {
  CHECK(ownsTask(target));
  if (root_window_.transient_presentation_slot().requestBack(source) ==
      BackResult::kHandled) {
    return BackResult::kHandled;
  }
  return target.requestBack(source);
}

BackResult Application::requestBackFromFocused(BackSource source) {
  if (root_window_.transient_presentation_slot().requestBack(source) ==
      BackResult::kHandled) {
    return BackResult::kHandled;
  }
  Widget* focused = context_.focus().focused();
  if (focused == nullptr) return BackResult::kUnhandled;
  Task* target = focused->getTask();
  if (target == nullptr) return BackResult::kUnhandled;
  CHECK(ownsTask(*target));
  return target->requestBack(source);
}

void Application::start() {
  ui_thread_id_ = roo::this_thread::get_id();
  if (touch_enabled_) touch_sensor_.start();
  ticker_.scheduleNow();
}

void Application::run() {
  start();
  env_->scheduler().run();
}

void Application::tick() {
  unsigned long now = millis();
  root_window_.refreshClickAnimation();
  bool is_click_animating = root_window_.click_animation().isClickAnimating();
  bool key_events_pending = drainKeyEvents();
#if defined(ROO_THREADS_SINGLETHREADED)
  if (touch_enabled_) touch_sensor_.pollOnce();
#endif
  bool gesture_dispatched = touch_enabled_ && gesture_detector_.tick();
  bool touch_active = touch_enabled_ && gesture_detector_.isTouchDown();
  bool redraw_timeout = false;
  if ((now - last_time_refreshed_ms_) >= kMinRefreshTimeDeltaMs) {
    bool completed = refresh(roo_time::Uptime::Now() + paint_interval_);
    if (!completed) {
      paint_interval_ = paint_interval_ * 2;
      redraw_timeout = true;
    } else {
      paint_interval_ = kMinRefreshDuration;
    }
  }
  roo_scheduler::Priority priority = is_click_animating || touch_active
                                         ? roo_scheduler::PRIORITY_NORMAL
                                         : roo_scheduler::PRIORITY_NORMAL;
  roo_time::Duration delay =
      key_events_pending || gesture_dispatched || touch_active || redraw_timeout
          ? roo_time::Millis(0)
          : roo_time::Millis(20);
  ticker_.scheduleAfter(delay, priority);
}

bool Application::drainKeyEvents() {
  if (key_source_ == nullptr) return false;

  KeyEvent events[kKeyDrainBatchSize];
  int total = 0;
  for (int batch = 0; batch < kMaxKeyDrainBatchesPerTick; ++batch) {
    int count = key_source_->drain(events, kKeyDrainBatchSize);
    if (count <= 0) return false;
    if (count > kKeyDrainBatchSize) count = kKeyDrainBatchSize;
    for (int i = 0; i < count; ++i) dispatchKeyEvent(events[i]);
    total += count;
    if (count < kKeyDrainBatchSize) return false;
  }
  return total == kKeyDrainBatchSize * kMaxKeyDrainBatchesPerTick;
}

void Application::dispatchKeyEvent(const KeyEvent& event) {
  if (event.phase == KeyPhase::kDown &&
      (event.code == KeyCode::kBack || event.code == KeyCode::kEscape)) {
    BackSource source = event.code == KeyCode::kBack
                            ? BackSource::kBackKey
                            : BackSource::kEscapeKey;
    if (requestBackFromFocused(source) == BackResult::kHandled) return;
    // An unhandled request continues through normal focused-widget dispatch,
    // including structural ancestors, just like every other key.
  }
  if ((event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) &&
      event.code == KeyCode::kTab) {
    context_.focus().moveFocus(
        root_window_, (event.modifiers & kKeyModifierShift) != 0);
    return;
  }
  Widget* focused = context_.focus().focused();
  if (focused == nullptr) return;
  if (focused->onKeyEvent(event)) return;
  // A focused child gets the first chance to consume a key; scroll containers
  // and other structural ancestors can then handle keys the child does not
  // own (for example PageDown after a button leaves it unhandled).
  for (Widget* ancestor = focused->parent(); ancestor != nullptr;
       ancestor = ancestor->parent()) {
    if (ancestor->onKeyEvent(event)) return;
  }
  if (event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) {
    FocusDirection direction;
    switch (event.code) {
      case KeyCode::kUp: direction = FocusDirection::kUp; break;
      case KeyCode::kDown: direction = FocusDirection::kDown; break;
      case KeyCode::kLeft: direction = FocusDirection::kLeft; break;
      case KeyCode::kRight: direction = FocusDirection::kRight; break;
      default: goto no_directional_traversal;
    }
    if (context_.focus().moveFocusDirection(root_window_, direction)) return;
  }
no_directional_traversal:
  bool primary = event.code == KeyCode::kEnter || event.code == KeyCode::kSpace;
  if (!primary) return;
  if (event.phase == KeyPhase::kDown && focused->isClickable() &&
      focused->isEnabled()) {
    armed_key_widget_ = focused;
    armed_key_ = event.code;
    focused->setPressed(true);
  } else if (event.phase == KeyPhase::kUp && armed_key_widget_ == focused &&
             armed_key_ == event.code) {
    armed_key_widget_ = nullptr;
    armed_key_ = KeyCode::kUnknown;
    focused->onSingleTapUp(focused->width() / 2, focused->height() / 2);
  }
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
  dc.setFillMode(roo_display::FillMode::kExtents);
  Adapter adapter(root_window_, deadline);
  dc.draw(adapter);
  return adapter.completed();
}

Task* Application::addTask(const roo_display::Box& bounds) {
  auto task = std::unique_ptr<Task>(new Task());
  auto task_panel = std::unique_ptr<TaskPanel>(new TaskPanel(context_, *task));
  task->init(task_panel.get());
  root_window_.addTask(*task_panel, bounds);
  tasks_.push_back(std::move(task));
  task_panels_.push_back(std::move(task_panel));
  return tasks_.back().get();
}

Task* Application::addPopupTask(const roo_display::Box& bounds) {
  auto task = std::unique_ptr<Task>(new Task());
  auto task_panel = std::unique_ptr<TaskPanel>(new TaskPanel(context_, *task));
  task->init(task_panel.get());
  root_window_.addPopup(*task_panel, bounds);
  tasks_.push_back(std::move(task));
  task_panels_.push_back(std::move(task_panel));
  return tasks_.back().get();
}

bool Application::ownsTask(const Task& task) const {
  for (const std::unique_ptr<Task>& candidate : tasks_) {
    if (candidate.get() == &task) return true;
  }
  return false;
}

PresentationStartResult Application::showDialog(Dialog& dialog,
                                                 Dialog::CallbackFn callback_fn) {
  return root_window_.showDialog(dialog, std::move(callback_fn));
}

PresentationStartResult Application::showAlertDialog(
    std::string title, std::string supporting_text,
    std::vector<std::string> button_labels, Dialog::CallbackFn callback_fn) {
  Dialog* dialog =
      new AlertDialog(context(), std::move(title), std::move(supporting_text),
                      std::move(button_labels));
  PresentationStartResult result = showDialog(
      *dialog, [dialog, callback_fn](int id) {
        if (callback_fn != nullptr) {
          callback_fn(id);
        }
        delete dialog;
      });
  if (result != PresentationStartResult::kStarted) delete dialog;
  return result;
}

void Application::clearDialog() { root_window_.clearDialog(); }

namespace {

class SyncTask : public roo_scheduler::Executable {
 public:
  SyncTask(std::function<void()> fn, roo::binary_semaphore& sem)
      : fn_(std::move(fn)), sem_(sem) {}

  void execute(roo_scheduler::ExecutionID id) override {
    fn_();
    sem_.release();
  }

 private:
  std::function<void()> fn_;
  roo::binary_semaphore& sem_;
};

}  // namespace

void Application::executeInUIThread(std::function<void()> fn) {
  if (roo::this_thread::get_id() == ui_thread_id_) {
    fn();
  } else {
    roo::binary_semaphore sem(0);
    SyncTask task(std::move(fn), sem);
    env_->scheduler().scheduleNow(task);
    // Wait until the task finishes.
    sem.acquire();
  }
}

}  // namespace roo_windows
