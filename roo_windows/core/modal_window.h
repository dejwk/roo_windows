#pragma once

#include "roo_display.h"
#include "roo_display/shape/basic_shapes.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/containers/static_layout.h"

namespace roo_windows {

class ModalWindow : public StaticLayout {
 public:
  ModalWindow(const Environment& env) : StaticLayout(env) {}

  void enter() {
    setVisible(true);
    getMainWindow()->enterModal(this);
  }

  void exit() {
    setVisible(false);
    getMainWindow()->exitModal(this);
  }

  void paintWidgetContents(const roo_display::Surface& s,
                           Clipper& clipper) override {
    if (isVisible() && isInvalidated()) {
      Color color = theme().color.defaultColor(s.bgcolor());
      color.set_a(0x20);
      s.drawObject(roo_display::FilledRect(0, 0, width() - 1, 1, color));
      s.drawObject(roo_display::FilledRect(0, 2, 1, height() - 4, color));
      color.set_a(0x40);
      s.drawObject(roo_display::FilledRect(width() - 2, 2, width() - 1,
                                           height() - 4, color));
      s.drawObject(roo_display::FilledRect(0, height() - 3, width() - 1,
                                           height() - 1, color));
    }
    roo_display::Surface news(s);
    news.clipToExtents(Box(2, 2, width() - 3, height() - 4));
    Panel::paintWidgetContents(news, clipper);
  }
};

}  // namespace roo_windows
