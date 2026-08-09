#include "application.h"

#include "roo_display.h"
#include "roo_logging.h"
#include "roo_threads.h"
#include "roo_threads/semaphore.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

Application::Application(const Environment* env, roo_display::Display& display)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      window_(*this, display, true),
      ticker_(env->scheduler(), [this]() { tick(); }) {
  auto keyboard_task = std::unique_ptr<UiTask>(new UiTask(
      *this, window_, roo_display::Box(0, 0, -1, -1), true, keyboard_));
  keyboard_task->legacyActivities().enterActivity(&keyboard_);
  ui_tasks_.push_back(std::move(keyboard_task));
  if (pending_key_source_ != nullptr) {
    ui_tasks_.front()->attachKeySource(*pending_key_source_);
  }
}

Application::Application(const Environment* env, roo_display::Display& display,
                         KeySource& keys, bool enable_touch)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      pending_key_source_(&keys),
      window_(*this, display, enable_touch),
      ticker_(env->scheduler(), [this]() { tick(); }) {
  auto keyboard_task = std::unique_ptr<UiTask>(new UiTask(
      *this, window_, roo_display::Box(0, 0, -1, -1), true, keyboard_));
  keyboard_task->legacyActivities().enterActivity(&keyboard_);
  ui_tasks_.push_back(std::move(keyboard_task));
  if (pending_key_source_ != nullptr) {
    ui_tasks_.front()->attachKeySource(*pending_key_source_);
  }
}

Application::~Application() {
  ticker_.cancel();
  window_.stop();
  ui_tasks_.clear();
}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  window_.root().addTask(std::move(child), box);
}

void Application::addPopup(WidgetRef child, const roo_display::Box& box) {
  window_.root().addPopup(std::move(child), box);
}

BackResult Application::requestBack(Task& target, BackSource source) {
  CHECK(ownsTask(target));
  return target.uiTask().requestBack(source);
}

void Application::start() {
  ui_thread_id_ = roo::this_thread::get_id();
  ui_thread_started_ = true;
  window_.start();
  ticker_.scheduleNow();
}

void Application::run() {
  start();
  env_->scheduler().run();
}

void Application::tick() {
  // A continuation must finish the exact frame snapshot whose foreground
  // exclusions were preserved. Advance animation again after it completes.
  window_.advanceFrameState();
  bool key_events_pending = false;
  for (const std::unique_ptr<UiTask>& task : ui_tasks_) {
    key_events_pending = task->drainKeyEvents() || key_events_pending;
  }
  bool touch_active = false;
  bool gesture_dispatched = window_.servicePointerInput(touch_active);
  bool redraw_timeout = false;
  window_.refreshIfDue(redraw_timeout);
  roo_time::Duration delay =
      key_events_pending || gesture_dispatched || touch_active || redraw_timeout
          ? roo_time::Millis(0)
          : roo_time::Millis(20);
  ticker_.scheduleAfter(delay, roo_scheduler::PRIORITY_NORMAL);
}

bool Application::refresh(roo_time::Uptime deadline) {
  return window_.refresh(deadline);
}

Task* Application::addTask(const roo_display::Box& bounds) {
  return &addUiTask(bounds).legacyActivities();
}

Task* Application::addPopupTask(const roo_display::Box& bounds) {
  UiTask* task = new UiTask(*this, window_, bounds, true, keyboard_);
  ui_tasks_.emplace_back(task);
  return &task->legacyActivities();
}

UiTask& Application::addUiTask(const roo_display::Box& bounds) {
  UiTask* task = new UiTask(*this, window_, bounds, false, keyboard_);
  ui_tasks_.emplace_back(task);
  if (compatibility_task_ == nullptr) {
    compatibility_task_ = task;
    if (pending_key_source_ != nullptr) {
      for (const std::unique_ptr<UiTask>& candidate : ui_tasks_) {
        candidate->detachKeySource();
      }
      task->attachKeySource(*pending_key_source_);
    }
  }
  return *task;
}

UiTask& Application::addUiTaskFullScreen() {
  return addUiTask(window_.display().extents());
}

bool Application::ownsTask(const Task& task) const {
  for (const std::unique_ptr<UiTask>& candidate : ui_tasks_) {
    if (&candidate->legacyActivities() == &task) return true;
  }
  return false;
}

PresentationStartResult Application::showDialog(
    Dialog& dialog, Dialog::CallbackFn callback_fn) {
  return window_.root().showDialog(dialog, std::move(callback_fn));
}

PresentationStartResult Application::showAlertDialog(
    std::string title, std::string supporting_text,
    std::vector<std::string> button_labels, Dialog::CallbackFn callback_fn) {
  Dialog* dialog =
      new AlertDialog(context(), std::move(title), std::move(supporting_text),
                      std::move(button_labels));
  PresentationStartResult result =
      showDialog(*dialog, [dialog, callback_fn](int id) {
        if (callback_fn != nullptr) {
          callback_fn(id);
        }
        delete dialog;
      });
  if (result != PresentationStartResult::kStarted) delete dialog;
  return result;
}

void Application::clearDialog() { window_.root().clearDialog(); }

TextFieldEditor& Application::text_field_editor() {
  return (compatibility_task_ != nullptr ? compatibility_task_
                                         : ui_tasks_.front().get())
      ->textFieldEditor();
}

const TextFieldEditor& Application::text_field_editor() const {
  return (compatibility_task_ != nullptr ? compatibility_task_
                                         : ui_tasks_.front().get())
      ->textFieldEditor();
}

bool Application::isUiThread() const {
  return !ui_thread_started_ || roo::this_thread::get_id() == ui_thread_id_;
}

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
