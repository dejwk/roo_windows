#pragma once

#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {
namespace material3 {

class NavigationBarDestinationTestAccess;
class NavigationRailDestinationTestAccess;

namespace internal {

/// The two content arrangements shared by Material navigation destinations.
enum class NavigationDestinationPresentation : uint8_t {
  kStacked,
  kInline,
};

/// Component-resolved slots consumed by the common destination paint pass.
struct NavigationDestinationGeometry {
  Rect icon_bounds;
  Rect label_bounds;
  Rect indicator_bounds;
};

/// Shared state, interaction, and paint behavior for Material navigation
/// destinations.
///
/// Concrete navigation components retain ownership of their geometry, tokens,
/// layout enums, and selection containers. They provide only the resolved
/// content slots and the component-specific activation callback.
class NavigationDestinationBase : public BasicWidget {
 public:
  roo::string_view label() const;
  void setLabel(roo::string_view label);

  const MonoIcon* icon() const;
  void setIcon(const MonoIcon* icon);

  const MonoIcon* selectedIcon() const;
  void setSelectedIcon(const MonoIcon* icon);

  bool selected() const;
  bool isClickable() const override;

  OverlayType getOverlayType() const override { return OVERLAY_CUSTOM; }

  ClickOverlayAnimation getClickOverlayAnimation() const override {
    return ClickOverlayAnimation::kFade;
  }

  bool useOverlayOnSelection() const override { return false; }

  ColorToken effectiveOverlayColorRole() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  NavigationDestinationBase(ApplicationContext& context, roo::string_view label,
                            const MonoIcon* icon,
                            const MonoIcon* selected_icon);

  const MonoIcon* displayedIcon() const;
  NavigationDestinationPresentation presentation() const;
  void setPresentation(NavigationDestinationPresentation presentation);
  void setSelectedFromOwner(bool selected);

  void onClicked() override;
  void notifyStateChanged(uint16_t state_diff) override;
  Rect getDirectPaintExclusionBounds() const override;

 private:
  virtual NavigationDestinationGeometry resolveDestinationGeometry() const = 0;
  virtual void activateFromOwner() = 0;

  friend class ::roo_windows::material3::NavigationBarDestinationTestAccess;
  friend class ::roo_windows::material3::NavigationRailDestinationTestAccess;

  roo::string_view label_;
  const MonoIcon* icon_;
  const MonoIcon* selected_icon_;
  uint8_t presentation_ : 1;
  uint8_t selected_ : 1;
};

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows
