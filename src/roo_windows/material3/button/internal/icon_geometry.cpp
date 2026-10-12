#include "roo_windows/material3/button/internal/icon_geometry.h"

#include <algorithm>

namespace roo_windows::material3::internal {

const IconGeometryTokens& IconGeometryFor(ButtonSize size) {
  static constexpr IconGeometryTokens kTokens[] = {{32, 20, 8, 6},
                                                   {40, 24, 12, 8},
                                                   {56, 24, 16, 12},
                                                   {96, 32, 28, 16},
                                                   {136, 40, 28, 16}};
  return kTokens[static_cast<uint8_t>(size)];
}

int16_t IconContainerWidth(ButtonSize size, IconButtonWidth width) {
  const IconGeometryTokens& tokens = IconGeometryFor(size);
  int16_t height = Scaled(tokens.height_dp);
  switch (width) {
    case IconButtonWidth::kNarrow:
      return std::max<int16_t>(Scaled(tokens.icon_size_dp), height - Scaled(8));
    case IconButtonWidth::kWide:
      return height + Scaled(8);
    case IconButtonWidth::kUniform:
      return height;
  }
  return height;
}

Dimensions ResolveIconSlot(ButtonSize size, const MonoIcon& icon,
                           const MonoIcon* alternate) {
  int16_t nominal = Scaled(IconGeometryFor(size).icon_size_dp);
  int16_t width = std::max<int16_t>(nominal, icon.anchorExtents().width());
  int16_t height = std::max<int16_t>(nominal, icon.anchorExtents().height());
  if (alternate != nullptr) {
    width = std::max<int16_t>(width, alternate->anchorExtents().width());
    height = std::max<int16_t>(height, alternate->anchorExtents().height());
  }
  return {width, height};
}
}  // namespace roo_windows::material3::internal
