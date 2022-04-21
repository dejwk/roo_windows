#include "keyboard.h"

#include <memory>
#include <string>

#include "button.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_material_icons/outlined/24/action.h"
#include "roo_material_icons/outlined/24/content.h"

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

const RleImage4bppxPolarized<Alpha4, PrgMemResource>& shift_24() {
  static RleImage4bppxPolarized<Alpha4, PrgMemResource> value(
      24, 24, shift_24_data, Alpha4(color::Black));
  return value;
}

// Image file caps_lock_24 24x24, 4-bit Alpha,  RLE, 130 bytes.
static const uint8_t caps_lock_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50, 0x40,
    0xEB, 0x05, 0x77, 0x38, 0x34, 0xEF, 0x76, 0xA0, 0x57, 0x71, 0x82, 0x4E,
    0xF7, 0x20, 0x6A, 0x05, 0x76, 0x82, 0x4E, 0xF7, 0x40, 0x6A, 0x05, 0x74,
    0x82, 0x5E, 0xF6, 0x60, 0x5A, 0x06, 0x72, 0x05, 0xA0, 0x67, 0x10, 0x5A,
    0x06, 0x70, 0x5A, 0x06, 0x73, 0x05, 0xA0, 0x65, 0x05, 0xA8, 0x2E, 0xAA,
    0x46, 0x82, 0x3A, 0xAD, 0xA0, 0x63, 0x05, 0xE0, 0x66, 0x05, 0xE0, 0x62,
    0x80, 0x11, 0x81, 0x8F, 0x66, 0x86, 0x5F, 0x91, 0x11, 0x12, 0x78, 0x17,
    0xF6, 0x68, 0x15, 0xF8, 0x75, 0x81, 0x7F, 0x66, 0x81, 0x5F, 0x87, 0x58,
    0x17, 0xF6, 0x68, 0x15, 0xF8, 0x75, 0x81, 0x7F, 0xC8, 0x02, 0xA8, 0x1C,
    0xF8, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x03, 0x80, 0x66, 0x03, 0x75, 0x07,
    0xFB, 0x08, 0x75, 0x04, 0x80, 0x67, 0x04, 0x80, 0xE2, 0x00,
};

const RleImage4bppxPolarized<Alpha4, PrgMemResource>& caps_lock_24() {
  static RleImage4bppxPolarized<Alpha4, PrgMemResource> value(
      24, 24, caps_lock_24_data, Alpha4(color::Black));
  return value;
}

static const int kExtraTopPaddingPx = 20;
static const int kHighlighterHeight = 50;

static const int kButtonMarginPercent = 10;
static const int kMinRowHeight = 10;
static const int kPreferredRowHeight = 10;
static const int kMinCellWidth = 5;
static const int kPreferredCellWidth = 15;

namespace {

std::string runeAsStr(uint32_t rune) {
  char buf[4];
  if (rune <= 0x7F) {
    buf[0] = rune;
    return std::string(buf, 1);
  }
  if (rune <= 0x7FF) {
    buf[1] = (rune & 0x3F) | 0x80;
    rune >>= 6;
    buf[0] = rune | 0xC0;
    return std::string(buf, 2);
  }
  if (rune <= 0xFFFF) {
    buf[2] = (rune & 0x3F) | 0x80;
    rune >>= 6;
    buf[1] = (rune & 0x3F) | 0x80;
    rune >>= 6;
    buf[0] = rune | 0xE0;
    return std::string(buf, 3);
  }
  buf[3] = (rune & 0x3F) | 0x80;
  rune >>= 6;
  buf[2] = (rune & 0x3F) | 0x80;
  rune >>= 6;
  buf[1] = (rune & 0x3F) | 0x80;
  rune >>= 6;
  buf[0] = rune | 0xF0;
  return std::string(buf, 4);
}

}  // namespace

class TextButton : public Button {
 public:
  TextButton(const Environment& env, uint32_t rune)
      : Button(env, runeAsStr(rune)), rune_(rune) {}

  bool showClickAnimation() const override { return false; }

  void onShowPress(int16_t x, int16_t y) override {
    ((KeyboardPage*)parent())->showHighlighter(*this);
    Button::onShowPress(x, y);
  }

  bool onSingleTapUp(int16_t x, int16_t y) override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    page->hideHighlighter();
    const KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->rune(rune_);
    }
    return Button::onSingleTapUp(x, y);
  }

  const roo_display::Font& getFont() const override {
    return *env().theme().font.h6;
  }

 private:
  uint32_t rune_;
};

class SpaceButton : public Button {
 public:
  SpaceButton(const Environment& env) : Button(env, "") {}

  bool showClickAnimation() const override { return false; }

  void onShowPress(int16_t x, int16_t y) override {}

  bool onSingleTapUp(int16_t x, int16_t y) override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    const KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->rune(' ');
    }
    return true;
  }
};

class FnButton : public Button {
 public:
  FnButton(const Environment& env, const MonoIcon& icon)
      : Button(env, icon) {}

  bool showClickAnimation() const override { return false; }
};

const Keyboard* KeyboardPage::keyboard() const { return (Keyboard*)parent(); }

Keyboard::Keyboard(const Environment& env, const KeyboardPageSpec* spec,
                   KeyboardListener* listener)
    : Panel(env), color_theme_(env.keyboardColorTheme()), listener_(listener) {
  setParentClipMode(Widget::UNCLIPPED);
  auto page = new KeyboardPage(env, spec);
  add(std::unique_ptr<KeyboardPage>(page));
  current_page_ = page;
  pages_.push_back(page);
}

PreferredSize Keyboard::getPreferredSize() const {
  return current_page_->getPreferredSize();
}

