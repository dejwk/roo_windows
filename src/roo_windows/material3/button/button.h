#pragma once

#include "roo_backport/string_view.h"
#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {
namespace material3 {

// Material 3 standard button family.
//
// Phase 1 scope (see docs/material3_buttons_design.md):
//   * five variants: text, filled, filled tonal, outlined, elevated,
//   * single-line label,
//   * optional leading icon,
//   * geometry, colors, typography, and elevation resolved from the active
//     theme; no per-instance overrides.
//
// Per design, label is held as a non-owning `roo::string_view`. The caller
// must keep the backing storage alive while the label is set.
//
// The shared-appearance override surface described in the design doc is
// intentionally deferred to a later phase and is not implemented here.
enum class ButtonVariant : uint8_t {
  kText,
  kFilled,
  kFilledTonal,
  kOutlined,
  kElevated,
};

class Button : public BasicSurfaceWidget {
 public:
  /// Creates a Material 3 button with the supplied label and visual variant.
  ///
  /// The label is stored as a non-owning view; the caller must keep the
  /// backing storage alive for as long as the button uses it.
  explicit Button(const Environment& env, roo::string_view label = {},
                  ButtonVariant variant = ButtonVariant::kFilled);

  /// Returns the active Material 3 button variant.
  ButtonVariant variant() const { return variant_; }

  /// Switches the button to a different Material 3 visual variant.
  ///
  /// This updates theme-derived decoration such as container color, outline,
  /// and resting elevation, then requests relayout and repaint as needed.
  void setVariant(ButtonVariant variant);

  /// Returns the current non-owning label view.
  roo::string_view label() const { return label_; }

  /// Replaces the displayed label.
  ///
  /// The supplied text is stored as a non-owning view and must remain valid
  /// while the button is using it.
  void setLabel(roo::string_view label);

  /// Returns whether a leading icon is currently configured.
  bool hasIcon() const { return icon_ != nullptr; }

  /// Returns the configured leading icon, or nullptr when unset.
  const MonoIcon* icon() const { return icon_; }

  /// Sets or clears the optional leading icon.
  ///
  /// Passing nullptr removes the icon. The button does not take ownership of
  /// the icon object.
  void setIcon(const MonoIcon* icon);

  // BasicSurfaceWidget overrides.
  /// Returns the default content padding for the current icon/text layout.
  Padding getDefaultPadding() const override;

  /// Returns true because Material 3 buttons always participate in click
  /// handling.
  bool isClickable() const override { return true; }

  /// Returns the theme color role used for inherited surface decoration.
  ColorRole containerRole() const override;

  /// Returns the resolved container fill color for the current state.
  Color background() const override;

  /// Returns the resolved outline color, used by the outlined variant.
  Color getOutlineColor() const override;

  /// Returns the border radius and outline width for the current variant.
  BorderStyle getBorderStyle() const override;

  /// Returns the resting elevation advertised to the surface framework.
  uint8_t getElevation() const override;

  // Widget overrides.
  /// Paints the button's label and optional leading icon.
  void paint(const Canvas& canvas) const override;

  /// Returns the minimum size needed for the current content and Material 3
  /// padding constraints.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  // Content color for label and icon, resolved against the active theme.
  Color resolveContentColor() const;

  roo::string_view label_;
  const MonoIcon* icon_;
  ButtonVariant variant_;
  // Cache of the elevation reported on the previous paint, so that we can
  // notify the framework when elevation changes (e.g. press transitions on
  // the elevated variant).
  uint8_t last_elevation_;
};

}  // namespace material3
}  // namespace roo_windows
