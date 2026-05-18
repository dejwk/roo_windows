#pragma once

#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/dialogs/string_constants.h"

namespace roo_windows {

/// Dialog that wraps a `RadioList` so the user can pick exactly one item from
/// a model.
///
/// The Cancel / OK footer is wired so that OK is enabled only when something
/// is selected; the chosen index is read via `selected()` from within the
/// callback. Override `onChange()` to react to interactive selection changes.
class RadioListDialog : public Dialog {
 public:
  RadioListDialog(const roo_windows::Environment& env,
                  std::function<std::unique_ptr<Widget>()> prototype_fn)
      : Dialog(env, {kStrDialogCancel, kStrDialogOK}),
        list_(env, prototype_fn) {
    init();
  }

  /// Returns the currently selected index in the wrapped `RadioList`.
  int selected() const { return list_.selected(); }

  /// Binds the supplied item model and disables the OK button until an item
  /// is selected.
  void setModel(ListModel& model) {
    list_.setModel(model);
    contents_.setContents(list_);
    last_button().setEnabled(false);
  }

  /// Disallowed: callers must bind their own `ListModel`, not the internal
  /// `RadioListModel` adapter.
  void setModel(RadioListModel& model) = delete;

  /// Clears the selection and disables the OK button.
  void reset() {
    list_.reset();
    last_button().setEnabled(false);
  }

  /// Programmatically updates the selected index; toggles the OK button's
  /// enabled state accordingly.
  void setSelected(int selected) {
    list_.setSelected(selected);
    last_button().setEnabled(selected >= 0);
  }

  /// Notifies the list that its bound model's contents have changed.
  void contentsChanged() { list_.modelChanged(); }

 protected:
  /// Hook fired on interactive selection changes; default implementation
  /// re-emits the panel-level interactive-change signal.
  virtual void onChange() { triggerInteractiveChange(); }

 private:
  void init() {
    setDividersVisible(true);
    last_button().setEnabled(false);
    list_.setOnInteractiveChange([this]() {
      last_button().setEnabled(selected() >= 0);
      onChange();
    });
  }

  roo_windows::RadioList list_;
};

}  // namespace roo_windows
