#pragma once

#include <vector>

#include "roo_scheduler.h"
#include "roo_windows/internal/text_edit_target.h"

namespace roo_windows {

class Application;

/// Shared, design-system-neutral editing controller for one `Task`.
///
/// One editor is owned by each `Task` and borrows the active `TextEditTarget`,
/// including Material 2, Material 3, or custom fields. It receives semantic
/// input, maintains the cursor position, selection range, glyph metrics cache,
/// and horizontal scroll offset. Caret blinking uses a sparse animation track
/// on the active field; the one-shot "recently entered glyph" reveal deadline
/// remains scheduler-owned. Keeping this state centrally avoids paying for it
/// on every `TextField` instance.
class TextFieldEditor {
 public:
  /// Creates a task editor using @p application and its @p scheduler.
  TextFieldEditor(Application& application,
                  roo_scheduler::SchedulerClient& scheduler)
      : application_(application),
        scheduler_(scheduler),
        blinking_cursor_is_on_(false),
        last_glyph_hider_(scheduler, [this]() { hideLastGlyph(); }),
        last_glyph_recently_entered_(false),
        target_(nullptr),
        cursor_position_(0),
        selection_begin_(0),
        selection_end_(0),
        selection_anchor_(0),
        draw_xoffset_(0) {}

  /// Cancels any active session and its timers before destruction.
  ~TextFieldEditor();

  /// Begins editing @p target, borrowing it until completion or cancellation.
  /// Unbinds the previous target before publishing the new session, then
  /// delivers its onEditFinished(false). Pass nullptr to cancel. All calls
  /// occur on the application UI thread; optional keyboard display is shared.
  void edit(internal::TextEditTarget* target,
            bool show_software_keyboard = true);

  /// True iff this editor is currently bound to `target`.
  bool isEdited(const internal::TextEditTarget* target) const;

  /// Returns the bound target widget, or nullptr outside an edit session.
  Widget* editedWidget() const {
    return target_ == nullptr ? nullptr : &target_->editWidget();
  }

  /// Returns whether the final newly typed glyph is temporarily revealed.
  bool lastGlyphRecentlyEntered() const { return last_glyph_recently_entered_; }

  /// Returns cached visual glyph metrics for the active value.
  const std::vector<roo_display::GlyphMetrics>& glyphs() const {
    return glyphs_;
  }

  /// Returns the current caret blink phase.
  bool isBlinkingCursorNowOn() const { return blinking_cursor_is_on_; }

  /// Returns the horizontal pixel offset that keeps the caret visible.
  int16_t draw_xoffset() const { return draw_xoffset_; }

  /// Returns the first selected glyph index.
  int16_t selection_begin() const { return selection_begin_; }

  /// Returns the exclusive end of the selected glyph range.
  int16_t selection_end() const { return selection_end_; }

  /// Returns the caret insertion index in glyphs.
  int16_t cursor_position() const { return cursor_position_; }

  /// Returns whether a nonempty glyph range is selected.
  bool has_selection() const { return selection_end_ > selection_begin_; }

  /// Returns true when the active editing target belongs to `subtree`.
  bool targetInSubtree(const Widget& subtree) const;

  /// Sets the half-open selection range in glyph indices and updates the
  /// caret position to the selection end.
  void setSelection(int16_t selection_begin, int16_t selection_end);

  /// Inserts (or overwrites the current selection with) a single Unicode
  /// code point at the caret and advances it.
  void rune(uint32_t rune);

  /// Confirms the edit and notifies the target's `onEditFinished(true)`.
  void enter();

  /// Deletes the current selection, or the glyph before the caret if no
  /// selection is active.
  void del();

  /// Finishes without confirmation; the live text value is not rolled back.
  void cancel();

  /// Deletes the selection or the glyph following the caret.
  void forwardDelete();

  /// Moves left one glyph, optionally extending selection from its anchor.
  void moveLeft(bool extend_selection = false);

  /// Moves right one glyph, optionally extending selection from its anchor.
  void moveRight(bool extend_selection = false);

  /// Moves to the beginning, optionally extending selection from its anchor.
  void moveHome(bool extend_selection = false);

  /// Moves to the end, optionally extending selection from its anchor.
  void moveEnd(bool extend_selection = false);

  /// Applies a registry sample to the matching active target.
  void applyCursorFrame(internal::TextEditTarget& target,
                        const AnimationSample& sample);

  /// Remeasures masking/font state while retaining selection and caret.
  void refreshMetrics();

  /// Remeasures a replaced value, clears selection and cancels recent reveal.
  void resetMetrics();

  /// Adjusts the horizontal offset to keep the caret in the given viewport.
  void ensureCursorVisible(int16_t viewport_width);

 private:
  void measure();
  void restartCursor();
  void stopCursor(internal::TextEditTarget& target);

  void restartLastGlyphRecentlyEntered();
  void hideLastGlyph();
  void moveCursor(int16_t position, bool extend_selection);
  void finish(bool confirmed);
  void changed();

  Application& application_;
  roo_scheduler::SchedulerClient& scheduler_;
  bool blinking_cursor_is_on_;
  roo_scheduler::SingletonTask last_glyph_hider_;

  // True if the last (rightmost) glyph is considered as 'recently entered'
  // and meant to be shown.
  bool last_glyph_recently_entered_;

  internal::TextEditTarget* target_;
  std::vector<roo_display::GlyphMetrics> glyphs_;
  std::vector<int16_t> offsets_;
  int16_t cursor_position_;
  int16_t selection_begin_;
  int16_t selection_end_;
  int16_t selection_anchor_;
  int16_t draw_xoffset_;
};

}  // namespace roo_windows
