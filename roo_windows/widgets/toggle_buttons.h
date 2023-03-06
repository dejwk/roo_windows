#pragma once

#include <memory>
#include <vector>

#include "roo_windows/core/panel.h"

namespace roo_windows {

class ToggleButtons : public Panel {
 public:
  ToggleButtons(const Environment& env, int16_t padding = Scaled(12))
      : Panel(env), env_(env), padding_(padding), active_(-1) {}

  roo_windows::Widget& addButton(const MonoIcon& icon);

  // Draws left- and right-side framing around the togglebutton control.
  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const override;
  Padding getPadding() const override { return Padding(0); }

  Dimensions getNaturalDimensions() const override {
    Dimensions d = getSuggestedMinimumDimensions();
    return Dimensions(d.width() + (2 * padding_ - 2) * buttons_.size(),
                      d.height() + (2 * padding_ - 2));
  }

  int getActive() const { return active_; }

  void setActive(int index);

 private:
  class ToggleButton : public roo_windows::Widget {
   public:
    ToggleButton(const Environment& env, const MonoIcon& icon,
                 ToggleButtons& group, int idx)
        : roo_windows::Widget(env), icon_(icon), group_(group), idx_(idx) {}

    bool useOverlayOnActivation() const override { return true; }
    bool isClickable() const override { return true; }

    void onClicked() override {
      group_.setActive(idx_);
      Widget::onClicked();
    }

    void paint(const Canvas& canvas) const override;

    Dimensions getSuggestedMinimumDimensions() const override {
      return Dimensions(icon_.anchorExtents().width() + 2,
                        icon_.anchorExtents().height() + 2);
    }

    Padding getPadding() const override { return Padding(11); }

    const MonoIcon& icon() const { return icon_; }

   private:
    const ToggleButtons& parentGroup() const { return group_; }

    const MonoIcon& icon_;
    ToggleButtons& group_;
    int idx_;
  };

  friend class ToggleButton;

  const Environment& env_;

  int16_t padding_;  // defaults to 12.
  int active_;

  // Using unique_ptr to make sure that the address of a button remains constant
  // as we add or remove buttons.
  std::vector<std::unique_ptr<ToggleButton>> buttons_;
};

}  // namespace roo_windows
