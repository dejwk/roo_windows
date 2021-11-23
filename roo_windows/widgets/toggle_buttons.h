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
  Padding getDefaultPadding() const override { return Padding(0); }

  Dimensions getNaturalDimensions() const override {
    Dimensions d = getSuggestedMinimumDimensions();
    return Dimensions(d.width() + 22 * buttons_.size(), d.height() + 22);
  }

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
      return Dimensions(icon_.extents().width() + 2,
                        icon_.extents().height() + 2);
    }

    Padding getDefaultPadding() const override { return Padding(11); }

    const MonoIcon& icon() const { return icon_; }

   private:
    const ToggleButtons* parentGroup() const {
      return (const ToggleButtons*)parent();
    }

    const MonoIcon& icon_;
  };

  friend class ToggleButton;

  const Environment& env_;

  int width_dp_;  // defaults to 72.
  int destination_size_dp_;
  //   roo_display::VAlign alignment_;
  int active_;

  std::vector<std::unique_ptr<ToggleButton>> buttons_;
};

}  // namespace roo_windows
