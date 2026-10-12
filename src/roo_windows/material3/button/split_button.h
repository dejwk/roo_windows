#pragma once

#include "roo_windows/core/container.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/menu/menu.h"

namespace roo_windows::material3 {

/// Material 3 emphasis shared by the primary and menu segments.
enum class SplitButtonVariant : uint8_t {
  kFilled,
  kFilledTonal,
  kOutlined,
  kElevated,
};

/// Presents a primary action beside a related borrowed menu.
/// The menu, label bytes, and optional icon must outlive this widget. Both
/// segments are ordinary button focus targets and share the enabled state.
class SplitButton : public Container, private MenuPresentationObserver {
 public:
  /// Creates a Small, left-to-right split button borrowing @p menu and @p
  /// label. The menu must use the same @p context and outlive this control.
  SplitButton(ApplicationContext& context, Menu& menu,
              roo::string_view label = {},
              SplitButtonVariant variant = SplitButtonVariant::kFilled);

  /// Cancels an owned presentation before releasing inline child buttons.
  ~SplitButton() override;

  /// Returns the current emphasis shared by both segments.
  SplitButtonVariant variant() const;

  /// Changes both segment appearances.
  void setVariant(SplitButtonVariant variant);

  /// Returns the expressive size used by both segments.
  ButtonSize size() const { return primary_.size(); }

  /// Changes size and closes an owned menu before layout changes.
  void setSize(ButtonSize size);

  /// Returns the borrowed primary label.
  roo::string_view label() const { return primary_.label(); }

  /// Replaces borrowed primary text, closing an owned menu first.
  void setLabel(roo::string_view label);

  /// Returns the borrowed primary icon, or nullptr.
  const MonoIcon* icon() const { return primary_.icon(); }

  /// Replaces borrowed primary artwork, closing an owned menu first.
  void setIcon(const MonoIcon* icon);

  /// Returns the local physical ordering of the segments.
  LayoutDirection layoutDirection() const { return direction_; }

  /// Mirrors segment geometry without changing menu placement policy.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns the related menu borrowed for the widget lifetime.
  Menu& menu() const { return *menu_; }

  /// Opens the menu from the trailing segment and reports admission status.
  MenuShowResult showMenu();

  /// Synchronously cancels this control's active presentation.
  void dismissMenu();

  /// Reports ownership of the currently open root presentation.
  bool isMenuOpen() const { return trailing_.isActivated(); }

  /// Keeps the grouping surface transparent between its segments.
  Color background() const override { return roo_display::color::Transparent; }

  /// Child palettes already resolve disabled colors without a second tint.
  bool useAutomaticDisabledStyle() const override { return false; }

  /// Returns zero because split padding belongs to the child geometry.
  Padding getPadding() const override { return Padding(0); }

  /// Matches ordinary button spacing outside the composed control.
  Margins getMargins() const override { return Margins(MarginSize::kRegular); }

  /// Keeps the transparent grouping surface out of opaque-cover analysis.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

  /// Returns zero decoration contribution from the grouping surface.
  uint8_t getElevation() const override { return 0; }

  /// Lets each segment's shadow extend beyond the transparent grouping bounds.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Relays enabled-state changes to both child controls.
  void notifyStateChanged(uint16_t state_diff) override;

  /// Closes an owned menu if this control becomes hidden or detached.
  void onPresentationChanged(const PresentationChange& change) override;

  /// Invalidates child geometry after theme or density refresh.
  void requestLayoutDescending() override;

 protected:
  /// Delivers the default action through this container's callback registry.
  virtual void onPrimaryInvoked();

  /// Resolves natural split dimensions under parent constraints.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Places exact child rectangles and closes an open menu on bounds changes.
  void onLayout(bool changed, const Rect& rect) override;

  /// Returns no direct surface exclusion so the gap retains its backdrop.
  Rect getDirectPaintExclusionBounds() const override {
    return Rect(0, 0, -1, -1);
  }

  int getChildrenCount() const override { return 2; }
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  class SegmentButton final : public Button {
   public:
    using Button::Button;

    Margins getMargins() const override { return Margins(0); }
    Padding getPadding() const override { return Padding(0); }
    Dimensions getSuggestedMinimumDimensions() const override;
    bool isClickable() const override;
    Rect getSloppyTouchParentBounds() const override { return parent_bounds(); }
    Rect getParentDecorationBounds() const override;
    void onClicked() override;
    ClickActivationPolicy getClickActivationPolicy() const override;

   protected:
    BorderStyle resolveBorderStyle(bool pressed, float progress) const override;
    Rect resolveContentBounds(const Rect& bounds) const override;
    void notifyStateChanged(uint16_t state_diff) override;
    void emitPersistentDecoration(PaintContext& ctx) const override;

   private:
    BorderStyle resolveShape(bool pressed, bool hover, bool open,
                             float progress) const;
    SplitButton* owner() { return static_cast<SplitButton*>(parent()); }
    const SplitButton* owner() const {
      return static_cast<const SplitButton*>(parent());
    }
    bool isTrailing() const;
  };

  void onMenuClosed(Menu& menu, PresentationFinishReason reason) override;
  void syncChevron();

  SegmentButton primary_;
  SegmentButton trailing_;
  Menu* menu_;
  LayoutDirection direction_ = LayoutDirection::kLeftToRight;
};

}  // namespace roo_windows::material3
