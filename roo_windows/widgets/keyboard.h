#pragma once

#include <inttypes.h>
#include <pgmspace.h>
#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "roo_windows/core/panel.h"
#include "roo_windows/keyboard_layout/keyboard_layout.h"
#include "roo_windows/widgets/button.h"

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
  PressHighlighter(const Environment& env);

  void setTarget(const TextButton* target) { target_ = target; }

  bool paint(const Surface& s) override;
  Dimensions getSuggestedMinimumDimensions() const override;

  const KeyboardPage* page() const;
  const Keyboard* keyboard() const;

  void moveTo(const Box& box) { Widget::moveTo(box); }

 private:
  const TextButton* target_;
};

class KeyboardPage : public Panel {
 public:
  KeyboardPage(const Environment& env, const KeyboardPageSpec* spec);

  void showHighlighter(const TextButton& btn);
  void hideHighlighter();

  const Keyboard* keyboard() const;

  PreferredSize getPreferredSize() const override;

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override;

  void onLayout(bool changed, const roo_display::Box& box) override;

 private:
  const KeyboardPageSpec* spec_;
  std::vector<Widget*> keys_;
  PressHighlighter* highlighter_;
};

class Keyboard : public Panel {
 public:
  Keyboard(const Environment& env, const KeyboardPageSpec* spec,
           KeyboardListener* listener);

  const KeyboardColorTheme& color_theme() const { return color_theme_; }

  void setListener(KeyboardListener* listener) { listener_ = listener; }

  const KeyboardListener* listener() const { return listener_; }

  Padding getDefaultPadding() const override { return Padding(1); }

  PreferredSize getPreferredSize() const override;

 protected:
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override;

  void onLayout(bool changed, const roo_display::Box& box) override;

 private:
  std::vector<KeyboardPage*> pages_;
  const KeyboardColorTheme& color_theme_;
  KeyboardPage* current_page_;
  KeyboardListener* listener_;
};

}  // namespace roo_windows