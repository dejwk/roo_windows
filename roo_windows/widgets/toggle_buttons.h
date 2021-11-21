#pragma once

#include <memory>
#include <vector>

#include "roo_windows/core/panel.h"

namespace roo_windows {

class ToggleButtons : public Panel {
 public:
  ToggleButtons(const Environment& env) : Panel(env), env_(env), active_(-1) {}

  roo_windows::Widget* addButton(const MonoIcon& icon);

  // Draws left- and right-side framing around the togglebutton control.
  bool paint(const roo_display::Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;

  int getActive() const { return active_; }

  void setActive(int index);

 private:
  class ToggleButton : public roo_windows::Widget {
   public:
    ToggleButton(const Environment& env, const MonoIcon& icon)
        : roo_windows::Widget(env), icon_(icon) {}

    bool useOverlayOnActivation() const override { return true; }
    bool isClickable() const override { return true; }

    bool paint(const roo_display::Surface& s) override;

    Dimensions getSuggestedMinimumDimensions() const override {
      return Dimensions(icon_.extents().width(), icon_.extents().height());
    }

    const MonoIcon& icon() const { return icon_; }

   private:
    const ToggleButtons* parentGroup() const {
      return (const ToggleButtons*)parent();
    }

    const MonoIcon& icon_;
  };

  friend class ToggleButton;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  //   roo_display::VAlign alignment_;
  int active_;

  const Environment& env_;
  std::vector<std::unique_ptr<ToggleButton>> buttons_;
};

}  // namespace roo_windows
