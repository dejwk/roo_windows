#pragma once

#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/dialogs/string_constants.h"

namespace roo_windows {

template <typename Model>
class RadioListDialog : public Dialog {
 public:
  RadioListDialog(const roo_windows::Environment& env)
      : Dialog(env, {kStrDialogCancel, kStrDialogOK}), list_(env) {
    init();
  }

  RadioListDialog(const roo_windows::Environment& env,
                  const typename Model::Element& prototype)
      : Dialog(env, {kStrDialogCancel, kStrDialogOK}),
        list_(env, prototype) {
    init();
  }

  int selected() const { return list_.selected(); }

  void setModel(Model& model) {
    list_.setModel(model);
    contents_.setContents(list_);
    last_button().setEnabled(false);
  }

  void reset() {
    list_.reset();
    last_button().setEnabled(false);
  }

  void setSelected(int selected) {
    list_.setSelected(selected);
    last_button().setEnabled(selected >= 0);
  }

  void contentsChanged() { list_.modelChanged(); }

  RadioListDialog(const roo_windows::Environment& env, Model& model)
      : RadioListDialog(env) {
    setModel(model);
  }

 protected:
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

  roo_windows::RadioList<Model> list_;
};

}  // namespace roo_windows
