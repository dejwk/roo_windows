#pragma once

#include "roo_display.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class ModalWindow;
class Application;

class MainWindow : public Panel {
 public:
  MainWindow(Application& app, const Box& bounds);

  void tick();

  // Applies any pending layout requests.
  void updateLayout();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  void paintWindow(const Surface& s);

  Application& app();
  const Application& app() const;
  const Theme& theme() const override;

  void add(WidgetRef child, const roo_display::Box& box) {
    Panel::add(std::move(child), box);
  }

  ClickAnimation& click_animation() { return click_animation_; }

 protected:
  void propagateDirty(const Widget* child, const Box& box) override;

 private:
  Application& app_;

  ClickAnimation click_animation_;

  // Stored as instance variable, to avoid vector reallocation on each paint.
  internal::ClipperState clipper_state_;

  // Maintains the area that encapsulates all content that needs to be
  // (re)drawn.
  Box redraw_bounds_;

  ModalWindow* modal_window_;

  roo_display::BackgroundFillOptimizer::FrameBuffer background_fill_buffer_;
};

}  // namespace roo_windows
