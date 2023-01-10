#pragma once

#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/widgets/text_block.h"

namespace roo_windows {

class AlertDialog : public Dialog {
 public:
  // Supporting text must currently explicitly use newline characters to break
  // the text.
  AlertDialog(const Environment& env, std::string supporting_text,
              const std::initializer_list<std::string>& button_labels);

  void setSupportingText(std::string supporting_text);

 private:
  TextBlock supporting_text_;
};

}  // namespace roo_windows
