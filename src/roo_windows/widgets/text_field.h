#pragma once

#include "roo_backport/string_view.h"
#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/tile.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/config.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class TextField;

static constexpr roo_time::Duration kCursorBlinkInterval =
    roo_time::Millis(500);

static constexpr roo_time::Duration kShowLastGlyphInterval =
    roo_time::Millis(1500);

/// Eye / eye-slash style toggle used by `TextField` to reveal a masked value.
///
/// Acts as an on/off `BasicWidget`: clicking toggles, paint() chooses the
/// matching pictogram. Owns no application state of its own; the consuming
/// `TextField` reacts to the state change.
class VisibilityToggle : public BasicWidget {
 public:
  VisibilityToggle(const Environment& env) : BasicWidget(env) { setOff(); }

  /// Reports the fixed icon-sized footprint used to draw the eye/eye-slash
  /// pictograms.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
  }

  bool isClickable() const override { return true; }

  /// Toggles the on/off state and then runs the base click behavior so the
  /// owning text field can react.
  void onClicked() override {
    toggle();
    Widget::onClicked();
  }

  /// Selects the eye or eye-slash pictogram based on the current state and
  /// paints it centered.
  void paint(PaintContext& ctx) const override;

  using Widget::isOff;
  using Widget::isOn;
  using Widget::setOff;
  using Widget::setOn;
  using Widget::toggle;
};

/// Shared editing controller for `TextField` widgets.
///
/// One editor is owned by the `Application` and bound to whichever
/// `TextField` currently has focus. It receives keyboard input as a
/// `KeyboardListener`, maintains the cursor position, selection range, glyph
/// metrics cache, and horizontal scroll offset, and drives cursor blinking
/// and "recently entered glyph" reveal timers via the scheduler. Keeping this
/// state centrally avoids paying for it on every `TextField` instance.
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

  /// Begins editing `target` as the active field; any previously active
  /// field is implicitly committed and unbound first.
  void edit(TextField* target);

  /// True iff this editor is currently bound to `target`.
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

  /// Sets the inclusive selection range in glyph indices and updates the
  /// caret position to the selection end.
  void setSelection(int16_t selection_begin, int16_t selection_end);

  /// Inserts (or overwrites the current selection with) a single Unicode
  /// code point at the caret and advances it.
  void rune(uint32_t rune) override;

  /// Confirms the edit and notifies the target's `onEditFinished(true)`.
  void enter() override;

  /// Deletes the current selection, or the glyph before the caret if no
  /// selection is active.
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

/// Single-line editable text widget.
///
/// Holds the current value, hint, font, color hints, alignment, and an
/// optional decoration (e.g. underline). All editing state (cursor, selection,
/// scroll) is delegated to the shared `TextFieldEditor`, so individual fields
/// stay lightweight. Set `setStarred(true)` to render the value as bullet
/// characters for password-style fields.
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

  /// Hook for subclasses; called when the editor finishes editing this field
  /// (`confirmed` is true on Enter, false on cancellation/blur).
  virtual void onEditFinished(bool confirmed) {}

  /// Routes a click into `edit()` so tapping the field starts editing.
  void onClicked() override {
    edit();
    Widget::onClicked();
  }

  /// Toggles password-style bullet rendering on the value.
  void setStarred(bool starred) {
    if (starred == starred_) return;
    starred_ = starred;
    if (isEdited()) editor().measure();
    setDirty();
  }

  const std::string& hint() const { return hint_; }

  /// Replaces the placeholder text shown when the value is empty.
  void setHint(roo::string_view hint) {
    hint_ = std::string((const char*)hint.data(), hint.size());
  }

  /// Reports a match-parent width and a fixed height matching the font's max
  /// glyph height plus any decoration (e.g. underline).
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

  /// Paints the value (or hint), selection highlight, and blinking caret
  /// when this field is the active editor target.
  void paint(PaintContext& ctx) const override;

  /// Reports the font-measured width and the same fixed height used by
  /// `getPreferredSize()`.
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

  /// Replaces the value. Triggers an editor re-measure when this field is
  /// the active editor target.
  void setContent(std::string value) {
    if (value_ == value) return;
    value_ = std::move(value);
    if (isEdited()) editor().measure();
    setDirty();
  }

  const roo_display::Font& font() const { return font_; }

  TextFieldEditor& editor() const { return editor_; }

 private:
  friend class TextFieldEditor;

  bool isEdited() const { return editor_.isEdited(this); }
  bool isStarred() const { return starred_; }

 public:
  /// Binds this field to the shared editor and starts editing.
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
