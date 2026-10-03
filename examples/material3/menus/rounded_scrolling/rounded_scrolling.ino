// Drag the schedule list to see selected rows meet the rounded viewport edge.
// The experimental container hook clips all descendant output automatically.

#include "examples/material3/menus/example_runtime.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/menu/menu.h"

using namespace roo_windows;

namespace {

// A patterned backdrop makes translucent edge coverage visible while dragging.
class Backdrop final : public Widget {
 public:
  using Widget::Widget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(320, 240);
  }
  void paint(PaintContext& ctx) const override {
    for (int y = 0; y < height(); y += 12) {
      for (int x = 0; x < width(); x += 12) {
        ctx.fillRect(Rect(x, y, x + 11, y + 11), ((x / 12 + y / 12) & 1) == 0
                                                     ? Color(0xFFE8DED0)
                                                     : Color(0xFFD2C7B7));
      }
    }
  }
};

class ScheduleList final : public SimpleScrollablePanel {
 public:
  explicit ScheduleList(ApplicationContext& context)
      : SimpleScrollablePanel(context) {
    auto group = std::make_unique<material3::MenuGroup>(context);
    // Borrowed labels are literals. The group owns each row and its item.
    const char* labels[] = {"Monday", "Tuesday",  "Wednesday", "Thursday",
                            "Friday", "Saturday", "Sunday"};
    for (int i = 0; i < 7; ++i) {
      material3::StandardMenuItemInit item;
      item.headline = labels[i];
      item.flags = material3::StandardMenuItemFlags::kSelectable;
      if ((i & 1) == 0) {
        item.flags = item.flags | material3::StandardMenuItemFlags::kSelected;
      }
      group->add(
          std::make_unique<material3::MenuRow<material3::StandardMenuItem>>(
              context, item));
    }
    setContents(std::move(group));
  }

  // No paint override or child changes are needed to enable smooth clipping.
  bool clipsChildrenToRoundedBounds() const override { return true; }
  BorderStyle getBorderStyle() const override { return BorderStyle(24, 0); }
  Color background() const override { return Color(0xFFF8EFF8); }
};

}  // namespace

void setup() {
  auto& app = material3_menu_example::app;
  app.add(std::make_unique<Backdrop>(app.context()),
          roo_display::Box(0, 0, 319, 239));
  app.add(std::make_unique<ScheduleList>(app.context()),
          roo_display::Box(40, 24, 279, 215));
  // Shared runtime configures the display/touch pins and the optional emulator.
  material3_menu_example::Start();
}

void loop() {}
