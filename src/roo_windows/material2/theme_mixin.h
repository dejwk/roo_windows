#pragma once

#include "roo_windows/core/widget.h"
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

  /// Resolves default foreground content from the live Material 2 palette.
  roo_display::Color defaultColor() const override {
    return this->theme().material2Theme().color.contentColorFor(
        Material2ColorRole(this->effectiveContainerRole()));
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