PreferredSize KeyboardPage::getPreferredSize() const {
  Padding padding = getDefaultPadding();
  return PreferredSize(
      PreferredSize::MatchParent(),
      PreferredSize::Exact(spec_->row_count * kPreferredRowHeight +
                           padding.top() + padding.bottom() +
                           kExtraTopPaddingPx));
}

Dimensions Keyboard::onMeasure(MeasureSpec width, MeasureSpec height) {
  return current_page_->measure(width, height);
}

void Keyboard::onLayout(bool changed, const roo_display::Box& box) {
  current_page_->layout(Box(0, 0, box.width() - 1, box.height() - 1));
}

KeyboardPage::KeyboardPage(const Environment& env, const KeyboardPageSpec* spec)
    : Panel(env), spec_(spec), highlighter_(env) {
  setBackground(env.keyboardColorTheme().background);
  setParentClipMode(Widget::UNCLIPPED);
  for (int i = 0; i < spec->row_count; ++i) {
    const auto& row = spec->rows[i];
    for (int j = 0; j < row.key_count; ++j) {
      Button* b;
      Color b_color;
      const auto& key = row.keys[j];
      switch (key.function) {
        case KeySpec::TEXT: {
          b = new TextButton(env, key.data);
          b_color = env.keyboardColorTheme().normalButton;
          break;
        }
        case KeySpec::SPACE: {
          b = new SpaceButton(env);
          b_color = env.keyboardColorTheme().normalButton;
          break;
        }
        case KeySpec::ENTER: {
          b = new FnButton(env, ic_outlined_24_action_done());
          b_color = env.keyboardColorTheme().acceptButton;
          break;
        }
        case KeySpec::SHIFT: {
          b = new FnButton(env, shift_24());
          b_color = env.keyboardColorTheme().modifierButton;
          break;
        }
        case KeySpec::SWITCH_PAGE:
        default: {
          b = new FnButton(env, ic_outlined_24_content_backspace());
          b_color = env.keyboardColorTheme().modifierButton;
          break;
        }
      }
      b->setInteriorColor(b_color);
      b->setOutlineColor(b_color);
      b->setTextColor(roo_display::color::White);
      keys_.emplace_back(b);
      add(std::unique_ptr<Button>(b));
    }
  }
  // Note: highlighter must be added last to be on top of all children.
  highlighter_.setVisible(false);
  highlighter_.setParentClipMode(Widget::UNCLIPPED);
  add(highlighter_);
}

Dimensions KeyboardPage::onMeasure(MeasureSpec width, MeasureSpec height) {
  // Calculate grid size.
  Padding padding = getDefaultPadding();
  int16_t row_height;
  int16_t full_height;
  // int16_t top_offset;
  if (height.kind() == MeasureSpec::UNSPECIFIED) {
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
  if (width.kind() == MeasureSpec::UNSPECIFIED) {
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
      w.measure(MeasureSpec::Exactly(cell_width - 2 * v_key_margin * key.width),
                MeasureSpec::Exactly(row_height - 2 * h_key_margin));
    }
  }
  // We skip the measurement of the highlighter.
  return Dimensions(full_height, full_width);
}

void KeyboardPage::onLayout(bool changed, const roo_display::Box& box) {
  // Recalculate now that we have a specific dimensions.
  Padding padding = getDefaultPadding();
  int16_t vspan =
      box.height() - padding.top() - padding.bottom() - kExtraTopPaddingPx;
  int16_t row_height = std::max<int16_t>(kMinRowHeight, vspan / spec_->row_count);
  int16_t top_offset = std::max<int16_t>(
      0,
      (box.height() - kExtraTopPaddingPx - spec_->row_count * row_height) / 2);
  int16_t hspan = box.width() - padding.left() - padding.right();
  int16_t cell_width = std::max<int16_t>(kMinCellWidth, hspan / spec_->row_width);
  int16_t left_offset =
      std::max<int16_t>(0, (box.width() - spec_->row_width * cell_width) / 2);

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
  const Box& bBounds = btn.parent_bounds();
  highlighter_.moveTo(Box(bBounds.xMin(), bBounds.yMin() - kHighlighterHeight,
                         bBounds.xMax(), bBounds.yMax() - 3));
  highlighter_.setTarget(&btn);
  highlighter_.setVisible(true);
}

void KeyboardPage::hideHighlighter() {
  highlighter_.setVisible(false);
  highlighter_.setTarget(nullptr);
}

PressHighlighter::PressHighlighter(const Environment& env) : Widget(env) {
  setParentClipMode(Widget::UNCLIPPED);
}

const KeyboardPage* PressHighlighter::page() const {
  return (KeyboardPage*)parent();
}

const Keyboard* PressHighlighter::keyboard() const {
  return page()->keyboard();
}

bool PressHighlighter::paint(const Surface& s) {
  if (target_ == nullptr) {
    s.drawObject(roo_display::Clear());
    return true;
  }
  const Theme& th = theme();
  const KeyboardColorTheme& kbTh = keyboard()->color_theme();
  Color overlay = roo_display::color::Black;
  overlay.set_a(th.pressedOpacity(kbTh.normalButton));
  Color bgcolor = roo_display::alphaBlend(kbTh.normalButton, overlay);
  s.drawObject(roo_display::MakeTileOf(
      roo_display::TextLabel(*th.font.body1, target_->label(),
                             target_->textColor()),
      bounds(), roo_display::HAlign::Center(), roo_display::VAlign::Top(3),
      bgcolor, roo_display::FILL_MODE_RECTANGLE));
  return true;
}

Dimensions PressHighlighter::getSuggestedMinimumDimensions() const {
  return Dimensions(0, 0);
}

}  // namespace roo_windows
