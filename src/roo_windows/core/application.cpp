#include "application.h"

#include "roo_display.h"
#include "roo_logging.h"
#include "roo_threads.h"
#include "roo_threads/mutex.h"
#include "roo_threads/semaphore.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

/// Thread-safe, coalescing scheduler endpoint for one Application.
///
/// Producers can request an immediate dispatch from any thread. During a UI
/// dispatch, requests are recorded and merged into the single follow-up task
/// so that producer callbacks never re-enter application code.
class ApplicationTicker final : public roo_scheduler::Executable {
 public:
  /// Creates a ticker that invokes `callback` for each accepted dispatch.
  ApplicationTicker(roo_scheduler::Scheduler& scheduler,
                    std::function<void()> callback)
      : scheduler_(scheduler), callback_(std::move(callback)) {}

  /// Requests an application dispatch as soon as the scheduler can run it.
  void requestNow() { requestAt(roo_time::Uptime::Now()); }

  /// Requests a dispatch after `delay`, retaining an earlier pending request.
  void requestAfter(roo_time::Duration delay) {
    requestAt(roo_time::Uptime::Now() + delay);
  }

  /// Rejects future requests and cancels a pending scheduler execution.
  void stop() {
    roo::lock_guard<roo::mutex> lock(mutex_);
    stopped_ = true;
    if (scheduled_id_ >= 0) scheduler_.cancel(scheduled_id_);
    scheduled_id_ = -1;
    scheduled_at_ = roo_time::Uptime::Max();
    after_dispatch_at_ = roo_time::Uptime::Max();
  }

  /// Runs the current scheduler execution and schedules its merged successor.
  void execute(roo_scheduler::ExecutionID id) override {
    {
      roo::lock_guard<roo::mutex> lock(mutex_);
      if (stopped_ || id != scheduled_id_) return;
      scheduled_id_ = -1;
      scheduled_at_ = roo_time::Uptime::Max();
      dispatching_ = true;
      after_dispatch_at_ = roo_time::Uptime::Max();
    }

    callback_();

    roo::lock_guard<roo::mutex> lock(mutex_);
    dispatching_ = false;
    if (stopped_ || after_dispatch_at_ == roo_time::Uptime::Max()) return;
    scheduleLocked(after_dispatch_at_);
    after_dispatch_at_ = roo_time::Uptime::Max();
  }

 private:
  // Calls into the scheduler only while ticker state is protected. A request
  // made while dispatching is deferred instead, preventing recursive tick().
  void requestAt(roo_time::Uptime when) {
    roo::lock_guard<roo::mutex> lock(mutex_);
    if (stopped_) return;
    if (dispatching_) {
      if (when < after_dispatch_at_) after_dispatch_at_ = when;
      return;
    }
    if (scheduled_id_ >= 0 && scheduled_at_ <= when) return;
    if (scheduled_id_ >= 0) scheduler_.cancel(scheduled_id_);
    scheduleLocked(when);
  }

  void scheduleLocked(roo_time::Uptime when) {
    scheduled_at_ = when;
    scheduled_id_ =
        scheduler_.scheduleOn(when, *this, roo_scheduler::PRIORITY_NORMAL);
  }

  roo_scheduler::Scheduler& scheduler_;
  std::function<void()> callback_;
  roo::mutex mutex_;
  roo_scheduler::ExecutionID scheduled_id_ = -1;
  roo_time::Uptime scheduled_at_ = roo_time::Uptime::Max();
  roo_time::Uptime after_dispatch_at_ = roo_time::Uptime::Max();
  bool stopped_ = false;
  bool dispatching_ = false;
};

/// Owns all incoming physical-key routes for one Application.
///
/// Each KeySource supplies its intrusive list node, so connection and draining
/// require no per-route allocation and Task remains free of producer state.
class ApplicationInputRouter {
 public:
  /// Creates the router that delivers sources into `app`'s tasks.
  explicit ApplicationInputRouter(Application& app) : app_(app) {}

