#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Blank : public Widget {
 public:
  Blank(const Environment& env, Dimensions dims)
      : Widget(env), dims_(dims) {}

  bool paint(const Surface& s) override {
    s.drawObject(roo_display::Clear());
    return true;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return dims_;
  }

 private:
  const Dimensions dims_;
};

}  // namespace roo_windows
