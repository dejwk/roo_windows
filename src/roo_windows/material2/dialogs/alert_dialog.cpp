#include "roo_windows/material2/dialogs/alert_dialog.h"

#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material2/typography.h"

namespace roo_windows::material2 {

AlertDialog::AlertDialog(ApplicationContext& context, std::string title,
                         std::string supporting_text,
                         std::vector<std::string> button_labels)
    : Dialog(context, std::move(button_labels)),
      supporting_text_(context, std::move(supporting_text),
                       material2::text_style_body1(),
                       roo_display::kLeft | roo_display::kMiddle) {
  supporting_text_.setPadding(PaddingSize::kLarge, PaddingSize::kNone);
  setTitle(std::move(title));
}

AlertDialog::~AlertDialog() { prepareForDerivedDestruction(); }

void AlertDialog::setSupportingText(std::string supporting_text) {
  supporting_text_.setContent(std::move(supporting_text));
}

bool AlertDialog::onEnter() {
  setPresentationContent(supporting_text_);
  return true;
}

PresentationStartResult ShowAlertDialog(Task& interaction_owner,
                                        std::string title,
                                        std::string supporting_text,
                                        std::vector<std::string> button_labels,
                                        Dialog::CallbackFn callback_fn) {
  Application& application = interaction_owner.application();
  CHECK(application.isUiThread());
  Dialog* dialog =
      new AlertDialog(application.context(), std::move(title),
                      std::move(supporting_text), std::move(button_labels));
  PresentationStartResult result =
      dialog->show(interaction_owner,
                   [dialog, callback_fn = std::move(callback_fn)](int id) {
                     if (callback_fn != nullptr) {
                       callback_fn(id);
                     }
                     delete dialog;
                   });
  if (result != PresentationStartResult::kStarted) delete dialog;
  return result;
}

}  // namespace roo_windows::material2
