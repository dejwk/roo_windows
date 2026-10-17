#include "roo_windows/material3/divider/divider.h"

#include <algorithm>

namespace roo_windows::material3 {

Color HorizontalDivider::dividerColor() const {
  return theme().material3Theme().color.outlineVariant;
}

int16_t HorizontalDivider::dividerThickness() const {
  return std::max<int16_t>(1, Scaled(1));
}

void HorizontalDivider::paint(PaintContext& ctx) const {
  if (!isDirty()) return;
  const Rect rect = bounds();
  ctx.fillRect(rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax(),
               dividerColor());
}

Color VerticalDivider::dividerColor() const {
  return theme().material3Theme().color.outlineVariant;
}

int16_t VerticalDivider::dividerThickness() const {
  return std::max<int16_t>(1, Scaled(1));
}

void VerticalDivider::paint(PaintContext& ctx) const {
  if (!isDirty()) return;
  const Rect rect = bounds();
  ctx.fillRect(rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax(),
               dividerColor());
}

}  // namespace roo_windows::material3
