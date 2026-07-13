#pragma once

#include <functional>

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

/// Modal dialog base class.
///
/// Provides the standard scaffold: title, scrollable content area, optional
/// horizontal dividers, and a row of footer buttons whose labels are supplied
/// at construction. The dialog intercepts touch-down events to enforce modal
/// behavior, renders at elevated z to float above the underlying surface, and
/// invokes the registered `CallbackFn` with the chosen button index on
/// dismissal (or -1 for `close()`).
///
/// Subclasses populate `contents_` with their specific UI. Override `onEnter`
/// / `onExit` to react to lifecycle transitions.
class Dialog : public VerticalLayout {
 public:
  typedef std::function<void(int action_idx)> CallbackFn;

  /// Cancels an active presentation without delivering application completion.
  ~Dialog() override;

  /// Replaces the dialog's title text.
  void setTitle(std::string title);

  /// If the dialog is currently open, closes it and invokes the callback
  /// with -1 (no action chosen).
  void close();

  /// Dialog floats at a high elevation over the underlying content.
  uint8_t getElevation() const override { return 20; }

  /// Returns rounded corners with no outline.
  BorderStyle getBorderStyle() const override { return BorderStyle(5, 0); }

 protected:
  Dialog(ApplicationContext& context, const std::vector<std::string> button_labels);

  /// Returns the number of footer buttons.
  int button_count() const { return buttons_.size(); }
  /// Returns the footer button at `idx`.
  SimpleButton& button(int idx) { return buttons_[idx]; }
  const SimpleButton& button(int idx) const { return buttons_[idx]; }

  /// Returns the last (typically affirmative) footer button.
  SimpleButton& last_button() { return buttons_[button_count() - 1]; }

  const SimpleButton& last_button() const {
    return buttons_[button_count() - 1];
  }

  /// Shows or hides the horizontal dividers that separate the title,
  /// content, and button rows.
  void setDividersVisible(bool visible) {
    divider1_.setVisibility(visible ? Visibility::kVisible : Visibility::kGone);
    divider2_.setVisibility(visible ? Visibility::kVisible : Visibility::kGone);
  }

  /// Attaches caller-provided content for the current presentation.
  ///
  /// The content is detached before completion. Call again before re-showing
  /// a dialog whose content is not recreated by `onEnter()`.
  void setPresentationContent(WidgetRef content);

  /// Finishes the visible dialog with the selected footer action index.
  void actionTaken(int id);

  TextLabel title_;
  HorizontalDivider divider1_;
  ScrollablePanel contents_;
  HorizontalDivider divider2_;

  /// Lifecycle hook invoked when the dialog becomes visible.
  virtual void onEnter() {}
  /// Lifecycle hook invoked when the dialog is dismissed with `result`.
  virtual void onExit(int result) {}

 private:
  friend class MainWindow;

  class Registration final : public TransientPresentationRegistration {
   public:
    explicit Registration(Dialog& dialog) : dialog_(dialog) {}

    void cancelPresentation() { cancel(); }

   protected:
    void detachPresentation(PresentationFinishReason reason) override;
    void onFinished(PresentationFinishReason reason) override;

   private:
    Dialog& dialog_;
  };

  class FullWidthPanel : public HorizontalLayout {
   public:
    using HorizontalLayout::HorizontalLayout;

    PreferredSize getPreferredSize() const override {
      return PreferredSize(PreferredSize::MatchParentWidth(),
                           PreferredSize::WrapContentHeight());
    }
  };

  void beginPresentation(CallbackFn callback_fn);
  void detachPresentation(PresentationFinishReason reason);
  void notifyFinished(PresentationFinishReason reason);
  void clearPresentationContent();

  void setCallbackFn(CallbackFn callback_fn) {
    callback_fn_ = std::move(callback_fn);
  }

  Registration& registration() { return registration_; }

  FullWidthPanel title_panel_;
  FullWidthPanel button_panel_;
  std::vector<SimpleButton> buttons_;
  CallbackFn callback_fn_;
  int result_ = -1;
  Widget* presentation_content_ = nullptr;

  // Must remain last so it vacates the window slot before dialog members.
  Registration registration_;
};

}  // namespace roo_windows
