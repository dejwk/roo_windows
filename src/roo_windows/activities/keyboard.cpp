#include "roo_windows/activities/keyboard.h"

#include <memory>
#include <string>

#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/outlined/action.h"
#include "roo_icons/outlined/content.h"
#include "roo_io/text/unicode.h"
#include "roo_windows/config.h"
#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/button.h"

namespace roo_windows {

using namespace roo_display;

// Image file shift_24 24x24, 4-bit Alpha,  RLE, 121 bytes.
static const uint8_t shift_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50,
    0x40, 0xEB, 0x05, 0x77, 0x38, 0x34, 0xEF, 0x76, 0xA0, 0x57, 0x71,
    0x82, 0x4E, 0xF7, 0x20, 0x6A, 0x05, 0x76, 0x82, 0x4E, 0xF7, 0x40,
    0x6A, 0x05, 0x74, 0x82, 0x5E, 0xF6, 0x60, 0x5A, 0x06, 0x72, 0x05,
    0xA0, 0x67, 0x10, 0x5A, 0x06, 0x70, 0x5A, 0x06, 0x73, 0x05, 0xA0,
    0x65, 0x05, 0xA8, 0x2E, 0xAA, 0x46, 0x82, 0x3A, 0xAD, 0xA0, 0x63,
    0x05, 0xE0, 0x66, 0x05, 0xE0, 0x62, 0x80, 0x11, 0x81, 0x8F, 0x66,
    0x86, 0x5F, 0x91, 0x11, 0x12, 0x78, 0x17, 0xF6, 0x68, 0x15, 0xF8,
    0x75, 0x81, 0x7F, 0x66, 0x81, 0x5F, 0x87, 0x58, 0x17, 0xF6, 0x68,
    0x15, 0xF8, 0x75, 0x81, 0x7F, 0xC8, 0x02, 0xA8, 0x1C, 0xF8, 0x75,
    0x07, 0xFB, 0x08, 0x75, 0x01, 0x80, 0x62, 0x01, 0x80, 0x9C, 0x20,
};

const RleImage4bppxBiased<Alpha4, ProgMemPtr>& shift_24() {
  static RleImage4bppxBiased<Alpha4, ProgMemPtr> value(24, 24, shift_24_data,
                                                       Alpha4(color::Black));
  return value;
}

// Image file shift_filled_24 24x24, 4-bit Alpha,  RLE, 86 bytes.
static const uint8_t shift_filled_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50,
    0x40, 0xEB, 0x05, 0x77, 0x30, 0x40, 0xED, 0x05, 0x77, 0x10, 0x40,
    0xEF, 0x05, 0x76, 0x04, 0x0E, 0xFA, 0x05, 0x74, 0x05, 0x0E, 0xFC,
    0x06, 0x72, 0x05, 0xFF, 0x06, 0x70, 0x5F, 0xFA, 0x06, 0x50, 0x5F,
    0xFC, 0x06, 0x30, 0x5F, 0xFE, 0x06, 0x28, 0x01, 0x10, 0x8F, 0xB8,
    0x49, 0x11, 0x11, 0x27, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08,
    0x75, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB,
    0x08, 0x75, 0x01, 0x80, 0x62, 0x01, 0x80, 0x9C, 0x20,
};

const RleImage4bppxBiased<Alpha4, ProgMemPtr>& shift_filled_24() {
  static RleImage4bppxBiased<Alpha4, ProgMemPtr> value(
      24, 24, shift_filled_24_data, Alpha4(color::Black));
  return value;
}

// Image file caps_lock_filled_24 24x24, 4-bit Alpha,  RLE, 96 bytes.
static const uint8_t caps_lock_filled_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50, 0x40,
    0xEB, 0x05, 0x77, 0x30, 0x40, 0xED, 0x05, 0x77, 0x10, 0x40, 0xEF, 0x05,
    0x76, 0x04, 0x0E, 0xFA, 0x05, 0x74, 0x05, 0x0E, 0xFC, 0x06, 0x72, 0x05,
    0xFF, 0x06, 0x70, 0x5F, 0xFA, 0x06, 0x50, 0x5F, 0xFC, 0x06, 0x30, 0x5F,
    0xFE, 0x06, 0x28, 0x01, 0x10, 0x8F, 0xB8, 0x49, 0x11, 0x11, 0x27, 0x07,
    0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x07,
    0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x01, 0x80, 0x62, 0x01, 0x75,
    0x06, 0x80, 0x6C, 0x07, 0x75, 0x07, 0x80, 0x6E, 0x08, 0x80, 0xE2, 0x00,
};

