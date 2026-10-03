#pragma once

#include <functional>
#include <string>

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows {
namespace material3 {
class TextField;
}

/// Full-screen destination for editing a string with Back and Confirm actions.
/// Edits a temporary value; Back discards it, confirmation delivers it after
/// removal from navigation. Pushing it pauses the previous destination;
/// leaving it through Back or Confirm resumes that destination.
///
/// The caller owns this destination and must remove it from history before
/// destruction. Use it on the application's UI thread.
class EditorDestination : public Destination {
 public:
  /// Creates an editor using @p context and the initial placeholder @p hint.
  explicit EditorDestination(ApplicationContext& context,
                             const std::string& hint = "");

  /// Returns the root pane containing the editor and action buttons.
  Widget& getContents() override { return main_pane_; }

  /// Cancels this edit and removes its destination.
  BackResult onBackRequested(BackSource source) override;

  /// Starts or resumes keyboard input when this destination becomes current.
  void onResume() override;

  /// Releases the source and delivers completion after leaving history.
  void onRemoved() override;

  /// Pushes this destination onto @p navigation to edit a copy of @p initial.
  /// Uses @p hint as the empty-value placeholder and @p obscure to mask text.
  /// Confirmation invokes @p enter_fn after removal; cancellation discards it.
  /// An empty callback is allowed. Its string argument is borrowed for the
  /// duration of the call; captured objects must outlive navigation membership.
  ///
  /// Call on the application's UI thread with an available navigation host
  /// from the same application context. This destination must be inactive
  /// and outside completion delivery. A superseding navigation callback can
  /// prevent admission, in which case no edit or completion is delivered.
  void triggerEdit(NavigationHost& navigation, const std::string& initial,
                   const std::string& hint,
                   std::function<void(const std::string&)> enter_fn,
                   bool obscure = false);

  /// Edits a copy of a legacy field and writes it back on confirmation.
  /// Preserves masking and uses the field's task navigation and placeholder.
  /// The field must be attached and outlive this destination's navigation
  /// membership. The same admission and threading rules as triggerEdit apply.
  void triggerEditField(TextField& field);

 private:
  friend class material3::TextField;

  /// Forwards keyboard confirmation to the containing destination.
  class EditedTextField : public TextField {
   public:
    /// Creates the compact editor using @p context and placeholder @p hint,
    /// forwarding confirmation to the borrowed @p destination.
    EditedTextField(ApplicationContext& context, const std::string& hint,
                    EditorDestination& destination);

    /// Confirms the destination when the shared text editor receives Enter.
    void onEditFinished(bool confirmed) override;

   private:
    EditorDestination& destination_;
  };

  // Uses task-local keyboard geometry, excluding fields in transient hosts.
  static bool NeedsExtraction(material3::TextField& field);

  // Starts a draft for an attached source, or returns false while in use.
  bool triggerEditField(material3::TextField& field);

  // Drops the borrowed source before its owned text and edit hooks disappear.
  void forgetSource(material3::TextField& field);

  // Removes the current destination and delivers its confirmed draft.
  void confirm();

  VerticalLayout main_pane_;
  HorizontalLayout content_pane_;
  SimpleButton back_;
  EditedTextField text_;
  SimpleButton enter_;
  bool editing_ : 1;
  bool confirmed_ : 1;
  bool completing_ : 1;
  material3::TextField* source_ = nullptr;
  std::function<void(const std::string&)> enter_fn_;
};

}  // namespace roo_windows
