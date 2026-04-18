#include "dialog.h"

#include "roo_windows/widgets/button.h"

namespace roo_windows {

namespace {

const roo_display::Rasterizable& scrim() {
  static roo_display::Scrim scrim;
  return scrim;
}

}  // namespace

Dialog::Dialog(const Environment& env, std::vector<std::string> button_labels)
    : VerticalLayout(env),
      title_(env, "", font_h6()),
      divider1_(env),
      contents_(env),
      divider2_(env),
      title_panel_(env),
      button_panel_(env),
      callback_fn_(nullptr) {
  title_panel_.setMargins(Margins(MarginSize::NONE, MarginSize::REGULAR));
  add(title_panel_);
  title_.setPadding(PaddingSize::LARGE, PaddingSize::SMALL);
  title_.setMargins(MarginSize::NONE, MarginSize::NONE);
  title_panel_.add(title_);
  setDividersVisible(false);
  add(divider1_);
  contents_.setVerticalScrollBarPresence(
      VerticalScrollBar::SHOWN_WHEN_SCROLLING);
  add(contents_, {weight : 1});
  add(divider2_);
  button_panel_.setPadding(PaddingSize::TINY);
  button_panel_.setMargins(Margins(MarginSize::NONE, MarginSize::SMALL));
  add(button_panel_, {gravity : kGravityRight});
  button_panel_.setGravity(kGravityRight | kGravityMiddle);
  buttons_.reserve(button_labels.size());
  int i = 0;
  for (std::string& label : button_labels) {
    buttons_.emplace_back(env, std::move(label), Button::TEXT);
    buttons_.back().setOnInteractiveChange([this, i]() { actionTaken(i); });
    button_panel_.add(buttons_.back());
    ++i;
  }
}

void Dialog::setTitle(std::string title) { title_.setText(std::move(title)); }

Widget* Dialog::dispatchTouchDownEvent(XDim x, YDim y) {
  Widget* result = Panel::dispatchTouchDownEvent(x, y);
  return result == nullptr ? this : result;
}

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

void Dialog::finalizePaintWidget(const Canvas& canvas, Clipper& clipper,
                                 const OverlaySpec& overlay_spec) const {
  Panel::finalizePaintWidget(canvas, clipper, overlay_spec);
  clipper.addOverlay(&scrim(), canvas.clip_box());
}

}  // namespace roo_windows