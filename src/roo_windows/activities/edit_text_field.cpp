#include "roo_windows/activities/edit_text_field.h"

#include <inttypes.h>

#include "roo_icons.h"
#include "roo_icons/outlined/navigation.h"
#include "roo_windows/config.h"

namespace roo_windows {

EditedTextField::EditedTextField(const Environment& env,
                                 TextFieldEditor& editor,
                                 const std::string& hint,
                                 EditTextField& activity)
    : TextField(env, editor, roo_windows::font_body1(), hint,
                roo_display::kLeft | roo_display::kMiddle, UNDERLINE),
      activity_(activity) {}

void EditedTextField::onEditFinished(bool confirmed) {
  TextField::onEditFinished(confirmed);
  if (confirmed) {
    // Triggered by a direct click on the 'Enter' button.
    activity_.confirm();
  }
}

EditTextField::EditTextField(const Environment& env, TextFieldEditor& editor,
                             const std::string& hint)
    : main_pane_(env),
      content_pane_(env),
      back_(env, SCALED_ROO_ICON(outlined, navigation_arrow_back),
            Button::TEXT),
      text_(env, editor, hint, *this),
      enter_(env, SCALED_ROO_ICON(outlined, navigation_check)),
      editing_(false),
      enter_fn_(nullptr) {
  main_pane_.add(content_pane_, VerticalLayout::Params());
  content_pane_.add(
      back_, HorizontalLayout::Params().setGravity(kVerticalGravityMiddle));
  content_pane_.add(text_, HorizontalLayout::Params().setWeight(1).setGravity(
                               kVerticalGravityMiddle));
  content_pane_.add(
      enter_, HorizontalLayout::Params().setGravity(kVerticalGravityMiddle));
  back_.setContentColor(env.theme().color.onSurface);
  back_.setOnInteractiveChange([&]() { cancel(); });
  enter_.setOnInteractiveChange([&]() { confirm(); });
}

void EditTextField::confirm() {
  if (!editing_) return;
  editing_ = false;
  text_.editor().edit(nullptr);
  enter_fn_(text_.content());
  enter_fn_ = nullptr;
  exit();
}

void EditTextField::cancel() {
  if (!editing_) return;
  editing_ = false;
  text_.editor().edit(nullptr);
  enter_fn_ = nullptr;
  exit();
}

void EditTextField::triggerEdit(
    Task& task, const std::string& initial, const std::string& hint,
    std::function<void(const std::string&)> enter_fn) {
  editing_ = true;
  text_.setContent(initial);
  text_.edit();
  task.enterActivity(this);
  enter_fn_ = enter_fn;
}

void EditTextField::triggerEditField(TextField& field) {
  triggerEdit(*field.getTask(), field.content(), field.hint(),
              [&](const std::string& value) { field.setContent(value); });
}

}  // namespace roo_windows