  /// Connects an unused source to an unused task in this application.
  ///
  /// After startup, this is a UI-thread operation. Connecting installs the
  /// producer callback immediately so pre-existing queued input wakes app.
  void connect(KeySource& source, Task& destination) {
    CHECK(source.destination_application_ == nullptr);
    CHECK(&destination.application() == &app_);
    CHECK(app_.state_ != Application::State::kStopping);
    if (app_.state_ != Application::State::kConstructed) {
      CHECK(app_.isUiThread());
    }
    for (KeySource* candidate = head_; candidate != nullptr;
         candidate = candidate->next_source_) {
      CHECK(candidate->destination_task_ != &destination);
    }

    source.destination_application_ = &app_;
    source.destination_task_ = &destination;
    source.next_source_ = head_;
    head_ = &source;
    if (app_.state_ != Application::State::kConstructed) {
      source.onReadinessStart();
      installReadinessHandler(source);
    }
  }

  /// Quiesces a source callback, cancels its pending fallback click, and
  /// removes the source from this application's incoming list.
  void disconnect(KeySource& source) {
    if (source.destination_application_ == nullptr) return;
    CHECK(source.destination_application_ == &app_);
    if (app_.state_ != Application::State::kStopping) {
      CHECK(app_.isUiThread());
    }
    // A host bridge must stop and quiesce before its handler can be removed.
    // That ordering prevents a bridge task from reaching a stale source.
    source.onReadinessStop();
    source.setReadinessHandler({});
    source.destination_task_->cancelKeyActivation();
    KeySource** link = &head_;
    while (*link != &source) {
      CHECK(*link != nullptr);
      link = &(*link)->next_source_;
    }
    *link = source.next_source_;
    source.destination_application_ = nullptr;
    source.destination_task_ = nullptr;
    source.next_source_ = nullptr;
  }

  /// Installs readiness callbacks for routes configured before app startup.
  void start() {
    for (KeySource* source = head_; source != nullptr;
         source = source->next_source_) {
      source->onReadinessStart();
      installReadinessHandler(*source);
    }
  }

  /// Disconnects every source before application-owned tasks are destroyed.
  void clear() {
    while (head_ != nullptr) disconnect(*head_);
  }

  /// Drains each source under its independent four-batch, four-event budget.
  ///
  /// Returns true when a source uses all sixteen event slots, which asks the
  /// periodic application path to schedule an immediate follow-up dispatch.
  bool drainReadySources() {
    bool has_full_budget_source = false;
    for (KeySource* source = head_; source != nullptr;
         source = source->next_source_) {
      bool consumed_full_budget = true;
      for (int batch = 0; batch < kMaxDrainBatches; ++batch) {
        KeyEvent events[kDrainBatchSize];
        int count = source->drain(events, kDrainBatchSize);
        if (count < 0) count = 0;
        if (count > kDrainBatchSize) count = kDrainBatchSize;
        for (int i = 0; i < count; ++i) {
          source->destination_task_->dispatchKeyEvent(events[i]);
        }
        if (count != kDrainBatchSize) {
          consumed_full_budget = false;
          break;
        }
      }
      has_full_budget_source = has_full_budget_source || consumed_full_budget;
    }
    return has_full_budget_source;
  }

 private:
  // The handler carries no event payload. It only wakes the stable ticker;
  // the UI thread later drains the source's own queue in source order.
  void installReadinessHandler(KeySource& source) {
    source.setReadinessHandler(
        [app = &app_]() { app->requestKeySourceTick(); });
  }

  static constexpr int kDrainBatchSize = 4;
  static constexpr int kMaxDrainBatches = 4;

  Application& app_;
  KeySource* head_ = nullptr;
};

/// Owns synchronous semantic-text delivery and all incoming emitter routes.
///
/// Emitter links are producer-owned, which lets application teardown detach
/// every surviving producer before task editors are destroyed.
class ApplicationTextInput {
 public:
  explicit ApplicationTextInput(Application& app) : app_(app) {}

  void connect(TextInputEmitter& emitter) {
    CHECK(emitter.destination_application_ == nullptr);
    CHECK(app_.state_ != Application::State::kStopping);
    if (app_.state_ != Application::State::kConstructed) {
      CHECK(app_.isUiThread());
    }
    emitter.destination_application_ = &app_;
    emitter.next_emitter_ = head_;
    head_ = &emitter;
  }

