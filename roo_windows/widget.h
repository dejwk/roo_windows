#pragma once

#include <vector>

#include "roo_display.h"
#include "roo_display/core/box.h"

namespace roo_windows {

class Panel;

using roo_display::Box;

class Widget {
 public:
  Widget(Panel* parent, Box bounds);

  virtual void drawContent(const roo_display::Surface& s) const {}

  void markDirty();

  const Box& bounds() const { return bounds_; }

  virtual void update(const roo_display::Surface& s) {
    if (!visible_ || !dirty_) return;
    roo_display::Surface cs = s;
    if (cs.clipToExtents(bounds()) == roo_display::Box::CLIP_RESULT_EMPTY) {
      return;
    }
    drawContent(cs);
    dirty_ = false;
  }

  bool isVisible() const { return visible_; }

 protected:
  bool dirty_;

 private:
  Panel* parent_;
  std::vector<std::unique_ptr<Widget>> children_;
  Box bounds_;
  bool visible_;
};

}  // namespace roo_windows