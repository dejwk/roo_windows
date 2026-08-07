#include "application.h"

#include "roo_display.h"
#include "roo_logging.h"
#include "roo_threads.h"
#include "roo_threads/semaphore.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

static constexpr int kKeyDrainBatchSize = 4;
static constexpr int kMaxKeyDrainBatchesPerTick = 4;

Application::Application(const Environment* env, roo_display::Display& display)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      text_field_editor_(env->scheduler(), keyboard_),
      key_source_(nullptr),
      window_(*this, display, true),
      ticker_(env->scheduler(), [this]() { tick(); }) {
  roo_windows::Task* kb_task = addPopupTaskFloating();
  kb_task->enterActivity(&keyboard_);
}

Application::Application(const Environment* env, roo_display::Display& display,
                         KeySource& keys, bool enable_touch)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      text_field_editor_(env->scheduler(), keyboard_),
      key_source_(&keys),
      window_(*this, display, enable_touch),
      ticker_(env->scheduler(), [this]() { tick(); }) {
  roo_windows::Task* kb_task = addPopupTaskFloating();
  kb_task->enterActivity(&keyboard_);
}

Application::~Application() {
  ticker_.cancel();
  window_.stop();
  for (const std::unique_ptr<Task>& task : tasks_) {
    task->clear();
  }
}

void Application::add(WidgetRef child, const roo_display::Box& box) {
  window_.root().addTask(std::move(child), box);
}

void Application::addPopup(WidgetRef child, const roo_display::Box& box) {
  window_.root().addPopup(std::move(child), box);
}

BackResult Application::requestBack(Task& target, BackSource source) {
  CHECK(ownsTask(target));
  if (window_.root().transient_presentation_slot().requestBack(source) ==
      BackResult::kHandled) {
    return BackResult::kHandled;
  }
  return target.requestBack(source);
}

BackResult Application::requestBackFromFocused(BackSource source) {
  if (window_.root().transient_presentation_slot().requestBack(source) ==
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
    BackSource source = event.code == KeyCode::kBack ? BackSource::kBackKey
                                                     : BackSource::kEscapeKey;
    if (requestBackFromFocused(source) == BackResult::kHandled) return;
    // An unhandled request continues through normal focused-widget dispatch,
    // including structural ancestors, just like every other key.
  }
  if ((event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) &&
      event.code == KeyCode::kTab) {
    context_.focus().moveFocus(window_.root(),
                               (event.modifiers & kKeyModifierShift) != 0);
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
      case KeyCode::kUp:
        direction = FocusDirection::kUp;
        break;
      case KeyCode::kDown:
        direction = FocusDirection::kDown;
        break;
      case KeyCode::kLeft:
        direction = FocusDirection::kLeft;
        break;
      case KeyCode::kRight:
        direction = FocusDirection::kRight;
        break;
      default:
        goto no_directional_traversal;
    }
    if (context_.focus().moveFocusDirection(window_.root(), direction)) return;
  }
no_directional_traversal:
  bool primary = event.code == KeyCode::kEnter || event.code == KeyCode::kSpace;
  if (!primary) return;
  if (event.phase == KeyPhase::kDown && focused->isClickable() &&
      focused->isEnabled()) {
    armed_key_widget_ = focused;
    armed_key_ = event.code;
    // Share the base visual press lifecycle without invoking touch-specific
    // onShowPress() overrides such as slider value changes.
    focused->Widget::onShowPress(focused->width() / 2, focused->height() / 2);
  } else if (event.phase == KeyPhase::kUp && armed_key_widget_ == focused &&
             armed_key_ == event.code) {
    armed_key_widget_ = nullptr;
    armed_key_ = KeyCode::kUnknown;
    focused->onSingleTapUp(focused->width() / 2, focused->height() / 2);
  }
}

bool Application::refresh(roo_time::Uptime deadline) {
  return window_.refresh(deadline);
}

Task* Application::addTask(const roo_display::Box& bounds) {
  auto task = std::unique_ptr<Task>(new Task());
  auto task_panel = std::unique_ptr<TaskPanel>(new TaskPanel(context_, *task));
  task->init(task_panel.get());
  window_.root().addTask(*task_panel, bounds);
  tasks_.push_back(std::move(task));
  task_panels_.push_back(std::move(task_panel));
  return tasks_.back().get();
}

Task* Application::addPopupTask(const roo_display::Box& bounds) {
  auto task = std::unique_ptr<Task>(new Task());
  auto task_panel = std::unique_ptr<TaskPanel>(new TaskPanel(context_, *task));
  task->init(task_panel.get());
  window_.root().addPopup(*task_panel, bounds);
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
