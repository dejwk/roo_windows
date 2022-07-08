#include "roo_windows/activities/edit_text_field.h"

#include <inttypes.h>

#include "roo_smooth_fonts/NotoSans_Regular/18.h"

namespace roo_windows {

EditedTextField::EditedTextField(const Environment& env, TextFieldEditor& editor, const std::string& hint, EditTextField& activity)
    : TextField(env, editor, roo_display::font_NotoSans_Regular_18(), hint,
            roo_display::HAlign::Left(), roo_display::VAlign::Middle()),
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
      back_(env, ic_outlined_24_navigation_arrow_back(), Button::TEXT),
      text_pane_(env),
      text_(env, editor, hint, *this),
      divider_(env),
      enter_(env, ic_outlined_24_navigation_check()),
      editing_(false),
      enter_fn_(nullptr) {
  main_pane_.add(content_pane_, VerticalLayout::Params());
  content_pane_.add(
      back_, HorizontalLayout::Params().setGravity(kVerticalGravityMiddle));
  content_pane_.add(text_pane_, HorizontalLayout::Params().setWeight(1));
  text_pane_.add(text_, VerticalLayout::Params());
  text_pane_.add(divider_, VerticalLayout::Params());
  content_pane_.add(
      enter_, HorizontalLayout::Params().setGravity(kVerticalGravityMiddle));
  back_.setTextColor(env.theme().color.onSurface);
  back_.setOnClicked([&]() { cancel(); });
  enter_.setOnClicked([&]() { confirm(); });
}

void EditTextField::confirm() {
  if (!editing_) return;
  editing_ = false;
  text_.editor().edit(nullptr);
  enter_fn_(text_.content());
  enter_fn_ = nullptr;
  back_.getTask()->popActivity();
}

void EditTextField::cancel() {
  if (!editing_) return;
  editing_ = false;
  text_.editor().edit(nullptr);
  enter_fn_ = nullptr;
  back_.getTask()->popActivity();
}

void EditTextField::triggerEdit(
    Task& task, const std::string& initial, const std::string& hint,
    std::function<void(const std::string&)> enter_fn) {
  editing_ = true;
  text_.setContent(initial);
  text_.edit();
  task.pushActivity(this);
  enter_fn_ = enter_fn;
}

void EditTextField::triggerEditField(TextField& field) {
  triggerEdit(*field.getTask(), field.content(), field.hint(),
              [&](const std::string& value) { field.setContent(value); });
}

}  // namespace roo_windows