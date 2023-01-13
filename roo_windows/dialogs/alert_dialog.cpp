#include "roo_windows/dialogs/alert_dialog.h"

namespace roo_windows {

AlertDialog::AlertDialog(const Environment& env, std::string title,
                         std::string supporting_text,
                         std::vector<std::string> button_labels)
    : Dialog(env, std::move(button_labels)),
      supporting_text_(env, std::move(supporting_text), *env.theme().font.body1,
                       roo_display::kLeft | roo_display::kMiddle) {
  supporting_text_.setPadding(PADDING_NONE, PADDING_NONE);
  Color on_surface = env.theme().color.onSurface;
  on_surface.set_a(0xB0);
  supporting_text_.setColor(alphaBlend(env.theme().color.surface, on_surface));
  contents_.setContents(supporting_text_);
  setTitle(std::move(title));
}

void AlertDialog::setSupportingText(std::string supporting_text) {
  supporting_text_.setContent(std::move(supporting_text));
}

}  // namespace roo_windows