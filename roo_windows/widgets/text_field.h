#pragma once

#include "roo_windows/config.h"

#include "roo_display/core/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class TextField;

static constexpr roo_time::Interval kCursorBlinkInterval =
    roo_time::Millis(500);

static constexpr roo_time::Interval kShowLastGlyphInterval =
    roo_time::Millis(1500);

class VisibilityToggle : public BasicWidget {
 public:
  VisibilityToggle(const Environment& env) : BasicWidget(env) { setOff(); }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
  }

  bool isClickable() const override { return true; }

  void onClicked() override {
    toggle();
    Widget::onClicked();
  }

  void paint(const Canvas& canvas) const override;

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;
};

class TextFieldEditor : public KeyboardListener {
 public:
  TextFieldEditor(roo_scheduler::Scheduler& scheduler, Keyboard& keyboard)
      : scheduler_(scheduler),
        keyboard_(keyboard),
        cursor_blinker_(scheduler, [this]() { blinkCursor(); }),
        blinking_cursor_is_on_(false),
        last_cursor_shown_time_(roo_time::Uptime::Now()),
        last_glyph_hider_(scheduler, [this]() { hideLastGlyph(); }),
        last_glyph_recently_entered_(false),
        target_(nullptr),
        cursor_position_(0),
        selection_begin_(0),
        selection_end_(0),
        draw_xoffset_(0) {}

  void edit(TextField* target);

  bool isEdited(const TextField* target) const;

  bool lastGlyphRecentlyEntered() const { return last_glyph_recently_entered_; }

  const std::vector<roo_display::GlyphMetrics>& glyphs() const {
    return glyphs_;
  }

  bool isBlinkingCursorNowOn() const { return blinking_cursor_is_on_; }

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
  void restartCursor();
  void blinkCursor();

  void restartLastGlyphRecentlyEntered();
  void hideLastGlyph();

  roo_scheduler::Scheduler& scheduler_;
  Keyboard& keyboard_;
  roo_scheduler::SingletonTask cursor_blinker_;
  bool blinking_cursor_is_on_;
  roo_time::Uptime last_cursor_shown_time_;
  roo_scheduler::SingletonTask last_glyph_hider_;

  // True if the last (rightmost) glyph is considered as 'recently entered'
  // and meant to be shown.
  bool last_glyph_recently_entered_;

  TextField* target_;
  std::vector<roo_display::GlyphMetrics> glyphs_;
  std::vector<int16_t> offsets_;
  int16_t cursor_position_;
  int16_t selection_begin_;
  int16_t selection_end_;
  int16_t draw_xoffset_;
};

class TextField : public BasicWidget {
 public:
  enum Decoration {
    NONE,
    UNDERLINE,
  };

  TextField(const Environment& env, TextFieldEditor& editor,
            const roo_display::Font& font, std::string hint,
            roo_display::Alignment alignment, Decoration decoration)
      : BasicWidget(env),
        editor_(editor),
        decoration_(decoration),
        value_(""),
        hint_(std::move(hint)),
        starred_(false),
        font_(font),
        text_color_(roo_display::color::Transparent),
        highlight_color_(roo_display::color::Transparent),
        alignment_(alignment) {}

  bool isClickable() const override { return true; }
  bool showClickAnimation() const override { return false; }
  bool useOverlayOnPress() const override { return false; }

  virtual void onEditFinished(bool confirmed) {}

  void onClicked() override {
    edit();
    Widget::onClicked();
  }

  void setStarred(bool starred) {
    if (starred == starred_) return;
    starred_ = starred;
    if (isEdited()) editor().measure();
    markDirty();
  }

  const std::string& hint() const { return hint_; }

  void setHint(roo_display::StringView hint) {
    hint_ = std::string((const char*)hint.data(), hint.size());
  }

  PreferredSize getPreferredSize() const override {
    Padding p = getPadding();
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
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::ExactHeight(preferred_height));
  }

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override {
    auto metrics = font_.getHorizontalStringMetrics(value_);
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
  bool isStarred() const { return starred_; }

 public:
  void edit() { editor_.edit(this); }

 private:
  TextFieldEditor& editor_;

  Decoration decoration_;

  std::string value_;
  std::string hint_;
  bool starred_;
  const roo_display::Font& font_;
  roo_display::Color text_color_;
  roo_display::Color highlight_color_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows
