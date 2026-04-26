#pragma once

#include <vector>

#include "roo_display.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/widgets/scrim.h"

namespace roo_windows {

class Application;

class MainWindow : public Container {
 public:
  MainWindow(Application& app, const roo_display::Box& bounds);

  ~MainWindow() override;

  void refreshClickAnimation();

  // Applies any pending layout requests.
  void updateLayout();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  bool paintWindow(const roo_display::Surface& s, roo_time::Uptime deadline);

  Application& app() const;
  const Theme& theme() const override;

  // Adds a regular task layer child. Tasks render behind popups and dialogs.
  void addTask(WidgetRef child, const Rect& rect);

  // Adds a popup layer child. Popups render above tasks but below dialogs.
  void addPopup(WidgetRef child, const Rect& rect);

  ClickAnimation& click_animation() { return click_animation_; }

  const ClickAnimation& click_animation() const { return click_animation_; }

  PressOverlay& press_overlay() { return press_overlay_; }

  void set_press_overlay(PressOverlay press_overlay) {
    press_overlay_ = std::move(press_overlay);
  }

  void showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn);

  void clearDialog();

 protected:
  int getChildrenCount() const override {
    return static_cast<int>(tasks_.size()) + static_cast<int>(popups_.size()) +
           (active_dialog_ != nullptr ? 2 : 0);
  }

  const Widget& getChild(int idx) const override {
    int task_count = static_cast<int>(tasks_.size());
    if (idx < task_count) return *tasks_[idx];
    idx -= task_count;
    int popup_count = static_cast<int>(popups_.size());
    if (idx < popup_count) return *popups_[idx];
    return idx == popup_count ? static_cast<const Widget&>(scrim_)
                              : static_cast<const Widget&>(*active_dialog_);
  }

  Widget& getChild(int idx) override {
    int task_count = static_cast<int>(tasks_.size());
    if (idx < task_count) return *tasks_[idx];
    idx -= task_count;
    int popup_count = static_cast<int>(popups_.size());
    if (idx < popup_count) return *popups_[idx];
    return idx == popup_count ? static_cast<Widget&>(scrim_)
                              : static_cast<Widget&>(*active_dialog_);
  }

  void propagateDirty(const Widget* child, const Rect& rect) override;

 private:
  void addToLayer(std::vector<Widget*>& layer, WidgetRef child,
                  const Rect& rect);

  void removeLastFromLayer(std::vector<Widget*>& layer);

  Application& app_;

  // Regular application tasks rendered behind popup overlays such as the
  // keyboard.
  std::vector<Widget*> tasks_;

  // Popup tasks rendered above regular tasks but below modal dialogs.
  std::vector<Widget*> popups_;

  ClickAnimation click_animation_;

  // Singleton used to draw circular 'click' overlay.
  PressOverlay press_overlay_;

  // Stored as instance variable, to avoid vector reallocation on each paint.
  internal::ClipperState clipper_state_;

  // Maintains the area that encapsulates all content that needs to be
  // (re)drawn.
  Rect redraw_bounds_;

  bool initialized_ = false;

  Dialog* active_dialog_ = nullptr;

  bool pending_scrim_blit_ = false;

  Scrim scrim_;
};

}  // namespace roo_windows