const RleImage4bppxBiased<Alpha4, ProgMemPtr>& caps_lock_filled_24() {
  static RleImage4bppxBiased<Alpha4, ProgMemPtr> value(
      24, 24, caps_lock_filled_24_data, Alpha4(color::Black));
  return value;
}

static const int kExtraTopPaddingPx = 2;
static const int kHighlighterHeight = 50;

static const int kButtonMarginPercent = 10;
static const int kMinRowHeight = 10;
static const int kPreferredRowHeight = 10;
static const int kMinCellWidth = 5;
static const int kPreferredCellWidth = 15;

namespace {

std::string runeAsStr(char32_t ch) {
  char buf[4];
  int n = roo_io::WriteUtf8Char(buf, ch);
  return std::string(buf, n);
}

}  // namespace

class KeyboardPage;
class TextButton;
class KeyboardWidget;

class KeyboardButton : public SimpleButton {
 public:
  using SimpleButton::SimpleButton;
  virtual void capsStateUpdated() {}

  // bool onScroll(int16_t dx, int16_t dy) override;
  // bool onFling(int16_t vx, int16_t vy) override;

  void onCancel() override;

  Margins getMargins() const override {
    int16_t m =
        std::max<int16_t>(width(), height()) * kButtonMarginPercent / 200;
    return Margins(m);
  }

  BorderStyle getBorderStyle() const override {
    return BorderStyle(Scaled(3), 0);
  }

 protected:
  KeyboardWidget& keyboard();
};

class PressHighlighter : public Widget {
 public:
  PressHighlighter(const Environment& env);

  void setTarget(const TextButton* target) { target_ = target; }

  void paint(const Canvas& canvas) const override;
  Dimensions getSuggestedMinimumDimensions() const override;

  const KeyboardPage* page() const;
  const KeyboardWidget* keyboard() const;

  void moveTo(const Rect& rect) { Widget::moveTo(rect); }

 private:
  const TextButton* target_;
};

class KeyboardPage : public Panel {
 public:
  KeyboardPage(const Environment& env, const KeyboardPageSpec* spec);

  void init(const Environment& env);

  void showHighlighter(const TextButton& btn);
  void hideHighlighter();

  const KeyboardWidget* keyboard() const;
  KeyboardWidget* keyboard();

  PreferredSize getPreferredSize() const override;
  Padding getPadding() const override { return Padding(6); }

  void capsStateUpdated();

  bool respectsChildrenBoundaries() const override { return true; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

 private:
  const KeyboardPageSpec* spec_;
  std::vector<KeyboardButton*> keys_;
  PressHighlighter highlighter_;
  bool initialized_;
};

class KeyboardWidget : public Panel {
 public:
  KeyboardWidget(const Environment& env, const KeyboardSpec* spec);

  const KeyboardColorTheme& color_theme() const { return color_theme_; }

  void setListener(KeyboardListener* listener) { listener_ = listener; }

  KeyboardListener* listener() const { return listener_; }

  Padding getPadding() const override { return Padding(1); }

  PreferredSize getPreferredSize() const override;

  void setCapsState(Keyboard::CapsState caps_state);

  Keyboard::CapsState caps_state() const { return caps_state_; }

  void setPage(int idx);

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

 private:
  const Environment* env_;
  std::vector<KeyboardPage*> pages_;
  const KeyboardColorTheme& color_theme_;
  Keyboard::CapsState caps_state_;
  KeyboardPage* current_page_;
  KeyboardListener* listener_;
};

KeyboardWidget& KeyboardButton::keyboard() {
  return *((KeyboardPage*)parent())->keyboard();
}

void KeyboardButton::onCancel() {
  KeyboardPage* page = ((KeyboardPage*)parent());
  page->hideHighlighter();
  Button::onCancel();
}

class TextButton : public KeyboardButton {
 public:
  TextButton(const Environment& env, uint16_t rune, uint16_t rune_caps)
      : KeyboardButton(env, runeAsStr(rune)),
        rune_(rune),
        rune_caps_(rune_caps) {
    setFont(font_body1());
  }

