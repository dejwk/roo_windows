#include "roo_windows/material3/dialog/basic_dialog.h"

#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {

BasicDialog::BasicDialog(ApplicationContext& context, WidgetRef body,
                         const DialogActionSpec* actions, uint8_t action_count)
    : DialogScaffoldBase(context, std::move(body),
                         internal::DialogScaffoldVariant::kBasic),
      actions_(context, *this, actions, action_count) {
  attachDerivedChrome(internal::DialogChromeSlot::kPrimary, actions_);
}

BasicDialog::~BasicDialog() { prepareForDerivedDestruction(); }

void BasicDialog::setIcon(const MonoIcon* icon) {
  setDialogIcon(icon);
  dialogTitle().setTextAlign(
      icon == nullptr ? (layoutDirection() == LayoutDirection::kLeftToRight
                             ? TextAlign::kStart
                             : TextAlign::kEnd)
                      : TextAlign::kCenter);
}

void BasicDialog::clearIcon() { setIcon(nullptr); }

void BasicDialog::setLayoutDirection(LayoutDirection direction) {
  setDialogLayoutDirection(direction);
  actions_.setLayoutDirection(direction);
  dialogTitle().setTextAlign(hasDialogIcon()
                                 ? TextAlign::kCenter
                                 : (direction == LayoutDirection::kLeftToRight
                                        ? TextAlign::kStart
                                        : TextAlign::kEnd));
}

DialogShowResult BasicDialog::show(Task& interaction_owner) {
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  return showBasicDialogSurface(interaction_owner);
}

void BasicDialog::dismiss() {
  if (!isShowing()) return;
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DialogDismissReason::kProgrammatic;
  finishDialog(PresentationFinishReason::kCancel);
}

void BasicDialog::invokeDialogAction(uint8_t id, DialogActionRole role) {
  if (!isShowing()) return;
  completion_kind_ = CompletionKind::kAction;
  action_id_ = id;
  action_role_ = role;
  finishDialog(PresentationFinishReason::kAction);
}

Widget* BasicDialog::preferredChromeFocusChild() {
  return actions_.preferredFocusChild();
}

BackResult BasicDialog::onDialogBackRequested(BackSource source) {
  completion_kind_ = CompletionKind::kDismiss;
  dismiss_reason_ = DismissReasonFor(source);
  finishDialog(PresentationFinishReason::kBack);
  return BackResult::kHandled;
}

void BasicDialog::onDialogPresentationFinished(
    PresentationFinishReason reason) {
  const CompletionKind kind = completion_kind_;
  completion_kind_ = CompletionKind::kDismiss;
  if (kind == CompletionKind::kAction &&
      reason == PresentationFinishReason::kAction) {
    const uint8_t id = action_id_;
    const DialogActionRole role = action_role_;
    onActionInvoked(id, role);
    return;
  }
  const DialogDismissReason dismiss_reason =
      reason == PresentationFinishReason::kBack ? dismiss_reason_
                                                : DismissReasonFor(reason);
  onDismissed(dismiss_reason);
}

DialogDismissReason BasicDialog::DismissReasonFor(BackSource source) {
  return source == BackSource::kEscapeKey ? DialogDismissReason::kEscape
                                          : DialogDismissReason::kBack;
}

DialogDismissReason BasicDialog::DismissReasonFor(
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

namespace internal {

AlertDialogBodyStorage::AlertDialogBodyStorage(ApplicationContext& context,
                                               std::string supporting_text)
    : supporting_text_(context, std::move(supporting_text),
                       text_style_body_medium()) {
  supporting_text_.setWrapMode(TextWrapMode::kWordWrap);
}

}  // namespace internal

AlertDialog::AlertDialog(ApplicationContext& context, std::string headline,
                         std::string supporting_text,
                         const DialogActionSpec* actions, uint8_t action_count)
    : internal::AlertDialogBodyStorage(context, std::move(supporting_text)),
      BasicDialog(context, WidgetRef(supporting_text_), actions, action_count) {
  setHeadline(std::move(headline));
}

void AlertDialog::setSupportingText(std::string supporting_text) {
  supporting_text_.setText(std::move(supporting_text));
}

}  // namespace roo_windows::material3
