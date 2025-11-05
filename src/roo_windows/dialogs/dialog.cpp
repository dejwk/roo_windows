#include "dialog.h"

#include "roo_windows/widgets/button.h"

namespace roo_windows {

namespace {

class Scrim : public roo_display::Rasterizable {
 public:
  void readColors(const int16_t* x, const int16_t* y, uint32_t count,
                  Color* result) const override {
    FillColor(result, count, Color(0x80000000));
  }

  bool readColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
                     Color* result) const override {
    *result = Color(0x80000000);
    return true;
  }

  roo_display::Box extents() const override {
    return roo_display::Box::MaximumBox();
  }
};

const Scrim& scrim() {
  static Scrim scrim;
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
  title_panel_.setMargins(Margins(MARGIN_NONE, MARGIN_REGULAR));
  add(title_panel_);
  title_.setPadding(PADDING_LARGE, PADDING_SMALL);
  title_.setMargins(MARGIN_NONE, MARGIN_NONE);
  title_panel_.add(title_);
  setDividersVisible(false);
  add(divider1_);
  contents_.setVerticalScrollBarPresence(
      VerticalScrollBar::SHOWN_WHEN_SCROLLING);
  add(contents_, {weight : 1});
  add(divider2_);
  button_panel_.setPadding(PADDING_TINY);
  button_panel_.setMargins(Margins(MARGIN_NONE, MARGIN_SMALL));
  add(button_panel_, {gravity : kGravityRight});
  button_panel_.setGravity(Gravity(kGravityRight, kGravityMiddle));
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