#include "dialog.h"

#include "roo_windows/widgets/button.h"

namespace roo_windows {

namespace {

class Scrim : public roo_display::Rasterizable {
 public:
  void ReadColors(const int16_t* x, const int16_t* y, uint32_t count,
                  Color* result) const override {
    Color::Fill(result, count, Color(0x80000000));
  }

  bool ReadColorRect(int16_t xMin, int16_t yMin, int16_t xMax, int16_t yMax,
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

Dialog::Dialog(const Environment& env,
               const std::initializer_list<std::string>& button_labels)
    : VerticalLayout(env),
      body_(env),
      title_(env, "", *env.theme().font.h6),
      contents_(env),
      button_panel_(env),
      shadow_() {
  body_.setPadding(Padding(PADDING_REGULAR, PADDING_REGULAR));
  title_.setPadding(PADDING_NONE, PADDING_NONE);
  body_.add(title_);
  body_.add(contents_, VerticalLayout::Params().setWeight(1));
  add(body_, VerticalLayout::Params().setWeight(1));
  add(button_panel_,
      VerticalLayout::Params().setGravity(kHorizontalGravityRight));
  button_panel_.setGravity(
      Gravity(kHorizontalGravityRight, kVerticalGravityMiddle));
  buttons_.reserve(button_labels.size());
  int i = 0;
  for (const std::string& label : button_labels) {
    buttons_.emplace_back(env, label, Button::TEXT);
    buttons_.back().setOnClicked([this, i]() { actionTaken(i); });
    button_panel_.add(buttons_.back());
    ++i;
  }
}

void Dialog::setTitle(const std::string& title) { title_.setText(title); }

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

void Dialog::paintWidgetContents(const Canvas& s, Clipper& clipper) {
  Panel::paintWidgetContents(s, clipper);
  Rect full, visible;
  getAbsoluteBounds(full, visible);
  shadow_ = Shadow(full, 20);
  clipper.addOverlay(&shadow_, s.clip_box());
  clipper.addOverlay(&scrim(), s.clip_box());
}

}  // namespace roo_windows