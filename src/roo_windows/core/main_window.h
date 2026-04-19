#pragma once

#include "roo_display.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/widgets/scrim.h"

namespace roo_windows {

class Application;

class MainWindow : public Panel {
 public:
  MainWindow(Application& app, const roo_display::Box& bounds);

  void refreshClickAnimation();

  // Applies any pending layout requests.
  void updateLayout();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  bool paintWindow(const roo_display::Surface& s, roo_time::Uptime deadline);

  Application& app() const;
  const Theme& theme() const override;

  void add(WidgetRef child, const Rect& rect) {
    Panel::add(std::move(child), rect);
  }

  ClickAnimation& click_animation() { return click_animation_; }

  const ClickAnimation& click_animation() const { return click_animation_; }

  PressOverlay& press_overlay() { return press_overlay_; }

  void set_press_overlay(PressOverlay press_overlay) {
    press_overlay_ = std::move(press_overlay);
  }

  // Special handling for the keyboard. It gets added as the first child, yet
  // The keyboard is always the first Panel child (index 0), but we want it
  // rendered on top of regular children yet behind an active dialog and its
  // scrim (which are the last two Panel children when present).
  const Widget& getChild(int idx) const override {
    int n = Panel::getChildrenCount();
    if (active_dialog_ != nullptr) {
      return idx >= n - 2   ? Panel::getChild(idx)
             : idx == n - 3 ? Panel::getChild(0)
                            : Panel::getChild(idx + 1);
    } else {
      return idx == n - 1 ? Panel::getChild(0)
                          : Panel::getChild(idx + 1);
    }
  }

  Widget& getChild(int idx) override {
    int n = Panel::getChildrenCount();
    if (active_dialog_ != nullptr) {
      return idx >= n - 2   ? Panel::getChild(idx)
             : idx == n - 3 ? Panel::getChild(0)
                            : Panel::getChild(idx + 1);
    } else {
      return idx == n - 1 ? Panel::getChild(0)
                          : Panel::getChild(idx + 1);
    }
  }

  void showDialog(Dialog& dialog, Dialog::CallbackFn callback_fn);

  void clearDialog();

 protected:
  void propagateDirty(const Widget* child, const Rect& rect) override;

 private:
  Application& app_;

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
