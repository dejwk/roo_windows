#pragma once

#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/key_source.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/core/task_panel.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

class Application;
class ApplicationInputRouter;
class DisplayWindow;
class Keyboard;

/// Owns task-local focus, editing, input routing, and fixed direct content.
class Task {
 public:
  using BackCallback = std::function<BackResult(BackSource)>;
  ~Task();

  Task(const Task&) = delete;
  Task& operator=(const Task&) = delete;
  Task(Task&&) = delete;
  Task& operator=(Task&&) = delete;

  /// Returns the permanently attached display window.
  DisplayWindow& window() { return window_; }
  const DisplayWindow& window() const { return window_; }
  /// Returns the application that owns this task.
  Application& application() { return app_; }
  const Application& application() const { return app_; }

  /// Returns the optional navigation host for a navigation task.
  NavigationHost* navigationHost() const { return navigation_; }

  /// Shows or hides this task layer without detaching its fixed content.
  void setVisible(bool visible);

  /// Returns focus state scoped to this task panel.
  FocusManager& focus() { return focus_; }
  const FocusManager& focus() const { return focus_; }

  /// Returns the editor scoped to this task.
  TextFieldEditor& textFieldEditor() { return editor_; }
  const TextFieldEditor& textFieldEditor() const { return editor_; }

  /// Configures this task's optional semantic Back handler. Passing an empty
  /// function clears it.
  void setBackCallback(BackCallback callback);

  /// Routes semantic Back through the display root and task content.
  BackResult requestBack(BackSource source = BackSource::kProgrammatic);

 private:
  friend class Application;
  friend class ApplicationInputRouter;
  friend class KeySource;
  friend class Widget;
  friend class Container;
  friend class NavigationHost;
  friend class Keyboard;

  Task(Application& app, DisplayWindow& window,
         const roo_display::Box& bounds, bool popup, Keyboard& keyboard,
         Widget& content);
  Task(Application& app, DisplayWindow& window,
         const roo_display::Box& bounds, bool popup, Keyboard& keyboard,
         NavigationHost& navigation);

  void dispatchKeyEvent(const KeyEvent& event);
  void cancelKeyActivation();
  void onSubtreeDetaching(Widget& subtree);
  void onWidgetFocusLost(Widget& widget);
  void attachNavigationContent(Widget& content);
  void detachNavigationContent();
  BackResult requestTaskBackCallback(BackSource source);

  Application& app_;
  DisplayWindow& window_;
  TaskPanel panel_;
  FocusManager focus_;
  TextFieldEditor editor_;
  bool popup_;
  Widget* armed_key_widget_ = nullptr;
  PhysicalKey armed_key_ = PhysicalKey::kNone;
  BackCallback back_callback_;
  NavigationHost* navigation_ = nullptr;
};

}  // namespace roo_windows
#include <functional>
#include <memory>

#include "roo_windows/core/back_request.h"