  bool showClickAnimation() const override { return false; }

  void onShowPress(XDim x, YDim y) override {
    ((KeyboardPage*)parent())->showHighlighter(*this);
    Button::onShowPress(x, y);
  }

  void capsStateUpdated() override {
    if (rune_ == rune_caps_) return;
    Keyboard::CapsState caps = keyboard().caps_state();
    if (caps == Keyboard::CAPS_STATE_LOW) {
      setLabel(runeAsStr(rune_));
    } else {
      setLabel(runeAsStr(rune_caps_));
    }
  }

  bool onSingleTapUp(XDim x, YDim y) override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    page->hideHighlighter();
    KeyboardListener* listener = page->keyboard()->listener();
    Keyboard::CapsState caps = keyboard().caps_state();
    if (listener != nullptr) {
      listener->rune(caps == Keyboard::CAPS_STATE_LOW ? rune_ : rune_caps_);
    }
    if (caps == Keyboard::CAPS_STATE_HIGH) {
      // Flip back.
      keyboard().setCapsState(Keyboard::CAPS_STATE_LOW);
    }
    return Button::onSingleTapUp(x, y);
  }

  uint16_t rune_;
  uint16_t rune_caps_;
};

class SpaceButton : public KeyboardButton {
 public:
  SpaceButton(const Environment& env) : KeyboardButton(env, "") {}

  bool showClickAnimation() const override { return false; }

  bool onSingleTapUp(XDim x, YDim y) override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->rune(' ');
    }
    return Button::onSingleTapUp(x, y);
  }
};

class EnterButton : public KeyboardButton {
 public:
  EnterButton(const Environment& env, const MonoIcon& icon)
      : KeyboardButton(env, icon) {}

  bool showClickAnimation() const override { return false; }

  bool onSingleTapUp(XDim x, YDim y) override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->enter();
    }
    return Button::onSingleTapUp(x, y);
  }
};

class ShiftButton : public KeyboardButton {
 public:
  ShiftButton(const Environment& env) : KeyboardButton(env, shift_24()) {}

  bool showClickAnimation() const override { return false; }

  void onShowPress(XDim x, YDim y) override {
    auto& kb = keyboard();
    switch (kb.caps_state()) {
      case Keyboard::CAPS_STATE_LOW: {
        kb.setCapsState(Keyboard::CAPS_STATE_HIGH);
        break;
      }
      case Keyboard::CAPS_STATE_HIGH:
      case Keyboard::CAPS_STATE_HIGH_LOCKED: {
        kb.setCapsState(Keyboard::CAPS_STATE_LOW);
        break;
      }
    }
    KeyboardButton::onShowPress(x, y);
  }

  bool supportsLongPress() override { return true; }

  void onLongPress(XDim x, YDim y) {
    auto& kb = keyboard();
    if (kb.caps_state() == Keyboard::CAPS_STATE_HIGH) {
      kb.setCapsState(Keyboard::CAPS_STATE_HIGH_LOCKED);
      setIcon(&caps_lock_filled_24());
    }
    KeyboardButton::onLongPress(x, y);
  }

  void capsStateUpdated() override {
    auto& kb = keyboard();
    switch (kb.caps_state()) {
      case Keyboard::CAPS_STATE_LOW: {
        setIcon(&shift_24());
        break;
      }
      case Keyboard::CAPS_STATE_HIGH: {
        setIcon(&shift_filled_24());
        break;
      }
      case Keyboard::CAPS_STATE_HIGH_LOCKED: {
        setIcon(&caps_lock_filled_24());
        break;
      }
    }
  }
};

class DelButton : public KeyboardButton {
 public:
  DelButton(const Environment& env, const MonoIcon& icon)
      : KeyboardButton(env, icon) {}

  bool showClickAnimation() const override { return false; }

  bool onSingleTapUp(XDim x, YDim y) override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->del();
    }
    return Button::onSingleTapUp(x, y);
  }
};

class PageSwitchButton : public KeyboardButton {
 public:
  PageSwitchButton(const Environment& env, std::string label, uint8_t target)
      : KeyboardButton(env, label), target_(target) {}

  bool showClickAnimation() const override { return false; }