  void disconnect(TextInputEmitter& emitter) {
    if (emitter.destination_application_ == nullptr) return;
    CHECK(emitter.destination_application_ == &app_);
    if (app_.state_ != Application::State::kStopping) {
      CHECK(app_.isUiThread());
    }
    TextInputEmitter** link = &head_;
    while (*link != &emitter) {
      CHECK(*link != nullptr);
      link = &(*link)->next_emitter_;
    }
    *link = emitter.next_emitter_;
    emitter.destination_application_ = nullptr;
    emitter.next_emitter_ = nullptr;
  }

  void clear() {
    while (head_ != nullptr) disconnect(*head_);
    active_editor_ = nullptr;
  }

  void activate(TextFieldEditor& editor) {
    checkUiThread();
    if (active_editor_ == &editor) return;
    TextFieldEditor* old_editor = active_editor_;
    active_editor_ = nullptr;
    if (old_editor != nullptr) old_editor->cancel();
    active_editor_ = &editor;
  }

  void deactivate(TextFieldEditor& editor) {
    if (app_.state_ != Application::State::kStopping) checkUiThread();
    if (active_editor_ == &editor) active_editor_ = nullptr;
  }

  bool commitRune(uint32_t rune) {
    checkUiThread();
    if (active_editor_ == nullptr || !isUnicodeScalar(rune)) return false;
    active_editor_->rune(rune);
    return true;
  }

  bool deleteBackward() {
    checkUiThread();
    if (active_editor_ == nullptr) return false;
    active_editor_->del();
    return true;
  }

  bool performAction(TextInputAction action) {
    checkUiThread();
    if (active_editor_ == nullptr) return false;
    switch (action) {
      case TextInputAction::kDone:
        active_editor_->enter();
        return true;
    }
    return false;
  }

 private:
  static bool isUnicodeScalar(uint32_t rune) {
    return rune <= 0x10ffff && (rune < 0xd800 || rune > 0xdfff);
  }

  void checkUiThread() const {
    CHECK(app_.state_ != Application::State::kStopping);
    CHECK(app_.isUiThread());
  }

  Application& app_;
  TextInputEmitter* head_ = nullptr;
  TextFieldEditor* active_editor_ = nullptr;
};

/// Marks the interval in which Application::tick() owns UI dispatch.
///
/// The state prevents nested ticker execution and gives destruction a simple
/// fail-fast check while the callback is running.
class TickerGuard {
 public:
  /// Enters ticker dispatch for `app`; requires its UI thread.
  explicit TickerGuard(Application& app) : app_(app) {
    CHECK(app_.state_ == Application::State::kStarted);
    CHECK(roo::this_thread::get_id() == app_.ui_thread_id_);
    app_.state_ = Application::State::kTickerRunning;
  }

  /// Restores the started state after the dispatch callback returns.
  ~TickerGuard() { app_.state_ = Application::State::kStarted; }

 private:
  Application& app_;
};

Application::Application(const Environment* env, roo_display::Display& display)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      input_router_(new ApplicationInputRouter(*this)),
      text_input_(new ApplicationTextInput(*this)),
      window_(*this, display, true),
      ticker_(new ApplicationTicker(env->scheduler(), [this]() { tick(); })) {
  roo_display::Box keyboard_bounds(0, window_.root().height() / 2,
                                   window_.root().width() - 1,
                                   window_.root().height() - 1);
  auto keyboard_task = std::unique_ptr<Task>(
      new Task(*this, window_, keyboard_bounds, true, keyboard_.getContents()));
  keyboard_.setTask(*keyboard_task);
  keyboard_task->setVisible(false);
  tasks_.push_back(std::move(keyboard_task));
}

