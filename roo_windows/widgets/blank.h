#pragma once

#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Blank : public BasicWidget {
 public:
  Blank(const Environment& env, Dimensions dims)
      : BasicWidget(env), dims_(dims) {}

  void paint(const Canvas& canvas) const override {
    canvas.clear();
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return dims_;
  }

 private:
  const Dimensions dims_;
};

}  // namespace roo_windows
