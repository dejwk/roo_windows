#pragma once

#include "roo_windows/core/padding_mixin.h"
#include "roo_windows/material2/dialogs/dialog.h"
#include "roo_windows/material2/dialogs/string_constants.h"
#include "roo_windows/widgets/text_block.h"

namespace roo_windows::material2 {

/// Simple message dialog: a title, a multi-line supporting text body, and
/// one or more dismiss buttons (defaulting to a single "OK").
///
/// Use for confirmations, warnings, and notifications where no additional
/// input is required. The selected button index is reported back through the
/// `Dialog`'s callback.
class AlertDialog : public Dialog {
 public:
  /// Builds an alert dialog with the supplied title, body, and footer
  /// buttons. The default button set is a single OK.
  AlertDialog(ApplicationContext& context, std::string title,
              std::string supporting_text,
              std::vector<std::string> button_labels = {kStrDialogOK});

  /// Detaches the borrowed supporting text before its storage is destroyed.
  ~AlertDialog() override;

  /// Replaces the multi-line supporting text.
  void setSupportingText(std::string supporting_text);

 protected:
  bool onEnter() override;

 private:
  class SupportingText : public PaddingMixin<TextBlock> {
   public:
    using PaddingMixin<TextBlock>::PaddingMixin;
    roo_display::Color defaultColor() const override {
      const auto& color = theme().material2Theme().color;
      return AlphaBlend(color.surface, color.onSurface.withA(0xB0));
    }
  };

  SupportingText supporting_text_;
};

/// Shows a heap-owned alert through @p interaction_owner's application.
/// Call on its UI thread. Completion receives the selected button index, or
/// -1 on dismissal/teardown, after detachment. The dialog is deleted after the
/// callback, or immediately if admission fails (without calling the callback).
/// Title, supporting text, button labels, and callback are copied/moved into
/// the presentation. Returns its admission result; no dialog pointer escapes.
PresentationStartResult ShowAlertDialog(Task& interaction_owner,
                                        std::string title,
                                        std::string supporting_text,
                                        std::vector<std::string> button_labels,
                                        Dialog::CallbackFn callback_fn);

}  // namespace roo_windows::material2
