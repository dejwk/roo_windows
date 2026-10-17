#include "roo_windows/config.h"

#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD && ROO_WINDOWS_ENABLE_MATERIAL2
#include "roo_icons.h"
#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/task.h"
#include "roo_windows/internal/editor_destination_factory.h"
#include "roo_windows/material2/editor_destination/editor_destination.h"
#include "roo_windows/material2/typography.h"

namespace roo_windows::material2 {

EditorDestination::EditedTextField::EditedTextField(ApplicationContext& context,
                                                    EditorDestination& owner)
    : TextField(context, font_body1(), "",
                roo_display::kLeft | roo_display::kMiddle, UNDERLINE),
      owner_(owner) {}

void EditorDestination::EditedTextField::onEditFinished(bool confirmed) {
  if (confirmed) owner_.confirm();
}

EditorDestination::EditorDestination(ApplicationContext& context,
                                     const std::string& hint)
    : back_(context, SCALED_ROO_ICON(outlined, navigation_arrow_back),
            Button::TEXT),
      text_(context, *this),
      clear_(context, SCALED_ROO_ICON(outlined, navigation_close),
             Button::TEXT),
      content_pane_(context),
      main_pane_(context) {
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
  text_.setContent(initial);
  text_.setHint(hint);
  text_.setStarred(obscure);
  updateClearVisibility();
}

const std::string& EditorDestination::draft() const { return text_.content(); }
void EditorDestination::clearDraft() {
  text_.setContent("");
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

}  // namespace roo_windows::material2

namespace roo_windows::internal {
std::unique_ptr<::roo_windows::EditorDestination>
CreateMaterial2EditorDestination(ApplicationContext& context) {
  return std::make_unique<material2::EditorDestination>(context);
}
}  // namespace roo_windows::internal
#endif
