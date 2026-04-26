#include "roo_windows/material3/card/flex_card.h"

namespace roo_windows {
namespace material3 {

FlexCard::FlexCard(const Environment& env, Style style, FlexDirection direction)
    : FlexLayout(env, direction),
      style_(style),
      override_flags_(0),
      elevation_override_(0),
      corner_radius_override_(0),
      outline_width_override_(0),
      container_role_override_(ColorRole::kUndefined),
      outline_role_override_(ColorRole::kUndefined) {
  Tokens defaults = styleDefaults(style_);
  container_role_override_ = defaults.container_role;
  outline_role_override_ = defaults.outline_role;
  elevation_override_ = defaults.elevation;
  outline_width_override_ = defaults.outline_width;
  corner_radius_override_ = defaults.corner_radius;

  setMargins(MarginSize::kRegular);
  setPadding(Padding(PaddingSize::kRegular, PaddingSize::kRegular));
}

void FlexCard::setStyle(Style style) {
  if (style_ == style) return;

  Tokens before{container_role_override_, outline_role_override_,
                elevation_override_, outline_width_override_,
                corner_radius_override_};

  style_ = style;
  Tokens defaults = styleDefaults(style_);

  if ((override_flags_ & kOverrideContainerRole) == 0) {
    container_role_override_ = defaults.container_role;
  }
  if ((override_flags_ & kOverrideOutlineRole) == 0) {
    outline_role_override_ = defaults.outline_role;
  }
  if ((override_flags_ & kOverrideElevation) == 0) {
    elevation_override_ = defaults.elevation;
  }
  if ((override_flags_ & kOverrideOutlineWidth) == 0) {
    outline_width_override_ = defaults.outline_width;
  }
  if ((override_flags_ & kOverrideCornerRadius) == 0) {
    corner_radius_override_ = defaults.corner_radius;
  }

  Tokens after{container_role_override_, outline_role_override_,
               elevation_override_, outline_width_override_,
               corner_radius_override_};

  if (before.container_role != after.container_role ||
      before.outline_role != after.outline_role ||
      before.outline_width != after.outline_width ||
      before.corner_radius != after.corner_radius) {
    invalidateInterior();
  }

  if (before.elevation != after.elevation) {
    elevationChanged(before.elevation > after.elevation ? before.elevation
                                                        : after.elevation);
  } else if (before.corner_radius != after.corner_radius) {
    elevationChanged(after.elevation);
  }
}

void FlexCard::setContainerRole(ColorRole role) {
  if ((override_flags_ & kOverrideContainerRole) != 0 &&
      container_role_override_ == role) {
    return;
  }
  ColorRole before = container_role_override_;
  override_flags_ |= kOverrideContainerRole;
  container_role_override_ = role;
  if (before != container_role_override_) {
    invalidateInterior();
  }
}

void FlexCard::clearContainerRoleOverride() {
  if ((override_flags_ & kOverrideContainerRole) == 0) return;
  ColorRole before = container_role_override_;
  override_flags_ &= ~kOverrideContainerRole;
  container_role_override_ = styleDefaults(style_).container_role;
  if (before != container_role_override_) {
    invalidateInterior();
  }
}

void FlexCard::setOutlineRole(ColorRole role) {
  if ((override_flags_ & kOverrideOutlineRole) != 0 &&
      outline_role_override_ == role) {
    return;
  }
  ColorRole before = outline_role_override_;
  override_flags_ |= kOverrideOutlineRole;
  outline_role_override_ = role;
  if (before != outline_role_override_) {
    invalidateInterior();
  }
}

void FlexCard::clearOutlineRoleOverride() {
  if ((override_flags_ & kOverrideOutlineRole) == 0) return;
  ColorRole before = outline_role_override_;
  override_flags_ &= ~kOverrideOutlineRole;
  outline_role_override_ = styleDefaults(style_).outline_role;
  if (before != outline_role_override_) {
    invalidateInterior();
  }
}

void FlexCard::setElevation(uint8_t elevation) {
  if ((override_flags_ & kOverrideElevation) != 0 &&
      elevation_override_ == elevation) {
    return;
  }
  uint8_t old_elevation = elevation_override_;
  override_flags_ |= kOverrideElevation;
  elevation_override_ = elevation;
  uint8_t new_elevation = elevation_override_;
  if (old_elevation != new_elevation) {
    elevationChanged(old_elevation > new_elevation ? old_elevation
                                                   : new_elevation);
  }
}

void FlexCard::clearElevationOverride() {
  if ((override_flags_ & kOverrideElevation) == 0) return;
  uint8_t old_elevation = elevation_override_;
  override_flags_ &= ~kOverrideElevation;
  elevation_override_ = styleDefaults(style_).elevation;
  uint8_t new_elevation = elevation_override_;
  if (old_elevation != new_elevation) {
    elevationChanged(old_elevation > new_elevation ? old_elevation
                                                   : new_elevation);
  }
}

void FlexCard::setOutlineWidth(SmallNumber outline_width) {
  if ((override_flags_ & kOverrideOutlineWidth) != 0 &&
      outline_width_override_ == outline_width) {
    return;
  }
  SmallNumber old_outline_width = outline_width_override_;
  override_flags_ |= kOverrideOutlineWidth;
  outline_width_override_ = outline_width;
  if (old_outline_width != outline_width_override_) {
    invalidateInterior();
  }
}

void FlexCard::clearOutlineWidthOverride() {
  if ((override_flags_ & kOverrideOutlineWidth) == 0) return;
  SmallNumber old_outline_width = outline_width_override_;
  override_flags_ &= ~kOverrideOutlineWidth;
  outline_width_override_ = styleDefaults(style_).outline_width;
  if (old_outline_width != outline_width_override_) {
    invalidateInterior();
  }
}

void FlexCard::setCornerRadius(uint8_t radius) {
  if ((override_flags_ & kOverrideCornerRadius) != 0 &&
      corner_radius_override_ == radius) {
    return;
  }
  uint8_t old_radius = corner_radius_override_;
  override_flags_ |= kOverrideCornerRadius;
  corner_radius_override_ = radius;
  uint8_t new_radius = corner_radius_override_;
  if (old_radius != new_radius) {
    invalidateInterior();
    elevationChanged(elevation_override_);
  }
}

void FlexCard::clearCornerRadiusOverride() {
  if ((override_flags_ & kOverrideCornerRadius) == 0) return;
  uint8_t old_radius = corner_radius_override_;
  override_flags_ &= ~kOverrideCornerRadius;
  corner_radius_override_ = styleDefaults(style_).corner_radius;
  uint8_t new_radius = corner_radius_override_;
  if (old_radius != new_radius) {
    invalidateInterior();
    elevationChanged(elevation_override_);
  }
}

ColorRole FlexCard::containerRole() const {
  return container_role_override_;
}

roo_display::Color FlexCard::getOutlineColor() const {
  ColorRole role = outline_role_override_;
  return role == ColorRole::kUndefined ? roo_display::color::Transparent
                                       : theme().color.role(role);
}

BorderStyle FlexCard::getBorderStyle() const {
  return BorderStyle(corner_radius_override_, outline_width_override_);
}

uint8_t FlexCard::getElevation() const {
  return elevation_override_;
}

FlexCard::Tokens FlexCard::styleDefaults(Style style) {
  switch (style) {
    case Style::kElevated:
      return Tokens{ColorRole::kSurfaceContainerLow, ColorRole::kUndefined, 3,
                    SmallNumber(0), (uint8_t)Scaled(12)};
    case Style::kFilled:
      return Tokens{ColorRole::kSurfaceContainerHighest, ColorRole::kUndefined,
                    0, SmallNumber(0), (uint8_t)Scaled(12)};
    case Style::kOutlined:
      return Tokens{ColorRole::kSurface, ColorRole::kOutlineVariant, 0,
                    SmallNumber::Of16ths(Scaled(16)), (uint8_t)Scaled(12)};
  }
  return Tokens{ColorRole::kSurfaceContainerHighest, ColorRole::kUndefined, 0,
                SmallNumber(0), (uint8_t)Scaled(12)};
}

}  // namespace material3
}  // namespace roo_windows
