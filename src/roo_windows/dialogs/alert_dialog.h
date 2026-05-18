#pragma once

#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/dialogs/string_constants.h"
#include "roo_windows/widgets/text_block.h"

namespace roo_windows {

/// Simple message dialog: a title, a multi-line supporting text body, and
/// one or more dismiss buttons (defaulting to a single "OK").
///
/// Use for confirmations, warnings, and notifications where no additional
/// input is required. The selected button index is reported back through the
/// `Dialog`'s callback.
class AlertDialog : public Dialog {
 public:
  AlertDialog(const Environment& env, std::string title,
              std::string supporting_text,
              std::vector<std::string> button_labels = {kStrDialogOK});

  void setSupportingText(std::string supporting_text);

 private:
  TextBlock supporting_text_;
};

}  // namespace roo_windows
