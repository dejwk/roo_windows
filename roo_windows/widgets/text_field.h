#pragma once

#include "roo_display/core/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class TextField;

class TextFieldEditor : public KeyboardListener {
 public:
  TextFieldEditor(Keyboard& keyboard)
      : keyboard_(keyboard),
        target_(nullptr),
        cursor_position_(0),
        selection_begin_(0),
        selection_end_(4),
        draw_xoffset_(0) {}

  void edit(TextField* target);

  bool isEdited(const TextField* target) const;

  const std::vector<roo_display::GlyphMetrics>& glyphs() const {
    return glyphs_;
  }

  int16_t draw_xoffset() const { return draw_xoffset_; }
  int16_t selection_begin() const { return selection_begin_; }
  int16_t selection_end() const { return selection_end_; }
  int16_t cursor_position() const { return cursor_position_; }

  bool has_selection() const { return selection_end_ > selection_begin_; }

  void setSelection(int16_t selection_begin, int16_t selection_end);

  void rune(uint32_t rune) override;

  void enter() override;

  void del() override;

 private:
  friend class TextField;

  void measure();

  Keyboard& keyboard_;

  TextField* target_;
  std::vector<roo_display::GlyphMetrics> glyphs_;
  std::vector<int16_t> offsets_;
  int16_t cursor_position_;
  int16_t selection_begin_;
  int16_t selection_end_;
  int16_t draw_xoffset_;
};

class TextField : public Widget {
 public:
  enum Decoration {
    NONE,
    UNDERLINE,
  };

  TextField(const Environment& env, TextFieldEditor& editor,
            const roo_display::Font& font, std::string hint,
            roo_display::HAlign halign, roo_display::VAlign valign,
            Decoration decoration)
      : Widget(env),
        editor_(editor),
        decoration_(decoration),
        value_(""),
        hint_(std::move(hint)),
        font_(font),
        text_color_(roo_display::color::Transparent),
        highlight_color_(roo_display::color::Transparent),
        halign_(halign),
        valign_(valign) {}

  bool isClickable() const override { return true; }
  bool showClickAnimation() const override { return false; }
  bool useOverlayOnPress() const override { return false; }

  virtual void onEditFinished(bool confirmed) {}

  const std::string& hint() const { return hint_; }

  PreferredSize getPreferredSize() const override {
    Padding p = getDefaultPadding();
    int16_t preferred_height =
        font_.metrics().maxHeight() + p.top() + p.bottom();
    switch (decoration_) {
      case UNDERLINE: {
        preferred_height += 6;
        break;
      }
      default: {
      }
    }
    return PreferredSize(PreferredSize::MatchParent(),
                         PreferredSize::Exact(preferred_height));
  }

  Padding getDefaultPadding() const override { return Padding(0, 2); }

  bool paint(const roo_display::Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override {
    auto metrics = font_.getHorizontalStringMetrics(
        (const uint8_t*)value_.c_str(), value_.size());
    int16_t preferred_height = metrics.height();
    switch (decoration_) {
      case UNDERLINE: {
        preferred_height += 6;
        break;
      }
      default: {
      }
    }
    return Dimensions(metrics.width(), preferred_height);
  }

  const std::string& content() const { return value_; }

  void setContent(std::string value) {
    if (value_ == value) return;
    value_ = std::move(value);
    if (isEdited()) editor().measure();
    markDirty();
  }

  const roo_display::Font& font() const { return font_; }

  TextFieldEditor& editor() const { return editor_; }

 private:
  friend class TextFieldEditor;

  bool isEdited() const { return editor_.isEdited(this); }

 public:
  void edit() { editor_.edit(this); }

 private:
  TextFieldEditor& editor_;

  Decoration decoration_;

  std::string value_;
  const std::string hint_;
  const roo_display::Font& font_;
  roo_display::Color text_color_;
  roo_display::Color highlight_color_;
  roo_display::HAlign halign_;
  roo_display::VAlign valign_;
};

}  // namespace roo_windows
