#pragma once

#include <memory>
#include <vector>

#include "press_overlay.h"
#include "roo_display.h"
#include "roo_display/core/color.h"
#include "roo_display/filter/foreground.h"
#include "roo_windows/theme.h"
#include "roo_windows/widget.h"

namespace roo_windows {

using roo_display::Color;

static const int16_t kTouchMargin = 8;

class Panel : public Widget {
 public:
  Panel(Panel* parent, const Box& bounds);

  Panel(Panel* parent, const Box& bounds, roo_display::Color bgcolor);

  Panel(Panel* parent, const Box& bounds, const Theme& theme,
        roo_display::Color bgcolor);

  void setBackground(Color bgcolor) { bgcolor_ = bgcolor; }
  Color background() const override { return bgcolor_; }

  const Theme& theme() const { return theme_; }

  void paint(const Surface& s) override;

  virtual bool onTouch(const TouchEvent& event);

 protected:
  const std::vector<std::unique_ptr<Widget>>& children() const {
    return children_;
  }

  void invalidateDescending() override;
  void invalidateDescending(const Box& box) override;

 private:
  friend class Widget;

  void addChild(Widget* child);

  std::vector<std::unique_ptr<Widget>> children_;
  const Theme& theme_;
  Color bgcolor_;
  Box invalid_region_;
};

}  // namespace roo_windows