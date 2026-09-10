#pragma once

#include <stdint.h>

#include <string>

#include "roo_windows/core/destination.h"
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
/// run only after the navigation host has removed the destination.
class FullScreenDialog : public internal::DialogScaffold {
 public:
  /// Creates a full-screen dialog with persistent generic body content.
  FullScreenDialog(ApplicationContext& context, WidgetRef body);

  /// Removes a current dialog without completion, then detaches its controls.
  /// A covered dialog must be removed from history before destruction.
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

  /// Pushes this dialog into the owner's navigation history.
  /// Requires a navigation task; fills that task's bounds. Use a full-screen
  /// task for a full-window dialog. Layout runs on the next refresh.
  DialogShowResult show(Task& interaction_owner);

  /// Returns whether the dialog belongs to history, including while covered.
  bool isShowing() const;

  /// Returns whether this dialog is the current navigation destination.
  bool isCurrent() const;

  /// Dismisses the current dialog, bypassing veto. No-op when not showing.
  /// Dismissing a covered dialog is a contract violation.
  void dismiss();

 protected:
  /// Call before inline body/chrome members die. Safe to call repeatedly.
  void prepareForDerivedDestruction() override;

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
    ClickActivationPolicy getClickActivationPolicy() const override {
      return ClickActivationPolicy::kAfterForcedFinalFrame;
    }

    void onClicked() override;

   private:
    FullScreenDialog& owner_;
  };

  class ConfirmButton final : public Button {
   public:
    ConfirmButton(ApplicationContext& context, FullScreenDialog& owner);

    /// Requests vetoable confirmation.
    ClickActivationPolicy getClickActivationPolicy() const override {
      return ClickActivationPolicy::kAfterForcedFinalFrame;
    }

    void onClicked() override;

   private:
    FullScreenDialog& owner_;
  };

  enum class CompletionKind : uint8_t { kDismiss, kConfirm };

  static DialogDismissReason DismissReasonFor(BackSource source);

  void requestClose();
  void requestConfirm();
  BackResult requestBack(BackSource source);
  void complete();

  class DialogDestination final : public Destination {
   public:
    explicit DialogDestination(FullScreenDialog& owner) : owner_(owner) {}
    Widget& getContents() override { return owner_; }
    BackResult onBackRequested(BackSource source) override {
      return owner_.requestBack(source);
    }
    void onStop() override;
    void onRemoved() override { owner_.complete(); }

   private:
    FullScreenDialog& owner_;
  };

  CloseButton close_;
  ConfirmButton confirm_;
  DialogActionSpec confirm_action_{};
  CompletionKind completion_kind_ = CompletionKind::kDismiss;
  DialogDismissReason dismiss_reason_ = DialogDismissReason::kProgrammatic;
  bool has_confirm_action_ = false;
  bool destroying_ = false;
  DialogDestination destination_;
};

}  // namespace roo_windows::material3
