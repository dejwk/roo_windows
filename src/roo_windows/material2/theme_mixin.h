#pragma once

#include <type_traits>

#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material2/theme.h"

namespace roo_windows::material2 {

/// Bridges the current core container vocabulary to Material 2 surface roles.
/// This translates role identity only; it never reads a Material 3 palette.
ColorRole Material2ColorRole(material3::ColorToken role);

/// Selects Material 2 content and interaction defaults without per-widget
/// state. Generic child layouts inside a Material 2 composite can use the same
/// mixin.
template <typename Base>
class ThemeMixin : public Base {
 public:
  using Base::Base;

  /// Publishes Material 2 colors only when this widget owns a surface role.
  /// Unfilled controls and generic layouts forward the surrounding pair.
  SurfaceColors surfaceColors() const override {
    if constexpr (std::is_base_of<SurfaceWidget, Base>::value) {
      const material3::ColorToken role = this->containerRole();
      if (role != material3::ColorToken::kNone) {
        const auto& colors = this->theme().material2Theme().color;
        const ColorRole local_role = Material2ColorRole(role);
        return {colors.contentColorFor(local_role),
                colors.accentColorFor(local_role)};
      }
    }
    return Base::surfaceColors();
  }

  /// Resolves the live Material 2 ink for the shared interaction pipeline.
  roo_display::Color interactionColor() const override {
    const auto& color = this->theme().material2Theme().color;
    const ColorRole role =
        Material2ColorRole(this->effectiveOverlayColorRole());
    return this->usesHighlighterColor() ? color.accentColorFor(role)
                                        : color.contentColorFor(role);
  }
};

}  // namespace roo_windows::material2
