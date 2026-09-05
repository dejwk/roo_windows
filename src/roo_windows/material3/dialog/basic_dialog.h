#pragma once

#include <stdint.h>

#include <string>

#include "roo_windows/material3/dialog/dialog_scaffold.h"

namespace roo_windows::material3 {

/// Centered Material 3 dialog with generic persistent body content.
///
/// The dialog copies one acknowledgement descriptor or one dismiss/confirm
/// pair into fixed storage. Body ownership follows `WidgetRef`: borrowed bodies
/// must outlive the dialog attachment; adopted bodies are deleted when replaced
/// or when the dialog is destroyed. Dismissal alone preserves the body.
class BasicDialog : public internal::DialogScaffoldBase,
                    private internal::DialogActionDelegate {
 public:
  /// Creates a basic dialog and validates its fixed action model.
  BasicDialog(ApplicationContext& context, WidgetRef body,
              const DialogActionSpec* actions, uint8_t action_count);

  /// Cancels hosting and detaches body/chrome before inline members die.
  ~BasicDialog() override;

  /// Sets the optional borrowed leading icon.
  void setIcon(const MonoIcon* icon);

  /// Clears the optional icon.
  void clearIcon();

  /// Replaces the owned, wrapping headline text.
  void setHeadline(std::string headline) {
    setDialogTitle(std::move(headline));
  }

  /// Replaces the persistent generic body and ends the previous borrow or
  /// adoption.
  void setBody(WidgetRef body) { setDialogBody(std::move(body)); }

  /// Updates one action's enabled state under the role constraints.
  void setActionEnabled(uint8_t action_id, bool enabled) {
    actions_.setActionEnabled(action_id, enabled);
  }

  /// Selects explicit logical headline and action ordering.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns the explicit logical layout direction.
  LayoutDirection layoutDirection() const { return dialogLayoutDirection(); }

  /// Attempts to show this dialog through `interaction_owner`'s shared host.
  DialogShowResult show(Task& interaction_owner);

  /// Returns whether this dialog currently occupies the shared host.
  bool isShowing() const { return isDialogShowing(); }

  /// Unconditionally dismisses an active dialog as programmatic dismissal.
  void dismiss();

 protected:
  /// Receives an enabled action after the dialog has completely detached.
  virtual void onActionInvoked(uint8_t action_id, DialogActionRole role) {
    (void)action_id;
    (void)role;
  }

  /// Receives non-action completion after the dialog has completely detached.
  virtual void onDismissed(DialogDismissReason reason) { (void)reason; }

 private:
  enum class CompletionKind : uint8_t { kDismiss, kAction };

  static DialogDismissReason DismissReasonFor(BackSource source);
  static DialogDismissReason DismissReasonFor(PresentationFinishReason reason);

  void invokeDialogAction(uint8_t id, DialogActionRole role) override;
  Widget* preferredChromeFocusChild() override;
  BackResult onDialogBackRequested(BackSource source) override;
  void onDialogPresentationFinished(PresentationFinishReason reason) override;

  internal::DialogActionStrip actions_;
  CompletionKind completion_kind_ = CompletionKind::kDismiss;
  DialogDismissReason dismiss_reason_ = DialogDismissReason::kProgrammatic;
  uint8_t action_id_ = 0;
  DialogActionRole action_role_ = DialogActionRole::kAcknowledge;
};

namespace internal {

/// Ordered storage base that outlives AlertDialog's borrowed body attachment.
class AlertDialogBodyStorage {
 protected:
  AlertDialogBodyStorage(ApplicationContext& context,
                         std::string supporting_text);

  TextBlock supporting_text_;
};

}  // namespace internal

/// Convenience basic dialog with owned headline and supporting prose.
///
/// Its supporting-text widget is permanently installed as the body. Private
/// inheritance intentionally prevents replacing that body through a
/// `BasicDialog` upcast.
class AlertDialog : private internal::AlertDialogBodyStorage,
                    private BasicDialog {
 public:
  /// Creates an alert with owned text and validated fixed actions.
  AlertDialog(ApplicationContext& context, std::string headline,
              std::string supporting_text, const DialogActionSpec* actions,
              uint8_t action_count);

  using BasicDialog::clearIcon;
  using BasicDialog::dismiss;
  using BasicDialog::isShowing;
  using BasicDialog::layoutDirection;
  using BasicDialog::setActionEnabled;
  using BasicDialog::setHeadline;
  using BasicDialog::setIcon;
  using BasicDialog::setLayoutDirection;
  using BasicDialog::show;

  /// Replaces the owned, wrapping supporting prose.
  void setSupportingText(std::string supporting_text);

 protected:
  using BasicDialog::onActionInvoked;
  using BasicDialog::onDismissed;
};

}  // namespace roo_windows::material3
