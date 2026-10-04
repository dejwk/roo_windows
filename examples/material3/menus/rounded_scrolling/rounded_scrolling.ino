// Learning goal: combine clipped scrolling content with an explicitly
// unclipped status marker that can overhang a rounded container.

#include "examples/material3/menus/example_runtime.h"
#include "roo_display/shape/smooth.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/panel.h"
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

  // The outer card owns the rounded clip, so this scrolling body needs no
  // special paint implementation.
  Color background() const override { return Color(0xFFF8EFF8); }
};

// A small foreground marker implemented as a deferred smooth overlay. Its
// bounds deliberately extend above and to the right of the schedule card.
class StatusMarker final : public Widget {
 public:
  using Widget::Widget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(44, 20);
  }

  void paint(PaintContext& ctx) const override {
    ctx.addOverlayShape(roo_display::SmoothFilledRoundRect(
        0, 0, width() - 1, height() - 1, 10, Color(0xFFFF7A45)));
  }

 protected:
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }
};

class ScheduleCard final : public Panel {
 public:
  explicit ScheduleCard(ApplicationContext& context) : Panel(context) {
    auto marker = std::make_unique<StatusMarker>(context);
    marker->setParentClipMode(ParentClipMode::kUnclipped);
    // Insert the marker first. Rounded child grouping still paints it above the
    // later clipped scrolling body, so collection order remains layout order.
    add(std::move(marker), Rect(212, -8, 255, 11));
    add(std::make_unique<ScheduleList>(context), Rect(0, 0, 239, 191));
  }

  bool clipsChildrenToRoundedBounds() const override { return true; }
  BorderStyle getBorderStyle() const override { return BorderStyle(24, 0); }
  Color background() const override { return Color(0xFFF8EFF8); }
};

}  // namespace

void setup() {
  auto& app = material3_menu_example::app;
  app.add(std::make_unique<Backdrop>(app.context()),
          roo_display::Box(0, 0, 319, 239));
  app.add(std::make_unique<ScheduleCard>(app.context()),
          roo_display::Box(40, 24, 279, 215));
  // Shared runtime configures the display/touch pins and the optional emulator.
  material3_menu_example::Start();
}

void loop() {}
