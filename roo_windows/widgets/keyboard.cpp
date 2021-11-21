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

static const int kTopMarginPx = 10;
static const int kBottomMarginPx = 0;
static const int kLeftMarginPx = 1;
static const int kRightMarginPx = 1;
static const int kHighlighterHeight = 50;

static const int kButtonBorderPercent = 10;

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
  TextButton(const Environment& env, KeyboardPage* parent, const Box& bounds,
             uint32_t rune)
      : Button(env, runeAsStr(rune)), rune_(rune) {
    parent->add(this);
    setParentBounds(bounds);
  }

  bool showClickAnimation() const override { return false; }

  void onPressed() override {
    ((KeyboardPage*)parent())->showHighlighter(*this);
  }

  void onReleased() override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    page->hideHighlighter();
    const KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->rune(rune_);
    }
  }

 private:
  uint32_t rune_;
};

class SpaceButton : public Button {
 public:
  SpaceButton(const Environment& env, KeyboardPage* parent, const Box& bounds)
      : Button(env, "") {
    parent->add(this);
    setParentBounds(bounds);
  }

  bool showClickAnimation() const override { return false; }

  void onPressed() override {}

  void onReleased() override {
    KeyboardPage* page = ((KeyboardPage*)parent());
    const KeyboardListener* listener = page->keyboard()->listener();
    if (listener != nullptr) {
      listener->rune(' ');
    }
  }
};

class FnButton : public Button {
 public:
  FnButton(const Environment& env, KeyboardPage* parent, const Box& bounds,
           const MonoIcon& icon)
      : Button(env, icon) {
    parent->add(this);
    setParentBounds(bounds);
  }

  bool showClickAnimation() const override { return false; }
};

const Keyboard* KeyboardPage::keyboard() const { return (Keyboard*)parent(); }

Keyboard::Keyboard(const Environment& env, Panel* parent,
                   const roo_display::Box& bounds, const KeyboardPageSpec* spec,
                   KeyboardListener* listener)
    : Panel(env),
      color_theme_(env.keyboardColorTheme()),
      listener_(listener) {
  parent->add(this);
  setParentBounds(bounds);
  setParentClipMode(Widget::UNCLIPPED);
  auto page =
      new KeyboardPage(env, this, bounds.width(), bounds.height(), spec);
  current_page_ = page;
  pages_.emplace_back(page);
}

KeyboardPage::KeyboardPage(const Environment& env, Keyboard* parent, int16_t w,
                           int16_t h, const KeyboardPageSpec* spec)
    : Panel(env) {
  parent->add(this);
  setParentBounds(Box(0, 0, w - 1, h - 1));
  setBackground(parent->color_theme().background);
  setParentClipMode(Widget::UNCLIPPED);
  // Calculate grid size.
  if (h <= kTopMarginPx + kBottomMarginPx) return;
  int row_height = (h - kTopMarginPx - kBottomMarginPx) / spec->row_count;
  if (row_height <= 2) return;
  if (h <= kLeftMarginPx + kRightMarginPx) return;
  int cell_width = (w - kLeftMarginPx - kRightMarginPx) / spec->row_width;
  if (cell_width <= 2) return;
  int h_key_border = row_height * kButtonBorderPercent / 100;
  if (h_key_border < 1) h_key_border = 1;
  int v_key_border = cell_width * kButtonBorderPercent / 100;
  if (v_key_border < 1) v_key_border = 1;

  int left_offset = (w - spec->row_width * cell_width) / 2;
  int top_offset = (h + kTopMarginPx - spec->row_count * row_height) / 2;
  int y = top_offset;
  for (int i = 0; i < spec->row_count; ++i) {
    const auto& row = spec->rows[i];
    int x = left_offset + row.start_offset * cell_width;
    for (int j = 0; j < row.key_count; ++j) {
      const auto& key = row.keys[j];
      Box bounds(x + v_key_border, y + h_key_border,
                 x + key.width * cell_width - v_key_border - 1,
                 y + row_height - h_key_border - 1);
      Button* b;
      Color b_color;
      switch (key.function) {
        case KeySpec::TEXT: {
          b = new TextButton(env, this, bounds, key.data);
          b_color = parent->color_theme().normalButton;
          break;
        }
        case KeySpec::SPACE: {
          b = new SpaceButton(env, this, bounds);
          b_color = parent->color_theme().normalButton;
          break;
        }
        case KeySpec::ENTER: {
          b = new FnButton(env, this, bounds, ic_outlined_24_action_done());
          b_color = parent->color_theme().acceptButton;
          break;
        }
        case KeySpec::SHIFT: {
          b = new FnButton(env, this, bounds, shift_24());
          b_color = parent->color_theme().modifierButton;
          break;
        }
        case KeySpec::SWITCH_PAGE:
        default: {
          b = new FnButton(env, this, bounds,
                           ic_outlined_24_content_backspace());
          b_color = parent->color_theme().modifierButton;
          break;
        }
      }
      b->setInteriorColor(b_color);
      b->setOutlineColor(b_color);
      b->setTextColor(roo_display::color::White);
      keys_.emplace_back(b);
      x += key.width * cell_width;
    }
    y += row_height;
  }
  // Note: highlighter must be added last to be on top of all children.
  highlighter_ = std::unique_ptr<PressHighlighter>(
      new PressHighlighter(env, this, Box(0, 0, -1, -1)));
  highlighter_->setVisible(false);
  highlighter_->setParentClipMode(Widget::UNCLIPPED);
}

void KeyboardPage::showHighlighter(const TextButton& btn) {
  const Box& bBounds = btn.parent_bounds();
  highlighter_->moveTo(Box(bBounds.xMin(), bBounds.yMin() - kHighlighterHeight,
                           bBounds.xMax(), bBounds.yMax() - 3));
  highlighter_->setTarget(&btn);
  highlighter_->setVisible(true);
}

void KeyboardPage::hideHighlighter() {
  highlighter_->setVisible(false);
  highlighter_->setTarget(nullptr);
}

PressHighlighter::PressHighlighter(const Environment& env, KeyboardPage* parent,
                                   const Box& bounds)
    : Widget(env) {
  parent->add(this);
  setParentBounds(bounds);
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
      roo_display::TextLabel(*th.font.button, target_->label(),
                             target_->textColor()),
      bounds(), roo_display::HAlign::Center(), roo_display::VAlign::Top(3),
      bgcolor, roo_display::FILL_MODE_RECTANGLE));
  return true;
}

Dimensions PressHighlighter::getSuggestedMinimumDimensions() const {
  return Dimensions(bounds().width(), bounds().height());
}

}  // namespace roo_windows
