#pragma once

#include "roo_windows/core/widget.h"
#include "roo_windows/material3/theme.h"

namespace density_example {

/// Applies a validated persisted/external setting on the UI thread between
/// frames. Invalid integers retain the current choice without requesting work.
inline bool ApplySetting(int setting,
                         roo_windows::material3::Material3Theme& material,
                         roo_windows::Widget& root) {
  if (setting < -5 || setting > 0) return false;
  const auto next = static_cast<roo_windows::material3::Density>(setting);
  if (next == material.density) return true;
  material.density = next;
  root.requestLayoutDescending();
  root.invalidateDescending();
  return true;
}

}  // namespace density_example
