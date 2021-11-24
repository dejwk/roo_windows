#include "roo_windows/widgets/divider.h"

#include "roo_display.h"
#include "roo_display/shape/basic_shapes.h"

namespace roo_windows {

bool HorizontalDivider::paint(const Surface& s) {
  if (!isInvalidated()) return true;
  const Box box = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  s.drawObject(roo_display::FilledRect(box.xMin(), box.yMax() - 1, box.xMax(),
                                       box.yMax() - 1, color));
  color.set_a(0x20);
  s.drawObject(roo_display::FilledRect(box.xMin(), box.yMax(), box.xMax(),
                                       box.yMax(), color));
  return true;
}

bool VerticalDivider::paint(const Surface& s) {
  if (!isInvalidated()) return true;
  const Box box = bounds();
  Color color = theme().color.onBackground;
  color.set_a(0x40);
  s.drawObject(roo_display::FilledRect(box.xMax() - 1, box.yMin(),
                                       box.xMax() - 1, box.yMax(), color));
  color.set_a(0x20);
  s.drawObject(roo_display::FilledRect(box.xMax(), box.yMin(), box.xMax(),
                                       box.yMax(), color));
  return true;
}

}  // namespace roo_windows
