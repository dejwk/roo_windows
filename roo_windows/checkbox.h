#pragma once

#include "roo_windows/widget.h"

namespace roo_windows {

class Checkbox : public Widget {
 public:
  Checkbox(Panel* parent, const Box& bounds, bool on)
      : Widget(parent, bounds), on_(on) {}

  void defaultPaint(const Surface& s) override;

  bool isClickable() const override { return true; }

  void onClicked() override {
    on_ = !on_;
    markDirty();
    Widget::onClicked();
  }

  bool isOn() const { return on_; }

 private:
  bool on_;
};

}  // namespace roo_windows