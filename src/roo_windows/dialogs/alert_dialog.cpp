#include "roo_windows/dialogs/alert_dialog.h"

namespace roo_windows {

AlertDialog::AlertDialog(ApplicationContext& context, std::string title,
                         std::string supporting_text,
                         std::vector<std::string> button_labels)
    : Dialog(context, std::move(button_labels)),
      supporting_text_(context, std::move(supporting_text), font_body1(),
                       roo_display::kLeft | roo_display::kMiddle) {
  supporting_text_.setPadding(PaddingSize::kLarge, PaddingSize::kNone);
  supporting_text_.setMargins(MarginSize::kNone, MarginSize::kNone);
  const FrameworkColorScheme& colors = context.theme().framework.color;
  Color on_surface = colors.resolve(FrameworkColorRole::kContent);
  on_surface.set_a(0xB0);
  supporting_text_.setColor(
      AlphaBlend(colors.resolve(FrameworkColorRole::kSurface), on_surface));
  setTitle(std::move(title));
}

void AlertDialog::setSupportingText(std::string supporting_text) {
  supporting_text_.setContent(std::move(supporting_text));
}

void AlertDialog::onEnter() { setPresentationContent(supporting_text_); }

}  // namespace roo_windows
