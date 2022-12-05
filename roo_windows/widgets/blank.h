#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Blank : public BasicWidget {
 public:
  Blank(const Environment& env, Dimensions dims)
      : BasicWidget(env), dims_(dims) {}

  bool paint(const Canvas& canvas) override {
    canvas.clear();
    return true;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return dims_;
  }

 private:
  const Dimensions dims_;
};

}  // namespace roo_windows
