#pragma once

#include <vector>

#include "roo_display.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows/widgets/scrim.h"

namespace roo_windows {

class Application;

/// Root container and shared UI services owner for an Application.
class MainWindow : public Container {
 public:
  MainWindow(Application& app, const roo_display::Box& bounds);

  ~MainWindow() override;

  /// Drives the shared click-animation forward one tick and invalidates
  /// regions that changed.
  void refreshClickAnimation();

  /// Applies any pending layout requests in the widget tree.
  void updateLayout();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  /// Performs a single paint pass onto the supplied display surface, bounded
  /// by `deadline`. Returns false when the deadline interrupted painting.
  bool paintWindow(const roo_display::Surface& s, roo_time::Uptime deadline);

  Application& app() const;
  const Theme& theme() const override;

  /// Adds a regular task layer child. Tasks render behind popups and
  /// dialogs.
  void addTask(WidgetRef child, const Rect& rect);

  /// Adds a popup layer child. Popups render above tasks but below dialogs.
  void addPopup(WidgetRef child, const Rect& rect);

  /// Returns the shared click-animation controller for this window.
  ClickAnimation& click_animation() { return click_animation_; }

  /// Returns the shared click-animation controller for this window.
  const ClickAnimation& click_animation() const { return click_animation_; }

  /// Returns the single slot for root interactive transient presentations.
  TransientPresentationSlot& transient_presentation_slot() {
    return transient_presentation_slot_;
  }

  /// Returns the single slot for root interactive transient presentations.
  const TransientPresentationSlot& transient_presentation_slot() const {
    return transient_presentation_slot_;
  }

  /// Shows a modal dialog when the interactive transient slot is available.
  PresentationStartResult showDialog(Dialog& dialog,
                                     Dialog::CallbackFn callback_fn);

  /// If a dialog is currently open, closes it (callback receives -1).
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

  void childInvalidatedRegion(const Widget* child, Rect rect) override;

 private:
  friend class Dialog;

  void addToLayer(std::vector<Widget*>& layer, WidgetRef child,
                  const Rect& rect);

  void removeLastFromLayer(std::vector<Widget*>& layer);

  void detachDialog(Dialog& dialog);

  Application& app_;

  // Regular application tasks rendered behind popup overlays such as the
  // keyboard.
  std::vector<Widget*> tasks_;

  // Popup tasks rendered above regular tasks but below modal dialogs.
  std::vector<Widget*> popups_;

  ClickAnimation click_animation_;

  // Stored as instance variable, to avoid vector reallocation on each paint.
  internal::ClipperState clipper_state_;

  // Maintains the area that encapsulates all content that needs to be
  // (re)drawn.
  Rect redraw_bounds_;

  bool initialized_ = false;

  Dialog* active_dialog_ = nullptr;

  bool pending_scrim_blit_ = false;

  Scrim scrim_;

  // Kept last so it clears presenter reachability before other window
  // resources are destroyed.
  TransientPresentationSlot transient_presentation_slot_;
};

}  // namespace roo_windows
