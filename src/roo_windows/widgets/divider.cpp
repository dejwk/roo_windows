#include "roo_windows/widgets/divider.h"

#include "roo_display.h"
#include "roo_display/shape/basic.h"

namespace roo_windows {

void HorizontalDivider::paint(PaintContext& ctx) const {
  if (!isInvalidated()) return;
  const Rect rect = bounds();
  Color color = theme().framework.color.resolve(FrameworkColorRole::kContent);
  color.set_a(0x40);
  ctx.fillRect(rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax() - 1, color);
  color.set_a(0x20);
  ctx.drawHLine(rect.xMin(), rect.yMax(), rect.xMax(), color);
}

void VerticalDivider::paint(PaintContext& ctx) const {
  if (!isInvalidated()) return;
  const Rect rect = bounds();
  Color color = theme().framework.color.resolve(FrameworkColorRole::kContent);
  color.set_a(0x40);
  ctx.fillRect(rect.xMin(), rect.yMin(), rect.xMax() - 1, rect.yMax(), color);
  color.set_a(0x20);
  ctx.drawVLine(rect.xMax(), rect.yMin(), rect.yMax(), color);
}

}  // namespace roo_windows
