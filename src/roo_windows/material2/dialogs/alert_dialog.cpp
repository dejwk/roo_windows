#include "roo_windows/material2/dialogs/alert_dialog.h"

namespace roo_windows::material2 {

AlertDialog::AlertDialog(ApplicationContext& context, std::string title,
                         std::string supporting_text,
                         std::vector<std::string> button_labels)
    : Dialog(context, std::move(button_labels)),
      supporting_text_(context, std::move(supporting_text),
                       material2::text_style_body1(),
                       roo_display::kLeft | roo_display::kMiddle) {
  supporting_text_.setPadding(PaddingSize::kLarge, PaddingSize::kNone);
  const Theme& theme = context.theme();
  Color on_surface = theme.frameworkColor(FrameworkColorRole::kContent);
  on_surface.set_a(0xB0);
  supporting_text_.setColor(AlphaBlend(
      theme.frameworkColor(FrameworkColorRole::kSurface), on_surface));
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

}  // namespace roo_windows::material2
