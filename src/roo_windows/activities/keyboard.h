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

/// Sink for low-level keyboard events delivered by `Keyboard`.
///
/// Implementations decide how to translate runes, enter, and delete events
/// into edits on whatever buffer they own (typically a `TextFieldEditor`).
class KeyboardListener {
 public:
  virtual ~KeyboardListener() {}

  /// Called for a printable character keypress.
  virtual void rune(uint32_t rune) = 0;
  /// Called when the Enter / commit key is pressed.
  virtual void enter() = 0;
  /// Called when the backspace / delete key is pressed.
  virtual void del() = 0;
};

class KeyboardWidget;
class Task;

/// On-screen software keyboard widget controller.
///
/// Renders the layout supplied at construction (regular / numeric / etc.)
/// and forwards key events to the currently-bound `KeyboardListener`. Tracks
/// caps state (`LOW`, `HIGH`, `HIGH_LOCKED`) and current page. Its fixed
/// popup task owns placement; show and hide only change widget visibility.
class Keyboard {
 public:
  enum CapsState {
    CAPS_STATE_LOW = 0,
    CAPS_STATE_HIGH = 1,
    CAPS_STATE_HIGH_LOCKED = 2,
  };

  Keyboard(ApplicationContext& context, const KeyboardSpec* spec);

  /// Returns the underlying `KeyboardWidget` that renders the layout.
  Widget& getContents();

  /// Routes future key events to the supplied listener (may be nullptr).
  void setListener(KeyboardListener* listener);
  void setTask(Task& task) { task_ = &task; }

  /// Makes the fixed keyboard popup visible.
  void show();
  /// Hides the fixed keyboard popup.
  void hide();

  /// Switches to the keyboard page at the supplied index (e.g. symbols).
  void setPage(int idx);
  /// Returns the current caps state.
  CapsState caps_state() const;
  /// Updates the caps state, refreshing the rendered key glyphs.
  void setCapsState(CapsState caps_state);

 private:
  KeyboardWidget* contents();
  const KeyboardWidget* contents() const;

  std::unique_ptr<Widget> contents_;
  Task* task_ = nullptr;
};

}  // namespace roo_windows
