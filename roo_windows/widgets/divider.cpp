#include "roo_windows/widgets/divider.h"

#include "roo_display.h"
#include "roo_display/shape/basic_shapes.h"

namespace roo_windows {

bool HorizontalDivider::paint(const Canvas& canvas) {
  if (!isInvalidated()) return true;
  const Rect rect = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  canvas.fillRect(rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax() - 1,
                  color);
  color.set_a(0x20);
  canvas.drawHLine(rect.xMin(), rect.yMax(), rect.xMax(), color);
  return true;
}

bool VerticalDivider::paint(const Canvas& canvas) {
  if (!isInvalidated()) return true;
  const Rect rect = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  canvas.fillRect(rect.xMin(), rect.yMin(), rect.xMax() - 1, rect.yMax(),
                  color);
  color.set_a(0x20);
  canvas.drawVLine(rect.xMax(), rect.yMin(), rect.yMax(), color);
  return true;
}

}  // namespace roo_windows
