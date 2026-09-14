#pragma once

#include <inttypes.h>
#include <pgmspace.h>
#include <stddef.h>

#include <memory>

#include "roo_windows/core/text_input.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/keyboard_layout/keyboard_layout.h"
#include "roo_windows/keyboard_layout/keyboard_layout_view.h"

namespace roo_windows {

class KeyboardWidget;
class Task;
class Application;

/// On-screen software keyboard widget controller.
///
/// Renders a borrowed generated layout with one surface widget. Layout bytes
/// must outlive the keyboard or remain alive until replaced by setLayout().
/// The legacy spec constructor is retained for custom-layout migration.
///
/// Emits semantic text input to its connected application. Tracks caps
/// state (`LOW`, `HIGH`, `HIGH_LOCKED`) and current page. Its fixed popup task
/// owns placement; show and hide only change widget visibility.
class Keyboard {
 public:
  enum CapsState {
    CAPS_STATE_LOW = 0,
    CAPS_STATE_HIGH = 1,
    CAPS_STATE_HIGH_LOCKED = 2,
  };

  Keyboard(ApplicationContext& context, const KeyboardSpec* spec);

  /// Borrows validated layout bytes, which must outlive this keyboard.
  Keyboard(ApplicationContext& context, KeyboardLayoutView layout);

  /// Replaces the borrowed layout, canceling input and resetting page/caps.
  /// Preserves visibility. Call on the application's UI thread after startup.
  void setLayout(KeyboardLayoutView layout);

  /// Returns the underlying `KeyboardWidget` that renders the layout.
  Widget& getContents();

  /// Returns the keyboard widget without changing its state.
  const Widget& getContents() const;

  /// Routes this keyboard's semantic input to `destination`, replacing any
  /// previous destination. The caller must use a UI thread shared with
  /// `destination` after it has started.
  void connect(Application& destination);
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
  TextInputEmitter text_input_;
  Task* task_ = nullptr;
};

}  // namespace roo_windows
