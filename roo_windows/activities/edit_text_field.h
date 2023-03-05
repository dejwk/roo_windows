#pragma once

#include <functional>

#include "roo_windows/activities/keyboard.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

class EditTextField;

class EditedTextField : public TextField {
 public:
  EditedTextField(const Environment& env, TextFieldEditor& editor,
                  const std::string& hint, EditTextField& activity);

  void onEditFinished(bool confirmed) override;

 private:
  EditTextField& activity_;
};

class EditTextField : public Activity {
 public:
  EditTextField(const Environment& env, TextFieldEditor& editor,
                const std::string& hint);

  Widget& getContents() override { return main_pane_; }

  // Launches a text-enter activity with the specified conditions, triggering
  // the specified function on confirmation.
  void triggerEdit(Task& task, const std::string& initial,
                   const std::string& hint,
                   std::function<void(const std::string&)> enter_fn);

  // Launches a text-enter activity to edit the text in the specified field.
  // Typical usage:
  // field.addOnClicked([&]() { enter_text.triggerEditField(field); });
  void triggerEditField(TextField& field);

 private:
  friend class EditedTextField;

  void confirm();
  void cancel();

  VerticalLayout main_pane_;
  HorizontalLayout content_pane_;
  SimpleButton back_;
  EditedTextField text_;
  SimpleButton enter_;
  bool editing_;
  std::function<void(const std::string&)> enter_fn_;
};

}  // namespace roo_windows
