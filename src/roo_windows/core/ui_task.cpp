#include "roo_windows/core/ui_task.h"

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

UiTask::UiTask(Application& app, DisplayWindow& window,
               const roo_display::Box& bounds, bool popup, Keyboard& keyboard)
    : app_(app),
      window_(window),
      panel_(app.context(), *this, legacy_task_),
      focus_(&panel_),
      editor_(app.env().scheduler(), keyboard),
      popup_(popup) {
  legacy_task_.init(&panel_);
  if (popup) {
    window_.root().addPopup(panel_, bounds);
  } else {
    window_.root().addTask(panel_, bounds);
  }
}

UiTask::~UiTask() {
  detachKeySource();
  editor_.cancel();
  focus_.onSubtreeDetaching(panel_);
  legacy_task_.clear();
  if (popup_) {
    window_.root().removePopup(panel_);
  } else {
    window_.root().removeTask(panel_);
  }
}

KeySourceAttachmentResult UiTask::attachKeySource(KeySource& source) {
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

void UiTask::detachKeySource() {
  if (key_source_ == nullptr) return;
  key_source_->attached_task_ = nullptr;
  key_source_ = nullptr;
}

void UiTask::onKeySourceDestroyed(KeySource& source) {
  if (key_source_ == &source) key_source_ = nullptr;
}

BackResult UiTask::requestBack(BackSource source) {
  if (window_.root().transient_presentation_slot().requestBack(source) ==
      BackResult::kHandled) {
    return BackResult::kHandled;
  }
  return legacy_task_.requestBack(source);
}

bool UiTask::drainKeyEvents() {
  if (key_source_ == nullptr) return false;
  KeyEvent events[kKeyDrainBatchSize];
  for (int batch = 0; batch < kMaxKeyDrainBatchesPerTick; ++batch) {
    int count = key_source_->drain(events, kKeyDrainBatchSize);
    if (count <= 0) return false;
    if (count > kKeyDrainBatchSize) count = kKeyDrainBatchSize;
    for (int i = 0; i < count; ++i) dispatchKeyEvent(events[i]);
    if (count < kKeyDrainBatchSize) return false;
  }
  return true;
}

void UiTask::dispatchKeyEvent(const KeyEvent& event) {
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
  Widget* focused = focus_.focused();
  // Application::add() remains a legacy structural path that does not create
  // a UiTask. Preserve its context-scoped keyboard routing until that API is
  // retired; attached UI tasks always use their local focus first.
  if (focused == nullptr) focused = app_.context().focus().focused();
  if (focused == nullptr) return;
  if (focused->onKeyEvent(event)) return;
  for (Widget* ancestor = focused->parent();
       ancestor != nullptr && ancestor != &panel_;
       ancestor = ancestor->parent()) {
    if (ancestor->onKeyEvent(event)) return;
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
  if (event.phase == KeyPhase::kDown && focused->isClickable() &&
      focused->isEnabled()) {
    armed_key_widget_ = focused;
    armed_key_ = event.code;
    focused->Widget::onShowPress(focused->width() / 2, focused->height() / 2);
  } else if (event.phase == KeyPhase::kUp && armed_key_widget_ == focused &&
             armed_key_ == event.code) {
    armed_key_widget_ = nullptr;
    armed_key_ = KeyCode::kUnknown;
    focused->onSingleTapUp(focused->width() / 2, focused->height() / 2);
  }
}

void UiTask::onSubtreeDetaching(Widget& subtree) {
  if (armed_key_widget_ != nullptr &&
      IsInSubtree(*armed_key_widget_, subtree)) {
    armed_key_widget_ = nullptr;
    armed_key_ = KeyCode::kUnknown;
  }
  if (editor_.targetInSubtree(subtree)) editor_.cancel();
  focus_.onSubtreeDetaching(subtree);
}

}  // namespace roo_windows
