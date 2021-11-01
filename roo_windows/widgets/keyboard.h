#pragma once

#include <inttypes.h>
#include <pgmspace.h>
#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "roo_windows/core/panel.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/keyboard_layout/keyboard_layout.h"

namespace roo_windows {

class KeyboardListener {
 public:
  virtual ~KeyboardListener() {}

  virtual void rune(uint32_t rune) const = 0;
  virtual void enter() const = 0;
  virtual void del() const = 0;
};

// class KeyboardBuffer {
//  public:
//   KeyboardBuffer() {}

//   void clear() {
//     offsets_.clear();
//     contents_.clear();
//   }

//   bpstd::string_view contents() const { return bpstd::string_view(contents_);
//   }

//   void append(Utf8Text text) {
//     offsets_.push_back(contents_.size() + text.len);
//     contents_.append(text.ptr, text.len);
//   }

//   bool del() {
//     if (offsets_.empty()) return false;
//     if (offsets_.size() == 1) {
//       clear();
//       return true;
//     }
//     offsets_.pop_back();
//     contents_.resize(offsets_.back());
//     return true;
//   }

//  private:
//   std::vector<uint8_t> offsets_;
//   std::string contents_;
// };

class Keyboard;
class KeyboardPage;
class TextButton;

class PressHighlighter : public Widget {
 public:
  PressHighlighter(KeyboardPage* parent, const Box& bounds);

  void setTarget(const TextButton* target) { target_ = target; }

  void paint(const Surface& s) override;

  const KeyboardPage* page() const;
  const Keyboard* keyboard() const;

 private:
  const TextButton* target_;
};

class KeyboardPage : public Panel {
 public:
  KeyboardPage(Keyboard* parent, int16_t w, int16_t h,
               const KeyboardPageSpec* spec);

  void showHighlighter(const TextButton& btn);
  void hideHighlighter();

  const Keyboard* keyboard() const;

 private:
  std::vector<std::unique_ptr<Widget>> keys_;
  std::unique_ptr<PressHighlighter> highlighter_;
};

class Keyboard : public Panel {
 public:
  Keyboard(Panel* parent, const roo_display::Box& bounds,
           const KeyboardPageSpec* spec, KeyboardListener* listener)
      : Keyboard(parent, bounds, spec, DefaultKeyboardColorTheme(), listener) {}

  Keyboard(Panel* parent, const roo_display::Box& bounds,
           const KeyboardPageSpec* spec, const KeyboardColorTheme& color_theme,
           KeyboardListener* listener);

  const KeyboardColorTheme& color_theme() const { return color_theme_; }

  void setListener(KeyboardListener* listener) { listener_ = listener; }

  const KeyboardListener* listener() const { return listener_; }

 private:
  std::vector<std::unique_ptr<KeyboardPage>> pages_;
  const KeyboardColorTheme& color_theme_;
  KeyboardPage* current_page_;
  KeyboardListener* listener_;
};

}  // namespace roo_windows