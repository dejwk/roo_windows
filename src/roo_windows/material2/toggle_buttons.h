#pragma once

#include <memory>
#include <vector>

#include "roo_windows/core/panel.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material2/theme_mixin.h"

namespace roo_windows::material2 {

/// Mutually-exclusive group of icon-only push buttons (segmented control).
///
/// Add icons through `addButton()`; at most one is active at a time, exposed
/// via `getActive()` / `setActive()`. The group paints its own framing around
/// the buttons rather than relying on per-button decoration.
class ToggleButtons : public ThemeMixin<Panel> {
 public:
  ToggleButtons(ApplicationContext& context, int16_t padding = Scaled(12))
      : ThemeMixin<Panel>(context), padding_(padding), active_(-1) {}

  /// Detaches buttons before their owning member vector is destroyed.
  ~ToggleButtons() override { removeAll(); }

  /// Appends a new icon-only button at the end of the strip and returns the
  /// owning widget reference (for layout/styling tweaks).
  roo_windows::Widget& addButton(const MonoIcon& icon);

  /// Draws left- and right-side framing around the togglebutton control.
  void paint(PaintContext& ctx) const override;

  /// Reports the minimum size derived from the largest button glyph.
  Dimensions getSuggestedMinimumDimensions() const override;
  Padding getPadding() const override { return Padding(0); }

  /// Adds horizontal padding per button and vertical padding for the framing
  /// on top of `getSuggestedMinimumDimensions()`.
  Dimensions getNaturalDimensions() const override {
    Dimensions d = getSuggestedMinimumDimensions();
    return Dimensions(d.width() + (2 * padding_ - 2) * buttons_.size(),
                      d.height() + (2 * padding_ - 2));
  }

  int getActive() const { return active_; }

  /// Sets the active button by index (or -1 to clear). Returns true if the
  /// active button changed; false if `index` was already active.
  bool setActive(int index);

 private:
  void notifyButtonClicked(int index);

  class ToggleButton : public ThemeMixin<SurfaceWidget> {
   public:
    ToggleButton(ApplicationContext& context, const MonoIcon& icon,
                 ToggleButtons& group, int idx)
        : ThemeMixin<SurfaceWidget>(context),
          icon_(icon),
          group_(group),
          idx_(idx) {}

    bool fullyCoversBoundsWithOpaqueColors() const override { return false; }
    Color componentOverlayColor() const override {
      return isActivated() ? theme().material2Theme().color.onSurface.withA(31)
                           : roo_display::color::Transparent;
    }
    bool isClickable() const override { return true; }

    ClickActivationPolicy getClickActivationPolicy() const override {
      return ClickActivationPolicy::kImmediateContinueAnimation;
    }

    /// Notifies the parent group that this button became active, then runs
    /// the base click hook.
    void onClicked() override {
      group_.notifyButtonClicked(idx_);
      Widget::onClicked();
    }

    /// Paints the icon beneath its component-owned persistent tint.
    void paint(PaintContext& ctx) const override;

    /// Reports the icon's anchor extents plus a 1 dp margin.
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

  int16_t padding_;  // defaults to 12.
  int active_;

  // Using unique_ptr to make sure that the address of a button remains constant
  // as we add or remove buttons.
  std::vector<std::unique_ptr<ToggleButton>> buttons_;
};

}  // namespace roo_windows::material2
