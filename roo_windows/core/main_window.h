#pragma once

#include "roo_display.h"
#include "roo_display/filter/background_fill_optimizer.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

class Application;

class MainWindow : public Panel {
 public:
  MainWindow(Application& app, const roo_display::Box& bounds);

  void tick();

  // Applies any pending layout requests.
  void updateLayout();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  void paintWindow(const roo_display::Surface& s);

  Application& app() const;
  const Theme& theme() const override;

  void add(WidgetRef child, const Rect& rect) {
    Panel::add(std::move(child), rect);
  }

  ClickAnimation& click_animation() { return click_animation_; }

  PressOverlay& press_overlay() { return press_overlay_; }

  void set_press_overlay(PressOverlay press_overlay) {
    press_overlay_ = std::move(press_overlay);
  }

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

  roo_display::BackgroundFillOptimizer::FrameBuffer background_fill_buffer_;
};

}  // namespace roo_windows
