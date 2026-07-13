#include "dialog.h"

#include "roo_windows/core/main_window.h"
#include "roo_windows/widgets/button.h"

namespace roo_windows {

Dialog::Dialog(ApplicationContext& context, std::vector<std::string> button_labels)
    : VerticalLayout(context),
      title_(context, "", font_h6()),
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
  if (!registration_.isActive()) return;
  MainWindow* window = getMainWindow();
  if (window != nullptr) window->detachDialog(*this);
  registration_.cancelPresentation();
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

void Dialog::beginPresentation(CallbackFn callback_fn) {
  result_ = -1;
  setCallbackFn(std::move(callback_fn));
  onEnter();
}

void Dialog::detachPresentation(PresentationFinishReason) {
  clearPresentationContent();
  MainWindow* window = getMainWindow();
  if (window != nullptr) window->detachDialog(*this);
}

void Dialog::notifyFinished(PresentationFinishReason) {
  Dialog::CallbackFn callback_fn = std::move(callback_fn_);
  callback_fn_ = nullptr;
  onExit(result_);
  if (callback_fn != nullptr) callback_fn(result_);
}

void Dialog::Registration::detachPresentation(PresentationFinishReason reason) {
  dialog_.detachPresentation(reason);
}

void Dialog::Registration::onFinished(PresentationFinishReason reason) {
  dialog_.notifyFinished(reason);
}

void Dialog::clearPresentationContent() {
  if (presentation_content_ == nullptr) return;
  // ScrollablePanel's inherited clearContents() bypasses its blit-cache
  // wrapper. Dispatch through the derived content setter to detach the child.
  contents_.setContents(WidgetRef());
  presentation_content_ = nullptr;
}

}  // namespace roo_windows
