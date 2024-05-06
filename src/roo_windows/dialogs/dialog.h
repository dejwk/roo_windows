#pragma once

#include <functional>

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {

class Dialog : public VerticalLayout {
 public:
  typedef std::function<void(int action_idx)> CallbackFn;

  Rect maxBounds() const override { return Rect::MaximumRect(); }

  void setTitle(std::string title);

  // Intercept all touch events to ensure the dialog is modal.
  Widget* dispatchTouchDownEvent(XDim x, YDim y) override;

  // If the dialog is currently open, closes it and calls callback_fn(-1).
  void close();

  // Dialog appear at a quite high elevation over the underlying content.
  uint8_t getElevation() const override { return 20; }

  // Use some round corners.
  BorderStyle getBorderStyle() const override { return BorderStyle(5, 0); }

  Padding getPadding() const override { return Padding(4, 4); }

 protected:
  Dialog(const Environment& env, const std::vector<std::string> button_labels);

  void finalizePaintWidget(const Canvas& canvas, Clipper& clipper,
                           const OverlaySpec& overlay_spec) const override;

  VerticalLayout body_;
  TextLabel title_;
  ScrollablePanel contents_;

  virtual void onEnter() {}
  virtual void onExit(int result) {}

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
  std::vector<SimpleButton> buttons_;
  CallbackFn callback_fn_;
};

}  // namespace roo_windows
