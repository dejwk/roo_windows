#include "roo_windows/material3/dialog/full_screen_dialog.h"

#include "roo_icons/outlined/24/navigation.h"
#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"

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
    : DialogScaffold(context, std::move(body),
                     internal::DialogScaffoldVariant::kFullScreen),
      close_(context, *this),
      confirm_(context, *this),
      destination_(*this) {
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
  NavigationHost* navigation = &interaction_owner.navigation();
  if (!navigation->isAvailable())
    return DialogShowResult::kInteractionOwnerUnavailable;
  if (destroying_ || parent() != nullptr ||
      &context() != &interaction_owner.application().context())
    return DialogShowResult::kSurfaceUnavailable;
  if (interaction_owner.window()
          .root()
          .transient_presentation_slot()
          .hasActivePresentation())
    return DialogShowResult::kHostBusy;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  navigation->push(destination_);
  return isShowing() ? DialogShowResult::kShown
                     : DialogShowResult::kSurfaceUnavailable;
}

void FullScreenDialog::dismiss() {
  if (!isShowing()) return;
  CHECK(isCurrent());
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  destination_.exit();
}

void FullScreenDialog::requestClose() {
  if (!isCurrent()) return;
  constexpr DialogDismissReason kReason = DialogDismissReason::kCloseButton;
  if (!onDismissRequested(kReason)) return;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = kReason;
  destination_.exit();
}

void FullScreenDialog::requestConfirm() {
  if (!isCurrent() || !has_confirm_action_ || !confirm_action_.enabled) return;
  const uint8_t id = confirm_action_.id;
  if (!onConfirmRequested(id)) return;
  completion_kind_ = CompletionKind::kConfirm;
  destination_.exit();
}

BackResult FullScreenDialog::requestBack(BackSource source) {
  const DialogDismissReason reason = DismissReasonFor(source);
  if (onDismissRequested(reason)) {
    completion_kind_ = CompletionKind::kDismiss;
    dismiss_reason_ = reason;
    destination_.exit();
  }
  return BackResult::kHandled;
}

bool FullScreenDialog::isShowing() const {
  return destination_.getNavigationHost() != nullptr;
}

bool FullScreenDialog::isCurrent() const {
  auto* host = destination_.getNavigationHost();
  return host != nullptr && host->isCurrent(destination_);
}

void FullScreenDialog::prepareForDerivedDestruction() {
  destroying_ = true;
  if (isShowing()) {
    CHECK(isCurrent());
    destination_.exit();
  }
  DialogScaffold::prepareForDerivedDestruction();
}

void FullScreenDialog::DialogDestination::onStop() {
  if (!getNavigationHost()->isAvailable()) {
    owner_.completion_kind_ = CompletionKind::kDismiss;
    owner_.dismiss_reason_ = DialogDismissReason::kInteractionOwnerDetached;
  }
}

void FullScreenDialog::complete() {
  const CompletionKind kind = completion_kind_;
  const uint8_t id = confirm_action_.id;
  const DialogDismissReason reason = dismiss_reason_;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  if (destroying_) return;
  if (kind == CompletionKind::kConfirm) {
    onConfirmed(id);
  } else {
    onDismissed(reason);
  }
}

DialogDismissReason FullScreenDialog::DismissReasonFor(BackSource source) {
  return source == BackSource::kEscapeKey ? DialogDismissReason::kEscape
                                          : DialogDismissReason::kBack;
}

}  // namespace roo_windows::material3
