#pragma once

#include <functional>

#include "roo_material_icons/outlined/24/navigation.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

class EditTextField : public Activity {
 public:
  EditTextField(const Environment& env, TextFieldEditor& editor,
                const std::string& hint, Keyboard& kb);

  Widget& getContents() override { return main_pane_; }

  void triggerEdit(Task& task, const std::string& initial,
                   const std::string& hint,
                   std::function<void(const std::string&)> enter_fn);

  // Opens up a text-enter activity to edit the text in the specified field.
  // Typical usage:
  // field.addOnClicked([&]() { enter_text.triggerEditField(field); });
  void triggerEditField(TextField& field);

 private:
  class Listener : public KeyboardListener {
   public:
    Listener(EditTextField& activity) : activity_(activity) {}

    void rune(uint32_t rune) override;
    void enter() override;
    void del() override;

   private:
    EditTextField& activity_;
  };

  friend class Listener;

  void confirm();
  void cancel();

  Keyboard& kb_;
  VerticalLayout main_pane_;
  HorizontalLayout content_pane_;
  Button back_;
  VerticalLayout text_pane_;
  TextField text_;
  HorizontalDivider divider_;
  Button enter_;
  Listener listener_;
  std::function<void(const std::string&)> enter_fn_;
};

}  // namespace roo_windows
