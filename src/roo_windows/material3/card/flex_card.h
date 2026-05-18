#pragma once

#include <inttypes.h>

#include "roo_windows/containers/flex_layout.h"

namespace roo_windows {
namespace material3 {
/// Material 3 card backed by a `FlexLayout` for its contents.
///
/// Combines one of the three card styles (elevated, filled, outlined) with the
/// flexible child layout of `FlexLayout`. Each visual aspect that the style
/// normally derives from the theme - container role, outline role, elevation,
/// outline width, and corner radius - can be individually overridden through
/// the `setXxx()` / `clearXxxOverride()` accessors.
class FlexCard : public FlexLayout {
 public:
  enum class Style : uint8_t {
    kElevated,
    kFilled,
    kOutlined,
  };

  explicit FlexCard(const Environment& env, Style style = Style::kFilled,
                    FlexDirection direction = FlexDirection::kColumn);

  Style style() const { return style_; }
  /// Switches the card style. Resets any previously overridden visual
  /// tokens to the new style's defaults; explicit overrides set after this
  /// call still take effect.
  void setStyle(Style style);

  /// Overrides the container color role used by the card's surface.
  void setContainerRole(ColorRole role);
  ColorRole containerRoleOverride() const { return container_role_override_; }
  /// Reverts to the style-default container color role.
  void clearContainerRoleOverride();

  /// Overrides the outline color role (only visible for the outlined style).
  void setOutlineRole(ColorRole role);
  ColorRole outlineRoleOverride() const { return outline_role_override_; }
  /// Reverts to the style-default outline color role.
  void clearOutlineRoleOverride();

  /// Overrides the resting elevation (dp).
  void setElevation(uint8_t elevation);
  bool hasElevationOverride() const {
    return (override_flags_ & kOverrideElevation) != 0;
  }
  uint8_t elevationOverride() const { return elevation_override_; }
  /// Reverts to the style-default elevation.
  void clearElevationOverride();

  /// Overrides the outline stroke width (only meaningful for outlined).
  void setOutlineWidth(SmallNumber outline_width);
  bool hasOutlineWidthOverride() const {
    return (override_flags_ & kOverrideOutlineWidth) != 0;
  }
  SmallNumber outlineWidthOverride() const { return outline_width_override_; }
  /// Reverts to the style-default outline width.
  void clearOutlineWidthOverride();

  /// Overrides the corner radius (px).
  void setCornerRadius(uint8_t radius);
  bool hasCornerRadiusOverride() const {
    return (override_flags_ & kOverrideCornerRadius) != 0;
  }
  uint8_t cornerRadiusOverride() const { return corner_radius_override_; }
  /// Reverts to the style-default corner radius.
  void clearCornerRadiusOverride();

  /// Returns either the configured override or the style-default container
  /// color role.
  ColorRole containerRole() const override;
  /// Returns the outline color resolved from the (overridden) outline role.
  roo_display::Color getOutlineColor() const override;
  /// Returns a `BorderStyle` built from the (overridden) corner radius and
  /// outline width.
  BorderStyle getBorderStyle() const override;
  /// Returns the (overridden) resting elevation in dp.
  uint8_t getElevation() const override;

 private:
  enum OverrideFlags : uint8_t {
    kOverrideContainerRole = 0x01,
    kOverrideOutlineRole = 0x02,
    kOverrideElevation = 0x04,
    kOverrideOutlineWidth = 0x08,
    kOverrideCornerRadius = 0x10,
  };

  struct Tokens {
    ColorRole container_role;
    ColorRole outline_role;
    uint8_t elevation;
    SmallNumber outline_width;
    uint8_t corner_radius;
  };

  static Tokens styleDefaults(Style style);

  Style style_;
  uint8_t override_flags_;

  uint8_t elevation_override_;
  uint8_t corner_radius_override_;
  SmallNumber outline_width_override_;
  ColorRole container_role_override_;
  ColorRole outline_role_override_;
};

}  // namespace material3
}  // namespace roo_windows
