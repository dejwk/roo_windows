#include "roo_windows/config.h"

#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
#include "roo_windows/core/application.h"
#include "roo_windows/core/editor_destination.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

HeightSpec EditorDestination::AvailableHeight(const Widget& pane,
                                              HeightSpec height) {
  const Task* task = pane.getTask();
  if (task == nullptr || pane.parent() == nullptr) return height;
  const Widget* keyboard =
      task->application().keyboard().getContents().parent();
  if (keyboard == nullptr) return height;
  XDim x;
  YDim top;
  pane.parent()->getAbsoluteOffset(x, top);
  YDim keyboard_top;
  keyboard->getAbsoluteOffset(x, keyboard_top);
  return HeightSpec::AtMost(
      std::max<int>(0, std::min<int>(height.value(), keyboard_top - top)));
}

bool EditorDestination::canStart(NavigationHost& navigation) {
  ApplicationContext& owner = getContents().context();
  Task* task = navigation.getTask();
  if (task == nullptr || !navigation.isAvailable()) return false;
  CHECK(&task->application().context() == &owner);
  CHECK(task->application().isUiThread());
  const EditorDestination* installed = owner.editorDestinationIfPresent();
  return !busy() && (installed == nullptr || !installed->busy());
}

bool EditorDestination::triggerEdit(
    NavigationHost& navigation, const std::string& initial,
    const std::string& hint, std::function<void(const std::string&)> enter_fn,
    bool obscure) {
  if (!canStart(navigation)) return false;
  editing_ = true;
  confirmed_ = false;
  prepareDraft(initial, hint, obscure);
  enter_fn_ = std::move(enter_fn);
  navigation.push(*this);
  if (getNavigationHost() != nullptr) return true;
  editing_ = false;
  source_ = nullptr;
  clearDraft();
  enter_fn_ = nullptr;
  return false;
}

BackResult EditorDestination::onBackRequested(BackSource source) {
  if (!editing_) return BackResult::kUnhandled;
  exit();
  return BackResult::kHandled;
}

void EditorDestination::onResume() {
  if (editing_) editInPlace();
}

void EditorDestination::confirm() {
  if (!editing_) return;
  confirmed_ = true;
  exit();
}

void EditorDestination::onRemoved() {
  editing_ = false;
  completing_ = true;
  bool confirmed = confirmed_;
  confirmed_ = false;
  std::function<void(const std::string&)> callback = std::move(enter_fn_);
  std::string value = draft();
  clearDraft();
  finishSource(confirmed, value);
  completing_ = false;
  // Preserve detached callback reentrancy and self-destruction: no member
  // access is allowed after invoking user completion.
  if (confirmed && callback != nullptr) callback(value);
}

bool EditorDestination::triggerEditField(internal::TextEditTarget& source,
                                         const std::string& hint) {
  Task* task = source.editWidget().getTask();
  if (task == nullptr || !canStart(task->navigation())) return false;
  source_ = &source;
  return triggerEdit(task->navigation(), source.textBuffer(), hint, nullptr,
                     source.obscureText());
}

void EditorDestination::forgetSource(internal::TextEditTarget& source) {
  if (source_ == &source) source_ = nullptr;
}

void EditorDestination::finishSource(bool confirmed, const std::string& value) {
  if (source_ == nullptr) return;
  if (confirmed && source_->textBuffer() != value) {
    source_->textBuffer() = value;
    Task* task = source_->editWidget().getTask();
    if (task != nullptr && task->textFieldEditor().isEdited(source_)) {
      task->textFieldEditor().resetMetrics();
    }
    source_->notifyEditVisualChange();
    source_->notifyTextChanged();
  }
  // A value-change callback can destroy the source and clear the borrow.
  internal::TextEditTarget* source = source_;
  source_ = nullptr;
  if (source != nullptr) source->onEditFinished(confirmed);
}

}  // namespace roo_windows
#endif
