#include "roo_windows/config.h"

#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
#include "roo_icons.h"
#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/internal/editor_destination_factory.h"
#include "roo_windows/material3/editor_destination/editor_destination.h"

namespace roo_windows::material3 {

EditorDestination::EditedTextField::EditedTextField(ApplicationContext& context,
                                                    EditorDestination& owner)
    : TextField(context, "", TextFieldVariant::kFilled), owner_(owner) {}

void EditorDestination::EditedTextField::onEditFinished(bool confirmed) {
  if (confirmed) owner_.confirm();
}

void EditorDestination::EditedTextField::edit() { startEditing(true); }

EditorDestination::EditorDestination(ApplicationContext& context,
                                     const std::string& hint)
    : back_(context, SCALED_ROO_ICON(outlined, navigation_arrow_back),
            IconButtonStyle::kStandard),
      text_(context, *this),
      clear_(context, SCALED_ROO_ICON(outlined, navigation_close),
             IconButtonStyle::kStandard),
      content_pane_(context),
      main_pane_(context) {
  back_.setShapeMorph(ButtonShapeMorph::kDisabled);
  clear_.setShapeMorph(ButtonShapeMorph::kDisabled);
  prepareDraft("", hint, false);
  main_pane_.add(content_pane_, VerticalLayout::Params());
  content_pane_.add(back_, {gravity : kGravityMiddle});
  content_pane_.add(text_, {gravity : kGravityMiddle, weight : 1});
  content_pane_.add(clear_, {gravity : kGravityMiddle});
  back_.setOnInteractiveChange(
      [this]() { getTask()->requestBack(BackSource::kNavigationButton); });
  clear_.setOnInteractiveChange([this]() {
    clearDraft();
    editInPlace();
  });
}

void EditorDestination::prepareDraft(const std::string& initial,
                                     const std::string& hint, bool obscure) {
  hint_ = hint;
  text_.setLabel(hint_);
  text_.obscure = obscure;
  text_.setText(initial);
  updateClearVisibility();
}

const std::string& EditorDestination::draft() const { return text_.text(); }
void EditorDestination::clearDraft() {
  text_.setText("");
  updateClearVisibility();
}

void EditorDestination::updateClearVisibility() {
  clear_.setVisibility(draft().empty() ? Visibility::kInvisible
                                       : Visibility::kVisible);
}

void EditorDestination::EditedTextField::notifyTextChanged() {
  owner_.updateClearVisibility();
}
void EditorDestination::editInPlace() { text_.edit(); }

}  // namespace roo_windows::material3

namespace roo_windows::internal {
std::unique_ptr<::roo_windows::EditorDestination>
CreateMaterial3EditorDestination(ApplicationContext& context) {
  return std::make_unique<material3::EditorDestination>(context);
}
}  // namespace roo_windows::internal
#endif