Application::Application(const Environment* env, roo_display::Display& display,
                         KeySource& keys, bool enable_touch)
    : env_(env),
      context_(env->scheduler(), env->theme(), env->keyboardColorTheme()),
      keyboard_(context_, kbEngUS()),
      input_router_(new ApplicationInputRouter(*this)),
      text_input_(new ApplicationTextInput(*this)),
      window_(*this, display, enable_touch),
      ticker_(new ApplicationTicker(env->scheduler(), [this]() { tick(); })) {
  roo_display::Box keyboard_bounds(0, window_.root().height() / 2,
                                   window_.root().width() - 1,
                                   window_.root().height() - 1);
  auto keyboard_task = std::unique_ptr<Task>(
      new Task(*this, window_, keyboard_bounds, true, keyboard_.getContents()));
  keyboard_.setTask(*keyboard_task);
  keyboard_task->setVisible(false);
  tasks_.push_back(std::move(keyboard_task));
  legacy_key_source_ = &keys;
  keys.connect(*tasks_.front());
}

Application::~Application() {
  if (state_ != State::kConstructed) {
    CHECK(roo::this_thread::get_id() == ui_thread_id_);
    CHECK(state_ != State::kTickerRunning);
  }
  state_ = State::kStopping;
  // Reject wakeups before clearing handlers, then remove all routes while
  // tasks and their fallback state are still valid.
  ticker_->stop();
  input_router_->clear();
  text_input_->clear();
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
  // Installing handlers queries queue state, so input enqueued before start
  // cannot wait for the periodic fallback to be observed.
  input_router_->start();
  window_.start();
  ticker_->requestNow();
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
  ticker_->requestAfter(delay);
}

bool Application::drainKeyEvents() {
  return input_router_->drainReadySources();
}

bool Application::refresh(roo_time::Uptime deadline) {
  checkUiThread();
  return window_.refresh(deadline);
}

Task& Application::addTask(Widget& content, const roo_display::Box& bounds) {
  checkUiThread();
  CHECK(content.parent() == nullptr);
  Task* task = new Task(*this, window_, bounds, false, content);
  tasks_.emplace_back(task);
  if (legacy_key_source_ != nullptr && legacy_key_source_->isConnected()) {
    legacy_key_source_->disconnect();
    legacy_key_source_->connect(*task);
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
  Task* task = new Task(*this, window_, bounds, false, navigation);
  tasks_.emplace_back(task);
  if (legacy_key_source_ != nullptr && legacy_key_source_->isConnected()) {
    legacy_key_source_->disconnect();
    legacy_key_source_->connect(*task);
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

void Application::connectKeySource(KeySource& source, Task& destination) {
  input_router_->connect(source, destination);
}

void Application::disconnectKeySource(KeySource& source) {
  input_router_->disconnect(source);
}

// Called by a source readiness handler. The ticker coalesces concurrent
// producer notifications and keeps dispatch on the application UI thread.
void Application::requestKeySourceTick() { ticker_->requestNow(); }

void Application::connectTextInputEmitter(TextInputEmitter& emitter) {
  text_input_->connect(emitter);
}

void Application::disconnectTextInputEmitter(TextInputEmitter& emitter) {
  text_input_->disconnect(emitter);
}

bool Application::commitTextInputRune(uint32_t rune) {
  return text_input_->commitRune(rune);
}

bool Application::deleteTextInputBackward() {
  return text_input_->deleteBackward();
}

bool Application::performTextInputAction(TextInputAction action) {
  return text_input_->performAction(action);
}

void Application::activateTextInput(TextFieldEditor& editor) {
  text_input_->activate(editor);
}

void Application::deactivateTextInput(TextFieldEditor& editor) {
  text_input_->deactivate(editor);
}

void Application::setTextEditorKeyboardListener(TextFieldEditor* editor,
                                                bool visible) {
  keyboard_.setListener(editor);
  if (visible) {
    keyboard_.show();
  } else {
    keyboard_.hide();
  }
}

namespace {

class SyncTask : public roo_scheduler::Executable {
 public:
  /// Creates one scheduler task that invokes `fn` and then releases `sem`.
  SyncTask(std::function<void()> fn, roo::binary_semaphore& sem)
      : fn_(std::move(fn)), sem_(sem) {}

  /// Runs the UI callback and unblocks its waiting caller.
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