  void onShowPress(XDim x, YDim y) override {
    keyboard().setPage(target_);
    KeyboardButton::onShowPress(x, y);
  }

 private:
  uint8_t target_;
};

const KeyboardWidget* KeyboardPage::keyboard() const {
  return (KeyboardWidget*)parent();
}

KeyboardWidget* KeyboardPage::keyboard() { return (KeyboardWidget*)parent(); }

KeyboardWidget::KeyboardWidget(const Environment& env, const KeyboardSpec* spec)
    : Panel(env),
      env_(&env),
      color_theme_(env.keyboardColorTheme()),
      caps_state_(Keyboard::CAPS_STATE_LOW),
      listener_(nullptr) {
  setParentClipMode(Widget::UNCLIPPED);
  for (int i = 0; i < spec->page_count; ++i) {
    auto page = new KeyboardPage(env, &spec->pages[i]);
    add(std::unique_ptr<KeyboardPage>(page));
    pages_.push_back(page);
  }
  setPage(-1);
}

PreferredSize KeyboardWidget::getPreferredSize() const {
  return current_page_ == nullptr ? PreferredSize(PreferredSize::ExactWidth(0),
                                                  PreferredSize::ExactHeight(0))
                                  : current_page_->getPreferredSize();
}

PreferredSize KeyboardPage::getPreferredSize() const {
  Padding padding = getPadding();
  return PreferredSize(
      PreferredSize::MatchParentWidth(),
      PreferredSize::ExactHeight(spec_->row_count * kPreferredRowHeight +
                                 padding.top() + padding.bottom() +
                                 kExtraTopPaddingPx));
}

Dimensions KeyboardWidget::onMeasure(WidthSpec width, HeightSpec height) {
  return (current_page_ == nullptr) ? Dimensions()
                                    : current_page_->measure(width, height);
}

void KeyboardWidget::onLayout(bool changed, const Rect& rect) {
  if (current_page_ != nullptr) {
    current_page_->layout(Rect(0, 0, rect.width() - 1, rect.height() - 1));
  }
}

void KeyboardWidget::setCapsState(Keyboard::CapsState caps_state) {
  if (caps_state == caps_state_) return;
  bool caps_switched = (caps_state == Keyboard::CAPS_STATE_LOW ||
                        caps_state_ == Keyboard::CAPS_STATE_LOW);
  caps_state_ = caps_state;
  if (caps_switched) {
    for (auto& page : pages_) {
      page->capsStateUpdated();
    }
  }
}

void KeyboardWidget::setPage(int idx) {
  if (idx < 0) {
    if (current_page_ == nullptr) return;
    current_page_ = nullptr;
  } else {
    if (current_page_ == pages_[idx]) return;
    current_page_ = pages_[idx];
    current_page_->init(*env_);
  }
  for (int i = 0; i < pages_.size(); ++i) {
    pages_[i]->setVisibility(i == idx ? VISIBLE : GONE);
  }
  setCapsState(Keyboard::CAPS_STATE_LOW);
  requestLayout();
}

KeyboardPage::KeyboardPage(const Environment& env, const KeyboardPageSpec* spec)
    : Panel(env), spec_(spec), highlighter_(env), initialized_(false) {
  setBackground(env.keyboardColorTheme().background);
  setParentClipMode(Widget::UNCLIPPED);
}

