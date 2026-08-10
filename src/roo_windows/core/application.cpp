#include "application.h"

#include "roo_display.h"
#include "roo_logging.h"
#include "roo_threads.h"
#include "roo_threads/semaphore.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

class TickerGuard {
 public:
  explicit TickerGuard(Application& app) : app_(app) {
    CHECK(app_.state_ == Application::State::kStarted);
    CHECK(roo::this_thread::get_id() == app_.ui_thread_id_);
    app_.state_ = Application::State::kTickerRunning;
  }

  ~TickerGuard() { app_.state_ = Application::State::kStarted; }

 private:
  Application& app_;
};

Application::Application(const Environment* env, roo_display::Display& display)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      window_(*this, display, true),
      ticker_(env->scheduler(), [this]() { tick(); }) {
  roo_display::Box keyboard_bounds(0, window_.root().height() / 2,
                                   window_.root().width() - 1,
                                   window_.root().height() - 1);
  auto keyboard_task = std::unique_ptr<Task>(new Task(
      *this, window_, keyboard_bounds, true, keyboard_, keyboard_.getContents()));
  keyboard_.setTask(*keyboard_task);
  keyboard_task->setVisible(false);
  tasks_.push_back(std::move(keyboard_task));
  if (pending_key_source_ != nullptr) {
    tasks_.front()->attachKeySource(*pending_key_source_);
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
  roo_display::Box keyboard_bounds(0, window_.root().height() / 2,
                                   window_.root().width() - 1,
                                   window_.root().height() - 1);
  auto keyboard_task = std::unique_ptr<Task>(new Task(
      *this, window_, keyboard_bounds, true, keyboard_, keyboard_.getContents()));
  keyboard_.setTask(*keyboard_task);
  keyboard_task->setVisible(false);
  tasks_.push_back(std::move(keyboard_task));
  if (pending_key_source_ != nullptr) {
    tasks_.front()->attachKeySource(*pending_key_source_);
  }
}

Application::~Application() {
  if (state_ != State::kConstructed) {
    CHECK(roo::this_thread::get_id() == ui_thread_id_);
    CHECK(state_ != State::kTickerRunning);
  }
  state_ = State::kStopping;
  ticker_.cancel();
  window_.stop();
  tasks_.clear();
}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  checkUiThread();
  window_.root().addTask(std::move(child), box);
}

void Application::addPopup(WidgetRef child, const roo_display::Box& box) {
  checkUiThread();
  window_.root().addPopup(std::move(child), box);
}

void Application::start() {
  CHECK(state_ == State::kConstructed);
  ui_thread_id_ = roo::this_thread::get_id();
  state_ = State::kStarted;
  window_.start();
  ticker_.scheduleNow();
}

void Application::run() {
  start();
  env_->scheduler().run();
}

void Application::tick() {
  TickerGuard ticker_guard(*this);

  // A continuation must finish the exact frame snapshot whose foreground
  // exclusions were preserved. Advance animation again after it completes.
  window_.advanceFrameState();
  bool key_events_pending = drainKeyEvents();
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

bool Application::drainKeyEvents() {
  bool key_events_pending = false;
  for (const std::unique_ptr<Task>& task : tasks_) {
    key_events_pending = task->drainKeyEvents() || key_events_pending;
  }
  return key_events_pending;
}

bool Application::refresh(roo_time::Uptime deadline) {
  checkUiThread();
  return window_.refresh(deadline);
}

Task& Application::addTask(Widget& content, const roo_display::Box& bounds) {
  checkUiThread();
  CHECK(content.parent() == nullptr);
  Task* task = new Task(*this, window_, bounds, false, keyboard_, content);
  tasks_.emplace_back(task);
  if (pending_key_source_ != nullptr) {
    for (const std::unique_ptr<Task>& candidate : tasks_) {
      candidate->detachKeySource();
    }
    task->attachKeySource(*pending_key_source_);
  }
  return *task;
}

Task& Application::addTaskFullScreen(Widget& content) {
  return addTask(content, window_.display().extents());
}

Task& Application::addTask(NavigationHost& navigation,
                           const roo_display::Box& bounds) {
  checkUiThread();
  CHECK(navigation.task_ == nullptr);
  Task* task = new Task(*this, window_, bounds, false, keyboard_, navigation);
  tasks_.emplace_back(task);
  if (pending_key_source_ != nullptr) {
    for (const std::unique_ptr<Task>& candidate : tasks_) {
      candidate->detachKeySource();
    }
    task->attachKeySource(*pending_key_source_);
  }
  return *task;
}

Task& Application::addTaskFullScreen(NavigationHost& navigation) {
  return addTask(navigation, window_.display().extents());
}

PresentationStartResult Application::showDialog(
    Dialog& dialog, Dialog::CallbackFn callback_fn) {
  checkUiThread();
  return window_.root().showDialog(dialog, std::move(callback_fn));
}

PresentationStartResult Application::showAlertDialog(
    std::string title, std::string supporting_text,
    std::vector<std::string> button_labels, Dialog::CallbackFn callback_fn) {
  checkUiThread();
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

void Application::clearDialog() {
  checkUiThread();
  window_.root().clearDialog();
}

bool Application::isUiThread() const {
  return state_ == State::kConstructed ||
         roo::this_thread::get_id() == ui_thread_id_;
}

void Application::checkUiThread() const {
  CHECK(state_ != State::kStopping);
  CHECK(isUiThread());
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
  CHECK(state_ == State::kStarted || state_ == State::kTickerRunning);
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
