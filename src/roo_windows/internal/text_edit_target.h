#pragma once

#include <string>

#include "roo_backport/string_view.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {
namespace internal {

/// Connects a single-line text widget to its task-owned TextFieldEditor.
///
/// The editor borrows the target, its widget and its live UTF-8 value; it does
/// not own them or keep a separate editing buffer. Implementations must end
/// their active session before destroying the target or its backing storage.
/// All access and callbacks occur on the application's UI thread.
///
/// Accessors must not change the edit session or invoke application callbacks.
/// Visual invalidation and value-change notifications are separate so that
/// caret, selection and masking updates do not report text changes.
class TextEditTarget {
 public:
  /// Destroys an already-unbound target; does not end an active edit session.
  virtual ~TextEditTarget() = default;

  /// Returns the widget whose lifetime, presentation and animations govern
  /// this target. Its identity must remain stable throughout a session.
  ///
  /// The editor uses animation tag 0 for the caret. The widget must forward
  /// those samples from onAnimationFrame() to
  /// TextFieldEditor::applyCursorFrame().
  virtual Widget& editWidget() = 0;

  /// Returns the live string that the editor may insert into or erase from.
  /// The string object must remain valid throughout the edit session.
  ///
  /// Its contents must be valid single-line UTF-8 and match value(). External
  /// replacements during editing must refresh the editor's metrics through
  /// TextFieldEditor::resetMetrics() before further input or painting.
  /// Mutating this reference does not itself dispatch change notifications.
  virtual std::string& textBuffer() = 0;

  /// Returns a borrowed, read-only view of the same UTF-8 contents as
  /// textBuffer(). This accessor must not allocate or construct a temporary
  /// backing string. The view may be invalidated by any buffer mutation or
  /// target destruction; callers must not retain it across either.
  virtual roo::string_view value() const = 0;

  /// Returns the font used to measure editable text, not labels or hints.
  /// The font must outlive any editor access. Changes during a session require
  /// TextFieldEditor::refreshMetrics(), which preserves selection and caret.
  virtual const roo_display::Font& textFont() const = 0;

  /// Returns the layout options used when measuring the editable text.
  /// Defaults to the font's default options. Changes during a session require
  /// TextFieldEditor::refreshMetrics(), as with textFont().
  virtual roo_display::Font::Options textFontOptions() const { return {}; }

  /// Returns whether text is displayed using one mask glyph per code point.
  /// Masking never changes the stored value or its UTF-8 editing offsets.
  /// The editor may briefly reveal a newly entered final code point.
  /// Changes during a session require TextFieldEditor::refreshMetrics().
  virtual bool obscureText() const = 0;

  /// Updates scrolling and invalidates affected pixels after an editing
  /// visual change, including caret, selection, masking and session changes.
  ///
  /// This hook must not dispatch application callbacks, destroy the target,
  /// or change the active session: the editor may continue accessing it.
  virtual void notifyEditVisualChange() = 0;

  /// Reports an editor mutation after metrics and visual state are updated.
  /// Not called for selection, caret or masking changes alone. The default
  /// implementation does nothing.
  ///
  /// May invoke application callbacks that destroy the target or change the
  /// active session; the editor does not access this target afterward in the
  /// operation delivering this notification.
  virtual void notifyTextChanged() {}

  /// Reports completion after this target is unbound and its caret animation
  /// and masking timer are stopped. A replacement session may already exist.
  ///
  /// @param confirmed True for confirmation; false for cancellation or
  /// replacement. Neither result rolls back the live value.
  ///
  /// May destroy the target or start another edit session. The editor makes
  /// no further accesses to this target in the operation completing it.
  virtual void onEditFinished(bool confirmed) = 0;
};

}  // namespace internal
}  // namespace roo_windows