void KeyboardPage::init(const Environment& env) {
  if (initialized_) return;
  initialized_ = true;
  for (int i = 0; i < spec_->row_count; ++i) {
    const auto& row = spec_->rows[i];
    for (int j = 0; j < row.key_count; ++j) {
      KeyboardButton* b;
      Color b_color;
      const auto& key = row.keys[j];
      const auto& key_caps = row.keys_caps[j];
      switch (key.function) {
        case KeySpec::TEXT: {
          b = new TextButton(env, key.data, key_caps.data);
          b_color = env.keyboardColorTheme().normalButton;
          break;
        }
        case KeySpec::SPACE: {
          b = new SpaceButton(env);
          b_color = env.keyboardColorTheme().normalButton;
          break;
        }
        case KeySpec::ENTER: {
          b = new EnterButton(env, ic_outlined_24_action_done());
          b_color = env.keyboardColorTheme().acceptButton;
          break;
        }
        case KeySpec::SHIFT: {
          b = new ShiftButton(env);
          b_color = env.keyboardColorTheme().modifierButton;
          break;
        }
        case KeySpec::DEL: {
          b = new DelButton(env, ic_outlined_24_content_backspace());
          b_color = env.keyboardColorTheme().modifierButton;
          break;
        }
        case KeySpec::SWITCH_PAGE: {
          b = new PageSwitchButton(
              env,
              std::string(row.pageswitch_key_labels + ((key.data >> 8) & 0xFF),
                          key.data >> 16),
              (key.data & 0xFF));
          b_color = env.keyboardColorTheme().modifierButton;
          break;
        }
      }
      b->setInteriorColor(b_color);
      b->setOutlineColor(b_color);
      b->setContentColor(roo_display::color::White);
      keys_.emplace_back(b);
      add(std::unique_ptr<Button>(b));
    }
  }
  // Note: highlighter must be added last to be on top of all children.
  highlighter_.setVisibility(INVISIBLE);
  highlighter_.setParentClipMode(Widget::UNCLIPPED);
  add(highlighter_);
}

Dimensions KeyboardPage::onMeasure(WidthSpec width, HeightSpec height) {
  // Calculate grid size.
  Padding padding = getPadding();
  int16_t row_height;
  int16_t full_height;
  // int16_t top_offset;
  if (height.kind() == UNSPECIFIED) {
    row_height = kPreferredRowHeight;
    full_height = row_height * spec_->row_count + padding.top() +
                  padding.bottom() + kExtraTopPaddingPx;
  } else {
    int16_t hspan =
        height.value() - padding.top() - padding.bottom() - kExtraTopPaddingPx;
    row_height = std::max<int16_t>(kMinRowHeight, hspan / spec_->row_count);
    full_height = height.value();
  }
  int16_t cell_width;
  int16_t full_width;
  if (width.kind() == UNSPECIFIED) {
    cell_width = kPreferredCellWidth;
    full_width =
        cell_width * spec_->row_width + padding.left() + padding.right();
  } else {
    int16_t vspan = width.value() - padding.left() - padding.right();
    cell_width = std::max<int16_t>(kMinCellWidth, vspan / spec_->row_width);
    full_width = width.value();
  }

  int h_key_margin = row_height * kButtonMarginPercent / 100;
  if (h_key_margin < 1) h_key_margin = 1;
  int v_key_margin = cell_width * kButtonMarginPercent / 100;
  if (v_key_margin < 1) v_key_margin = 1;

  // Now go through all children and have them measured.
  int child_idx = 0;
  for (int i = 0; i < spec_->row_count; ++i) {
    const auto& row = spec_->rows[i];
    for (int j = 0; j < row.key_count; ++j) {
      const auto& key = row.keys[j];
      Widget& w = child_at(child_idx++);
      w.measure(WidthSpec::Exactly(cell_width - 2 * v_key_margin * key.width),
                HeightSpec::Exactly(row_height - 2 * h_key_margin));
    }
  }
  // We skip the measurement of the highlighter.
  return Dimensions(full_height, full_width);
}

void KeyboardPage::onLayout(bool changed, const Rect& rect) {
  // Recalculate now that we have a specific dimensions.
  Padding padding = getPadding();
  int16_t vspan =
      rect.height() - padding.top() - padding.bottom() - kExtraTopPaddingPx;
  int16_t row_height =
      std::max<int16_t>(kMinRowHeight, vspan / spec_->row_count);
  int16_t top_offset = std::max<int16_t>(
      0,
      (rect.height() - kExtraTopPaddingPx - spec_->row_count * row_height) / 2);
  int16_t hspan = rect.width() - padding.left() - padding.right();
  int16_t cell_width =
      std::max<int16_t>(kMinCellWidth, hspan / spec_->row_width);
  int16_t left_offset =
      std::max<int16_t>(0, (rect.width() - spec_->row_width * cell_width) / 2);

  int h_key_margin = row_height * kButtonMarginPercent / 100;
  if (h_key_margin < 1) h_key_margin = 1;
  int v_key_margin = cell_width * kButtonMarginPercent / 100;
  if (v_key_margin < 1) v_key_margin = 1;

  int y = top_offset + kExtraTopPaddingPx;
  int child_idx = 0;
  for (int i = 0; i < spec_->row_count; ++i) {
    const auto& row = spec_->rows[i];
    int x = left_offset + row.start_offset * cell_width;
    for (int j = 0; j < row.key_count; ++j) {
      const auto& key = row.keys[j];
      Box bounds(x + v_key_margin, y + h_key_margin,
                 x + key.width * cell_width - v_key_margin - 1,
                 y + row_height - h_key_margin - 1);
      Widget& w = child_at(child_idx++);
      w.layout(bounds);
      x += key.width * cell_width;
    }
    y += row_height;
  }
  // Skipping the highlighter.
}

