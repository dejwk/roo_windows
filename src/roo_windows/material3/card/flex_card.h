#pragma once

#include <inttypes.h>

#include "roo_windows/containers/flex_layout.h"

namespace roo_windows {
namespace material3 {

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
  void setStyle(Style style);

  void setContainerRole(ColorRole role);
  ColorRole containerRoleOverride() const { return container_role_override_; }
  void clearContainerRoleOverride();

  void setOutlineRole(ColorRole role);
  ColorRole outlineRoleOverride() const { return outline_role_override_; }
  void clearOutlineRoleOverride();

  void setElevation(uint8_t elevation);
  bool hasElevationOverride() const {
    return (override_flags_ & kOverrideElevation) != 0;
  }
  uint8_t elevationOverride() const { return elevation_override_; }
  void clearElevationOverride();

  void setOutlineWidth(SmallNumber outline_width);
  bool hasOutlineWidthOverride() const {
    return (override_flags_ & kOverrideOutlineWidth) != 0;
  }
  SmallNumber outlineWidthOverride() const { return outline_width_override_; }
  void clearOutlineWidthOverride();

  void setCornerRadius(uint8_t radius);
  bool hasCornerRadiusOverride() const {
    return (override_flags_ & kOverrideCornerRadius) != 0;
  }
  uint8_t cornerRadiusOverride() const { return corner_radius_override_; }
  void clearCornerRadiusOverride();

  ColorRole containerRole() const override;
  roo_display::Color getOutlineColor() const override;
  BorderStyle getBorderStyle() const override;
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
