#pragma once

#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {

/// Resolves an owned Material 3 surface without propagating semantic roles.
/// Adds no instance state. Generic descendants inherit only resolved colors.
template <typename Base>
class Material3Surface : public Base {
 public:
  using Base::Base;

  /// Returns the local palette role; kNone forwards the surrounding colors.
  virtual ColorToken containerRole() const { return ColorToken::kNone; }

  /// Publishes readable foreground and emphasis for the owned surface.
  SurfaceColors surfaceColors() const override {
    const ColorToken role = containerRole();
    if (role == ColorToken::kNone) return Base::surfaceColors();
    const ColorScheme& colors = this->theme().material3Theme().color;
    return {colors.contentColorFor(role), colors.accentColorFor(role)};
  }
};

/// Material 3 surface leaf with no additional instance state.
using Material3SurfaceWidget = Material3Surface<SurfaceWidget>;

}  // namespace roo_windows::material3
