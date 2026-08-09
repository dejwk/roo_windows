#pragma once

#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/key_source.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

class Application;
class DisplayWindow;
class Keyboard;

/// Result of attaching one polled key source to a display-local task.
enum class KeySourceAttachmentResult : uint8_t {
  kAttached,
  kSourceAlreadyAttached,
  kTaskAlreadyHasSource,
  kWrongThread,
};

/// Owns task-local focus, editing, input routing, and a legacy activity stack.
class UiTask {
 public:
  ~UiTask();

  UiTask(const UiTask&) = delete;
  UiTask& operator=(const UiTask&) = delete;
  UiTask(UiTask&&) = delete;
  UiTask& operator=(UiTask&&) = delete;

  /// Returns the permanently attached display window.
  DisplayWindow& window() { return window_; }
  const DisplayWindow& window() const { return window_; }

  /// Returns focus state scoped to this task panel.
  FocusManager& focus() { return focus_; }
  const FocusManager& focus() const { return focus_; }

  /// Returns the editor scoped to this task.
  TextFieldEditor& textFieldEditor() { return editor_; }
  const TextFieldEditor& textFieldEditor() const { return editor_; }

  /// Attaches `source` when both source and task have no attachment.
  KeySourceAttachmentResult attachKeySource(KeySource& source);
  /// Detaches the current polled source, if any.
  void detachKeySource();

  /// Routes semantic Back through the display root then the legacy adapter.
  BackResult requestBack(BackSource source = BackSource::kProgrammatic);

  /// Returns the temporary activity-stack compatibility adapter.
  Task& legacyActivities() { return legacy_task_; }
  const Task& legacyActivities() const { return legacy_task_; }

 private:
  friend class Application;
  friend class KeySource;
  friend class Container;

  UiTask(Application& app, DisplayWindow& window,
         const roo_display::Box& bounds, bool popup, Keyboard& keyboard);

  bool drainKeyEvents();
  void dispatchKeyEvent(const KeyEvent& event);
  void onSubtreeDetaching(Widget& subtree);
  void onKeySourceDestroyed(KeySource& source);

  Application& app_;
  DisplayWindow& window_;
  Task legacy_task_;
  TaskPanel panel_;
  FocusManager focus_;
  TextFieldEditor editor_;
  bool popup_;
  KeySource* key_source_ = nullptr;
  Widget* armed_key_widget_ = nullptr;
  KeyCode armed_key_ = KeyCode::kUnknown;
};

}  // namespace roo_windows
