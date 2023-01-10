#pragma once

#include <functional>

#include "roo_glog/logging.h"

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/shadows/shadow.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

class Dialog : public VerticalLayout {
 public:
  typedef std::function<void(int action_idx)> CallbackFn;

  Rect maxBounds() const override { return Rect::MaximumRect(); }

  void setTitle(const std::string& title);

  // Intercept all touch events to ensure the dialog is modal.
  Widget* dispatchTouchDownEvent(XDim x, YDim y) override;

  // If the dialog is currently open, closes it and calls callback_fn(-1).
  void close();

 protected:
  Dialog(const Environment& env,
         const std::initializer_list<std::string>& button_labels);

  void paintWidgetContents(const Canvas& s, Clipper& clipper) override;

  VerticalLayout body_;
  TextLabel title_;
  ScrollablePanel contents_;

 private:
  friend class Task;

  // Called by a task.
  void setCallbackFn(CallbackFn callback_fn) {
    callback_fn_ = std::move(callback_fn);
  }

  // Called when the user explicitly chooses one of the options of the dialog.
  // Invokes the callback_fn, passing the ID.
  void actionTaken(int id);

  HorizontalLayout button_panel_;
  std::vector<Button> buttons_;
  CallbackFn callback_fn_;

  // For now, shadows are a bit ugly and slow, so the goal of adding support for
  // elevation to any component is postponed. We keep it specific to dialogs.
  Shadow shadow_;
  int depth_;
};

}  // namespace roo_windows
