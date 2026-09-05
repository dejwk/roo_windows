#include "dialog.h"

#include "roo_windows/core/display_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/button.h"

namespace roo_windows {

Dialog::Dialog(ApplicationContext& context,
               std::vector<std::string> button_labels)
    : VerticalLayout(context),
      title_(context, "", material2::text_style_h6()),
      divider1_(context),
      contents_(context),
      divider2_(context),
      title_panel_(context),
      button_panel_(context),
      callback_fn_(nullptr),
      registration_(*this) {
  title_panel_.setMargins(Margins(MarginSize::kNone, MarginSize::kRegular));
  add(title_panel_);
  title_.setPadding(PaddingSize::kLarge, PaddingSize::kSmall);
  title_.setMargins(MarginSize::kNone, MarginSize::kNone);
  title_panel_.add(title_);
  setDividersVisible(false);
  add(divider1_);
  contents_.setVerticalScrollBarPresence(
      VerticalScrollBar::Presence::kShownWhenScrolling);
  add(contents_, {weight : 1});
  add(divider2_);
  button_panel_.setPadding(PaddingSize::kTiny);
  button_panel_.setMargins(Margins(MarginSize::kNone, MarginSize::kSmall));
  add(button_panel_, {gravity : kGravityRight});
  button_panel_.setGravity(kGravityRight | kGravityMiddle);
  buttons_.reserve(button_labels.size());
  int i = 0;
  for (std::string& label : button_labels) {
    buttons_.emplace_back(context, std::move(label), Button::TEXT);
    buttons_.back().setOnInteractiveChange([this, i]() { actionTaken(i); });
    button_panel_.add(buttons_.back());
    ++i;
  }
}

Dialog::~Dialog() {
  prepareForDerivedDestruction();

  // Panel stores borrowed pointers to these member widgets. Detach them while
  // the pointees are still alive; C++ destroys members before base classes and
  // destroys buttons_ before button_panel_.
  button_panel_.clearChildrenForDestruction();
  title_panel_.clearChildrenForDestruction();
  removeAll();
}

PresentationStartResult Dialog::show(Task& interaction_owner,
                                     CallbackFn callback_fn) {
  static constexpr TransientSurfaceSpec kDialogSpec{
      TransientBarrierPaint::kScrim, TransientAdmissionPolicy::kRejectIfBusy,
      OutsideInteractionPolicy::kAbsorb,
      TransientPresentationPolicy(true, true), false};
  Preparation preparation(*this, interaction_owner, std::move(callback_fn));
  PresentationStartResult result =
      internal::GetTransientSurfaceHost(interaction_owner)
          .showPrepared(registration_, interaction_owner, *this, focus_scope_,
                        kDialogSpec, preparation);
  if (result == PresentationStartResult::kStarted) onShow();
  return result;
}

void Dialog::setTitle(std::string title) { title_.setText(std::move(title)); }

void Dialog::setPresentationContent(WidgetRef content) {
  clearPresentationContent();
  presentation_content_ = content.get();
  if (presentation_content_ != nullptr) {
    contents_.setContents(std::move(content));
  }
}

void Dialog::actionTaken(int idx) {
  result_ = idx;
  registration_.finish(PresentationFinishReason::kAction);
}

void Dialog::close() {
  result_ = -1;
  registration_.finish(PresentationFinishReason::kCancel);
}

bool Dialog::beginPresentation(CallbackFn callback_fn) {
  CHECK(!session_entered_);
  result_ = -1;
  setCallbackFn(std::move(callback_fn));
  session_entered_ = true;
  return onEnter();
}

void Dialog::detachPresentation(PresentationFinishReason) {
  endPresentationSession();
}

void Dialog::notifyFinished(PresentationFinishReason) {
  Dialog::CallbackFn callback_fn = std::move(callback_fn_);
  callback_fn_ = nullptr;
  onDismiss(result_);
  if (callback_fn != nullptr) callback_fn(result_);
}

bool Dialog::Preparation::createAndResolveBounds(Rect& root_bounds_in_window) {
  if (!dialog_.beginPresentation(std::move(callback_fn_))) return false;
  MainWindow& window = owner_.window().root();
  Dimensions dims = dialog_.measure(WidthSpec::AtMost(window.width()),
                                    HeightSpec::AtMost(window.height()));
  XDim left = (window.width() - dims.width()) / 2;
  YDim top = (window.height() - dims.height()) / 2;
  root_bounds_in_window =
      Rect(left, top, left + dims.width() - 1, top + dims.height() - 1);
  return true;
}

void Dialog::Preparation::deleteAfterFailedAdmission() {
  dialog_.rollbackPreparedPresentation();
}

void Dialog::Registration::detachPresentation(PresentationFinishReason reason) {
  dialog_.detachPresentation(reason);
}

void Dialog::Registration::onFinished(PresentationFinishReason reason) {
  dialog_.notifyFinished(reason);
}

void Dialog::clearPresentationContent() {
  if (presentation_content_ == nullptr) return;
  focus_scope_.clearRememberedFocus();
  // ScrollablePanel's inherited clearContents() bypasses its blit-cache
  // wrapper. Dispatch through the derived content setter to detach the child.
  contents_.setContents(WidgetRef());
  presentation_content_ = nullptr;
}

void Dialog::endPresentationSession() {
  if (!session_entered_) return;
  clearPresentationContent();
  session_entered_ = false;
  onExit();
}

void Dialog::rollbackPreparedPresentation() {
  endPresentationSession();
  callback_fn_ = nullptr;
}

void Dialog::prepareForDerivedDestruction() {
  if (registration_.isActive()) {
    registration_.disablePresentationInput();
    endPresentationSession();
    registration_.cancelPresentation();
  } else {
    rollbackPreparedPresentation();
  }
}

}  // namespace roo_windows
