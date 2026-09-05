#include "roo_windows/material3/dialog/full_screen_dialog.h"

#include "roo_icons/outlined/24/navigation.h"
#include "roo_logging.h"

namespace roo_windows::material3 {

FullScreenDialog::CloseButton::CloseButton(ApplicationContext& context,
                                           FullScreenDialog& owner)
    : IconButton(context, ic_outlined_24_navigation_close(),
                 IconButtonStyle::kStandard),
      owner_(owner) {}

void FullScreenDialog::CloseButton::onClicked() {
  IconButton::onClicked();
  owner_.requestClose();
}

FullScreenDialog::ConfirmButton::ConfirmButton(ApplicationContext& context,
                                               FullScreenDialog& owner)
    : Button(context, {}, ButtonVariant::kText), owner_(owner) {
  setVisibility(Visibility::kGone);
}

void FullScreenDialog::ConfirmButton::onClicked() {
  Button::onClicked();
  owner_.requestConfirm();
}

FullScreenDialog::FullScreenDialog(ApplicationContext& context, WidgetRef body)
    : DialogScaffoldBase(context, std::move(body),
                         internal::DialogScaffoldVariant::kFullScreen),
      close_(context, *this),
      confirm_(context, *this) {
  attachDerivedChrome(internal::DialogChromeSlot::kPrimary, close_);
  attachDerivedChrome(internal::DialogChromeSlot::kSecondary, confirm_);
}

FullScreenDialog::~FullScreenDialog() { prepareForDerivedDestruction(); }

void FullScreenDialog::setConfirmAction(const DialogActionSpec& action) {
  CHECK(action.role == DialogActionRole::kConfirm);
  clearDialogRememberedFocus();
  confirm_action_ = action;
  has_confirm_action_ = true;
  confirm_.setLabel(confirm_action_.label);
  confirm_.setEnabled(confirm_action_.enabled);
  confirm_.setVisibility(Visibility::kVisible);
}

void FullScreenDialog::clearConfirmAction() {
  if (!has_confirm_action_) return;
  clearDialogRememberedFocus();
  has_confirm_action_ = false;
  confirm_.setVisibility(Visibility::kGone);
  confirm_.setLabel({});
}

void FullScreenDialog::setLayoutDirection(LayoutDirection direction) {
  setDialogLayoutDirection(direction);
  dialogTitle().setTextAlign(direction == LayoutDirection::kLeftToRight
                                 ? TextAlign::kStart
                                 : TextAlign::kEnd);
}

DialogShowResult FullScreenDialog::show(Task& interaction_owner) {
  if (isShowing()) return DialogShowResult::kAlreadyPresented;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  return showFullScreenDialogSurface(interaction_owner);
}

void FullScreenDialog::dismiss() {
  if (!isShowing()) return;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  finishDialog(PresentationFinishReason::kCancel);
}

void FullScreenDialog::requestClose() {
  if (!isShowing()) return;
  constexpr DialogDismissReason kReason = DialogDismissReason::kCloseButton;
  if (!onDismissRequested(kReason)) return;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = kReason;
  finishDialog(PresentationFinishReason::kCancel);
}

void FullScreenDialog::requestConfirm() {
  if (!isShowing() || !has_confirm_action_ || !confirm_action_.enabled) return;
  const uint8_t id = confirm_action_.id;
  if (!onConfirmRequested(id)) return;
  completion_kind_ = CompletionKind::kConfirm;
  finishDialog(PresentationFinishReason::kAction);
}

Widget* FullScreenDialog::preferredChromeFocusChild() {
  if (has_confirm_action_ && confirm_.isEnabled()) return &confirm_;
  return &close_;
}

BackResult FullScreenDialog::onDialogBackRequested(BackSource source) {
  const DialogDismissReason reason = DismissReasonFor(source);
  if (onDismissRequested(reason)) {
    completion_kind_ = CompletionKind::kDismiss;
    dismiss_reason_ = reason;
    finishDialog(PresentationFinishReason::kBack);
  }
  return BackResult::kHandled;
}

void FullScreenDialog::onDialogPresentationFinished(
    PresentationFinishReason reason) {
  const CompletionKind kind = completion_kind_;
  completion_kind_ = CompletionKind::kDismiss;
  if (kind == CompletionKind::kConfirm &&
      reason == PresentationFinishReason::kAction) {
    const uint8_t id = confirm_action_.id;
    onConfirmed(id);
    return;
  }
  const DialogDismissReason dismiss_reason =
      reason == PresentationFinishReason::kBack ||
              reason == PresentationFinishReason::kCancel
          ? dismiss_reason_
          : DismissReasonFor(reason);
  onDismissed(dismiss_reason);
}

DialogDismissReason FullScreenDialog::DismissReasonFor(BackSource source) {
  return source == BackSource::kEscapeKey ? DialogDismissReason::kEscape
                                          : DialogDismissReason::kBack;
}

DialogDismissReason FullScreenDialog::DismissReasonFor(
    PresentationFinishReason reason) {
  switch (reason) {
    case PresentationFinishReason::kOwnerDestroyed:
    case PresentationFinishReason::kInteractionOwnerDetached:
      return DialogDismissReason::kInteractionOwnerDetached;
    case PresentationFinishReason::kHostDestroyed:
      return DialogDismissReason::kHostDestroyed;
    case PresentationFinishReason::kBack:
      return DialogDismissReason::kBack;
    default:
      return DialogDismissReason::kProgrammatic;
  }
}

}  // namespace roo_windows::material3
