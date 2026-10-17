#pragma once

#include <type_traits>

#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material2/theme.h"

namespace roo_windows::material2 {

/// Resolves locally owned Material 2 surfaces; unfilled widgets inherit colors.
/// Adds no per-widget state or dependency on another system's palette roles.
template <typename Base>
class ThemeMixin : public Base {
 public:
  using Base::Base;

  /// Returns the owned Material 2 surface role, or kNone to inherit colors.
  virtual ColorRole containerRole() const { return ColorRole::kNone; }

  /// Publishes colors only for a locally owned semantic surface.
  SurfaceColors surfaceColors() const override {
    if constexpr (std::is_base_of<SurfaceWidget, Base>::value) {
      const ColorRole role = containerRole();
      if (role != ColorRole::kNone) {
        const auto& colors = this->theme().material2Theme().color;
        return {colors.contentColorFor(role), colors.accentColorFor(role)};
      }
    }
    return Base::surfaceColors();
  }
};

}  // namespace roo_windows::material2
