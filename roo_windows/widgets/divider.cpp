#include "roo_windows/widgets/divider.h"

#include "roo_display.h"
#include "roo_display/shape/basic_shapes.h"

namespace roo_windows {

bool HorizontalDivider::paint(const Canvas& canvas) {
  if (!isInvalidated()) return true;
  const Rect rect = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  canvas.drawObject(roo_display::FilledRect(
      rect.xMin(), rect.yMin(), rect.xMax(), rect.yMax() - 1, color));
  color.set_a(0x20);
  canvas.drawObject(roo_display::FilledRect(rect.xMin(), rect.yMax(),
                                            rect.xMax(), rect.yMax(), color));
  return true;
}

bool VerticalDivider::paint(const Canvas& canvas) {
  if (!isInvalidated()) return true;
  const Rect rect = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  canvas.drawObject(roo_display::FilledRect(
      rect.xMin(), rect.yMin(), rect.xMax() - 1, rect.yMax(), color));
  color.set_a(0x20);
  canvas.drawObject(roo_display::FilledRect(rect.xMax(), rect.yMin(),
                                            rect.xMax(), rect.yMax(), color));
  return true;
}

}  // namespace roo_windows
