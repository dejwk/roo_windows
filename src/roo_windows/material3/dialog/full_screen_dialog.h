#pragma once

#include <stdint.h>

#include <string>

#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/dialog/dialog_scaffold.h"

namespace roo_windows::material3 {

namespace test {
class DialogTestAccess;
}

/// Full-window Material 3 dialog for compact editing and wizard flows.
///
/// Close, Back, Escape, and optional confirmation first consult synchronous
/// veto hooks. `dismiss()` bypasses those hooks. Accepted completion callbacks
/// run only after the shared host has detached the dialog root.
class FullScreenDialog : public internal::DialogScaffoldBase {
 public:
  /// Creates a full-screen dialog with persistent generic body content.
  FullScreenDialog(ApplicationContext& context, WidgetRef body);

  /// Cancels hosting and detaches body/header controls before members die.
  ~FullScreenDialog() override;

  /// Replaces the owned short header title.
  void setHeaderTitle(std::string title) { setDialogTitle(std::move(title)); }

  /// Clears the optional header title.
  void clearHeaderTitle() { setDialogTitle({}); }

  /// Replaces the persistent body and ends the previous borrow or adoption.
  void setBody(WidgetRef body) { setDialogBody(std::move(body)); }

  /// Installs or replaces the optional copied confirming action.
  ///
  /// `action.role` must be `kConfirm`; its label remains borrowed.
  void setConfirmAction(const DialogActionSpec& action);

  /// Removes the optional confirming action.
  void clearConfirmAction();

  /// Selects explicit logical header ordering.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns the explicit logical layout direction.
  LayoutDirection layoutDirection() const { return dialogLayoutDirection(); }

  /// Attempts to show this full-window root through the owner's shared host.
  DialogShowResult show(Task& interaction_owner);

  /// Returns whether this dialog currently occupies the shared host.
  bool isShowing() const { return isDialogShowing(); }

  /// Unconditionally dismisses an active dialog as programmatic dismissal.
  void dismiss();

 protected:
  /// Decides whether a close, Back, or Escape request may dismiss the dialog.
  virtual bool onDismissRequested(DialogDismissReason reason) {
    (void)reason;
    return true;
  }

  /// Decides whether the optional confirming action may close the dialog.
  virtual bool onConfirmRequested(uint8_t action_id) {
    (void)action_id;
    return true;
  }

  /// Receives accepted non-confirm completion after structural detachment.
  virtual void onDismissed(DialogDismissReason reason) { (void)reason; }

  /// Receives accepted confirmation after structural detachment.
  virtual void onConfirmed(uint8_t action_id) { (void)action_id; }

 private:
  friend class test::DialogTestAccess;

  class CloseButton final : public IconButton {
   public:
    CloseButton(ApplicationContext& context, FullScreenDialog& owner);

    /// Requests vetoable close-button dismissal.
    void onClicked() override;

   private:
    FullScreenDialog& owner_;
  };

  class ConfirmButton final : public Button {
   public:
    ConfirmButton(ApplicationContext& context, FullScreenDialog& owner);

    /// Requests vetoable confirmation.
    void onClicked() override;

   private:
    FullScreenDialog& owner_;
  };

  enum class CompletionKind : uint8_t { kDismiss, kConfirm };

  static DialogDismissReason DismissReasonFor(BackSource source);
  static DialogDismissReason DismissReasonFor(PresentationFinishReason reason);

  void requestClose();
  void requestConfirm();
  Widget* preferredChromeFocusChild() override;
  BackResult onDialogBackRequested(BackSource source) override;
  void onDialogPresentationFinished(PresentationFinishReason reason) override;

  CloseButton close_;
  ConfirmButton confirm_;
  DialogActionSpec confirm_action_{};
  CompletionKind completion_kind_ = CompletionKind::kDismiss;
  DialogDismissReason dismiss_reason_ = DialogDismissReason::kProgrammatic;
  bool has_confirm_action_ = false;
};

}  // namespace roo_windows::material3
