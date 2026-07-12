#include "dialog.h"

#include "roo_windows/widgets/button.h"

namespace roo_windows {

Dialog::Dialog(ApplicationContext& context, std::vector<std::string> button_labels)
    : VerticalLayout(context),
      title_(context, "", font_h6()),
      divider1_(context),
      contents_(context),
      divider2_(context),
      title_panel_(context),
      button_panel_(context),
      callback_fn_(nullptr) {
  title_panel_.setMargins(Margins(MarginSize::kNone, MarginSize::kRegular));
  add(title_panel_);
  title_.setPadding(PaddingSize::kLarge, PaddingSize::kSmall);
  title_.setMargins(MarginSize::kNone, MarginSize::kNone);
  title_panel_.add(title_);
  setDividersVisible(false);
  add(divider1_);
  contents_.setVerticalScrollBarPresence(
      VerticalScrollBar::Presence::kShownWhenScrolling);
  add(contents_, {weight : 1});
  add(divider2_);
  button_panel_.setPadding(PaddingSize::kTiny);
  button_panel_.setMargins(Margins(MarginSize::kNone, MarginSize::kSmall));
  add(button_panel_, {gravity : kGravityRight});
  button_panel_.setGravity(kGravityRight | kGravityMiddle);
  buttons_.reserve(button_labels.size());
  int i = 0;
  for (std::string& label : button_labels) {
    buttons_.emplace_back(context, std::move(label), Button::TEXT);
    buttons_.back().setOnInteractiveChange([this, i]() { actionTaken(i); });
    button_panel_.add(buttons_.back());
    ++i;
  }
}

void Dialog::setTitle(std::string title) { title_.setText(std::move(title)); }

void Dialog::actionTaken(int idx) {
  if (callback_fn_ != nullptr) {
    callback_fn_(idx);
  }
}

void Dialog::close() {
  if (callback_fn_ != nullptr) {
    callback_fn_(-1);
  }
}

}  // namespace roo_windows
