#include "roo_windows/core/task.h"

#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/display_window.h"

namespace roo_windows {
namespace {
constexpr int kKeyDrainBatchSize = 4;
constexpr int kMaxKeyDrainBatchesPerTick = 4;

bool IsInSubtree(const Widget& candidate, const Widget& subtree) {
  for (const Widget* current = &candidate; current != nullptr;
       current = current->parent()) {
    if (current == &subtree) return true;
  }
  return false;
}
}  // namespace

Task::Task(Application& app, DisplayWindow& window,
               const roo_display::Box& bounds, bool popup, Keyboard& keyboard,
               Widget& content)
    : app_(app),
      window_(window),
      panel_(app.context(), *this),
      focus_(&panel_),
      editor_(app.env().scheduler(), keyboard),
      popup_(popup) {
  CHECK(content.parent() == nullptr);
  panel_.setContent(content, roo_display::Box(0, 0, -1, -1));
  if (popup) {
    window_.root().addPopup(panel_, bounds);
  } else {
    window_.root().addTask(panel_, bounds);
  }
}

Task::Task(Application& app, DisplayWindow& window,
               const roo_display::Box& bounds, bool popup, Keyboard& keyboard,
               NavigationHost& navigation)
    : app_(app),
      window_(window),
      panel_(app.context(), *this),
      focus_(&panel_),
      editor_(app.env().scheduler(), keyboard),
      popup_(popup),
      navigation_(&navigation) {
  navigation.install(*this);
  if (popup) {
    window_.root().addPopup(panel_, bounds);
  } else {
    window_.root().addTask(panel_, bounds);
  }
}

Task::~Task() {
  detachKeySource();
  back_callback_ = {};
  editor_.cancel();
  focus_.onSubtreeDetaching(panel_);
  if (navigation_ != nullptr) {
    navigation_->disconnect();
  } else if (panel_.content_ != nullptr) {
    panel_.clearContent();
  }
  if (popup_) {
    window_.root().removePopup(panel_);
  } else {
    window_.root().removeTask(panel_);
  }
}

void Task::setBackCallback(BackCallback callback) {
  back_callback_ = std::move(callback);
}

void Task::setVisible(bool visible) {
  panel_.setVisibility(visible ? Visibility::kVisible : Visibility::kGone);
}

KeySourceAttachmentResult Task::attachKeySource(KeySource& source) {
  if (!app_.isUiThread()) return KeySourceAttachmentResult::kWrongThread;
  if (source.attached_task_ != nullptr) {
    return KeySourceAttachmentResult::kSourceAlreadyAttached;
  }
  if (key_source_ != nullptr) {
    return KeySourceAttachmentResult::kTaskAlreadyHasSource;
  }
  source.attached_task_ = this;
  key_source_ = &source;
  return KeySourceAttachmentResult::kAttached;
}

void Task::detachKeySource() {
  if (key_source_ == nullptr) return;
  if (armed_key_widget_ != nullptr) onWidgetFocusLost(*armed_key_widget_);
  key_source_->attached_task_ = nullptr;
  key_source_ = nullptr;
}

void Task::onKeySourceDestroyed(KeySource& source) {
  if (key_source_ == &source) {
    if (armed_key_widget_ != nullptr) onWidgetFocusLost(*armed_key_widget_);
    key_source_ = nullptr;
  }
}

BackResult Task::requestBack(BackSource source) {
  if (window_.root().transient_presentation_slot().requestBack(source) ==
      BackResult::kHandled) {
    return BackResult::kHandled;
  }
  if (navigation_ != nullptr) return navigation_->requestBack(source);
  return requestTaskBackCallback(source);
}

void Task::attachNavigationContent(Widget& content) {
  CHECK(navigation_ != nullptr);
  panel_.setContent(content, roo_display::Box(0, 0, -1, -1));
}

void Task::detachNavigationContent() {
  CHECK(navigation_ != nullptr);
  panel_.clearContent();
}

BackResult Task::requestTaskBackCallback(BackSource source) {
  return back_callback_ == nullptr ? BackResult::kUnhandled
                                   : back_callback_(source);
}

Task::KeyDrainResult Task::drainOneKeyBatch() {
  if (key_source_ == nullptr) return {false, 0, false};
  KeyEvent events[kKeyDrainBatchSize];
  int count = key_source_->drain(events, kKeyDrainBatchSize);
  if (count < 0) count = 0;
  if (count > kKeyDrainBatchSize) count = kKeyDrainBatchSize;
  for (int i = 0; i < count; ++i) {
    dispatchKeyEvent(events[i]);
  }
  return {true, count, count == kKeyDrainBatchSize};
}

bool Task::drainKeyEvents() {
  for (int batch = 0; batch < kMaxKeyDrainBatchesPerTick; ++batch) {
    KeyDrainResult result = drainOneKeyBatch();
    if (!result.has_source || !result.source_may_have_more) return false;
  }
  return true;
}

void Task::dispatchKeyEvent(const KeyEvent& event) {
  Widget* focused = focus_.focused();
  // Application::add() remains a legacy structural path that does not create
  // a Task. Preserve its context-scoped keyboard routing until that API is
  // retired; attached UI tasks always use their local focus first.
  if (focused == nullptr) focused = app_.context().focus().focused();
  if (focused != nullptr) {
    if (focused->onKeyEvent(event)) return;
    for (Widget* ancestor = focused->parent();
         ancestor != nullptr && ancestor != &panel_;
         ancestor = ancestor->parent()) {
      if (ancestor->onKeyEvent(event)) return;
    }
  }
  if (event.phase == KeyPhase::kDown &&
      (event.code == KeyCode::kBack || event.code == KeyCode::kEscape)) {
    BackSource source = event.code == KeyCode::kBack ? BackSource::kBackKey
                                                     : BackSource::kEscapeKey;
    if (requestBack(source) == BackResult::kHandled) return;
  }
  if ((event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) &&
      event.code == KeyCode::kTab) {
    focus_.moveFocus(panel_, (event.modifiers & kKeyModifierShift) != 0);
    return;
  }
  if (event.phase == KeyPhase::kDown || event.phase == KeyPhase::kRepeat) {
    FocusDirection direction;
    bool is_directional = true;
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
        is_directional = false;
        break;
    }
    if (is_directional && focus_.moveFocusDirection(panel_, direction)) return;
  }
  if (event.code != KeyCode::kEnter && event.code != KeyCode::kSpace) return;
  if (focused == nullptr) return;
  if (event.phase == KeyPhase::kDown && focused->isClickable() &&
      focused->isEnabled()) {
    if (armed_key_widget_ != nullptr) onWidgetFocusLost(*armed_key_widget_);
    armed_key_widget_ = focused;
    armed_key_ = event.physical_key;
    focused->Widget::onShowPress(focused->width() / 2, focused->height() / 2);
  } else if (event.phase == KeyPhase::kUp && armed_key_widget_ == focused &&
             armed_key_ == event.physical_key) {
    armed_key_widget_ = nullptr;
    armed_key_ = PhysicalKey::kNone;
    focused->onSingleTapUp(focused->width() / 2, focused->height() / 2);
  }
}

void Task::onWidgetFocusLost(Widget& widget) {
  if (armed_key_widget_ != &widget) return;
  armed_key_widget_->setPressed(false);
  armed_key_widget_ = nullptr;
  armed_key_ = PhysicalKey::kNone;
}

void Task::onSubtreeDetaching(Widget& subtree) {
  if (armed_key_widget_ != nullptr &&
      IsInSubtree(*armed_key_widget_, subtree)) {
    onWidgetFocusLost(*armed_key_widget_);
  }
  if (editor_.targetInSubtree(subtree)) editor_.cancel();
  focus_.onSubtreeDetaching(subtree);
}

}  // namespace roo_windows