void KeyboardPage::showHighlighter(const TextButton& btn) {
  const Rect& bBounds = btn.parent_bounds();
  highlighter_.moveTo(Rect(bBounds.xMin(), bBounds.yMin() - kHighlighterHeight,
                           bBounds.xMax(), bBounds.yMax() - 3));
  highlighter_.setTarget(&btn);
  highlighter_.setVisibility(VISIBLE);
}

void KeyboardPage::hideHighlighter() {
  // Use INVISIBLE to avoid needlessly re-measuring the keyboard page.
  highlighter_.setVisibility(INVISIBLE);
  highlighter_.setTarget(nullptr);
}

void KeyboardPage::capsStateUpdated() {
  for (auto& key : keys_) {
    key->capsStateUpdated();
  }
}

PressHighlighter::PressHighlighter(const Environment& env) : Widget(env) {
  setParentClipMode(Widget::UNCLIPPED);
}

const KeyboardPage* PressHighlighter::page() const {
  return (KeyboardPage*)parent();
}

const KeyboardWidget* PressHighlighter::keyboard() const {
  return page()->keyboard();
}

KeyboardWidget* Keyboard::contents() {
  return (KeyboardWidget*)contents_.get();
}

const KeyboardWidget* Keyboard::contents() const {
  return (KeyboardWidget*)contents_.get();
}

void PressHighlighter::paint(const Canvas& canvas) const {
  if (target_ == nullptr) {
    canvas.clear();
    return;
  }
  const Theme& th = theme();
  const KeyboardColorTheme& kbTh = keyboard()->color_theme();
  Color overlay = roo_display::color::Black;
  overlay.set_a(th.pressedOpacity(kbTh.normalButton));
  Color bgcolor = roo_display::AlphaBlend(kbTh.normalButton, overlay);
  canvas.drawObject(roo_display::MakeTileOf(
      roo_display::StringViewLabel(target_->label(), font_body1(),
                                   target_->contentColor()),
      bounds().asBox(), roo_display::kCenter | roo_display::kTop.shiftBy(3),
      bgcolor));
}

Dimensions PressHighlighter::getSuggestedMinimumDimensions() const {
  return Dimensions(0, 0);
}

Keyboard::Keyboard(const Environment& env, const KeyboardSpec* spec)
    : contents_(new KeyboardWidget(env, spec)) {
  contents_->setVisibility(Widget::GONE);
}

Widget& Keyboard::getContents() { return *contents_; }

void Keyboard::show() {
  contents()->setPage(0);
  contents()->setVisibility(Widget::VISIBLE);
}

void Keyboard::hide() {
  contents()->setVisibility(Widget::GONE);
  contents()->setPage(-1);
  contents()->setCapsState(CapsState::CAPS_STATE_LOW);
}

void Keyboard::setListener(KeyboardListener* listener) {
  contents()->setListener(listener);
}

// Position the keyboard in the lower half of the screen.
roo_display::Box Keyboard::getPreferredPlacement(const Task& task) {
  auto& window = task.getMainWindow();
  Dimensions dims(window.width(), window.height());
  XDim dx;
  YDim dy;
  task.getAbsoluteOffset(dx, dy);
  return roo_display::Box(0, dims.height() / 2, dims.width() - 1,
                          dims.height() - 1)
      .translate(-dx, -dy);
}

void Keyboard::setPage(int idx) { contents()->setPage(idx); }

Keyboard::CapsState Keyboard::caps_state() const {
  return contents()->caps_state();
}

void Keyboard::setCapsState(CapsState caps_state) {
  contents()->setCapsState(caps_state);
}

}  // namespace roo_windows
