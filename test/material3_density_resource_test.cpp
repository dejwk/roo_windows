#include <cstdlib>
#include <iostream>
#include <new>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/filled/navigation.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace {
thread_local bool tracking = false;
thread_local size_t allocations = 0;

// Counts C++ allocation calls on the UI thread, including renderer streams.
void* Allocate(size_t bytes) {
  if (tracking) ++allocations;
  void* pointer = std::malloc(bytes == 0 ? 1 : bytes);
  if (pointer == nullptr) std::abort();
  return pointer;
}
}  // namespace

void* operator new(size_t bytes) { return Allocate(bytes); }
void* operator new[](size_t bytes) { return Allocate(bytes); }
void* operator new(size_t bytes, const std::nothrow_t&) noexcept {
  if (tracking) ++allocations;
  return std::malloc(bytes == 0 ? 1 : bytes);
}
void* operator new[](size_t bytes, const std::nothrow_t& tag) noexcept {
  return ::operator new(bytes, tag);
}
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, size_t) noexcept { std::free(pointer); }
void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  std::free(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  std::free(pointer);
}

namespace roo_windows::material3 {
namespace {
class DensityResourceTest : public testing::Test {
 protected:
  DensityResourceTest()
      : device_(240, 320, raster_, roo_display::Rgb565()),
        display_(device_),
        material_(DefaultTheme().material3Theme()),
        theme_{MakeFrameworkTheme(material_), &material_},
        environment_(scheduler_, theme_),
        app_(&environment_, display_) {}

  ApplicationContext& context() { return app_.context(); }

  void change(Density density) {
    material_.density = density;
    app_.root().requestLayoutDescending();
    app_.root().invalidateDescending();
    app_.refresh();
  }

  roo::byte raster_[240 * 320 * 2];
  roo_display::OffscreenDevice<roo_display::Rgb565> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Material3Theme material_;
  Theme theme_;
  Environment environment_;
  Application app_;
};

// Verifies real warmed geometry and paint paths introduce no additional C++
// allocations when density changes; existing renderer stream costs are
// separate.
TEST_F(DensityResourceTest, SteadyGeometryAndPaintAllocationCost) {
  Button button(context(), "Save");
  button.setIcon(&SCALED_ROO_ICON(filled, navigation_check));
  TextField filled(context(), "Device");
  filled.setText("Kitchen");
  TextField outlined(context(), "Room", TextFieldVariant::kOutlined);
  outlined.setText("Hall");
  ListRow<CheckboxListItem> row(context(), "Alerts");
  MenuRow<StandardMenuItem> menu(context(),
                                 StandardMenuItemInit{"Settings", {}});
  menu.item().setShortcut("Ctrl+S");
  menu.setMenuItem(menu.item());
  menu.clearDensityOverride();
  VerticalLayout column(context());
  column.add(button);
  column.add(filled);
  column.add(outlined);
  column.add(row);
  column.add(menu);
  Task& task = app_.addTaskFullScreen(column);
  app_.refresh();
  size_t baseline_geometry = 0;
  size_t baseline_paint = 0;
  for (int level = 0; level >= -5; --level) {
    change(static_cast<Density>(level));
    allocations = 0;
    tracking = true;
    Widget* widgets[] = {&button, &filled, &outlined, &row, &menu};
    for (int pass = 0; pass < 20; ++pass) {
      for (Widget* widget : widgets) {
        widget->getSuggestedMinimumDimensions();
        widget->getPreferredSize();
        widget->measure(WidthSpec::Exactly(220), HeightSpec::Unspecified(0));
      }
    }
    tracking = false;
    size_t geometry = allocations;
    app_.root().requestLayoutDescending();
    app_.refresh();
    allocations = 0;
    for (int frame = 0; frame < 20; ++frame) {
      app_.root().invalidateDescending();
      tracking = true;
      app_.refresh();
      tracking = false;
    }
    size_t paint = allocations;
    if (level == 0) {
      baseline_geometry = geometry;
      baseline_paint = paint;
    }
    EXPECT_EQ(geometry, 0u);
    EXPECT_LE(geometry, baseline_geometry);
    EXPECT_LE(paint, baseline_paint);
    std::cout << "density=" << level << " geometry_20=" << geometry
              << " paint_20=" << paint << '\n';
    RecordProperty("geometry_allocations_" + std::to_string(-level),
                   std::to_string(geometry));
    RecordProperty("paint_allocations_" + std::to_string(-level),
                   std::to_string(paint));
  }
  task.navigation().clear();
}

using ResourceRow = ListRow<HeadlineListItem>;
class ResourceModel : public DynamicListModel<ResourceRow> {
 public:
  int elementCount() const override { return 100; }
  void prepare(ResourceRow& row) const override {
    row.item().setHeadline("Device");
  }
  void bind(int, ResourceRow& row) const override {
    row.item().setHeadline("Device");
  }
};

class ResourceSection : public DynamicList<ResourceRow> {
 public:
  using DynamicList<ResourceRow>::DynamicList;
  using DynamicListBase::rowStride;
  using ListLayout::poolCapacity;
};

class ResourceList : public List {
 public:
  using List::List;
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }
};

// Verifies compaction grows retained storage only with the viewport's row
// count, and returning to zero reuses that capacity rather than rebuilding the
// model.
TEST_F(DensityResourceTest, VirtualPoolCapacityAndGrowth) {
  ResourceModel model;
  ResourceSection section(context(), model);
  ResourceList list(context());
  list.setVariant(ListVariant::kBaseline);
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Task& task = app_.addTaskFullScreen(scroll);
  app_.refresh();
  const Density levels[] = {Density::kDefault, Density::kMinus2,
                            Density::kMinus5, Density::kDefault};
  size_t maximum = 0;
  int transition = 0;
  for (Density density : levels) {
    change(density);
    size_t required = 320 / section.rowStride() + 2;
    maximum = std::max(maximum, required);
    EXPECT_EQ(section.poolCapacity(), maximum);
    std::cout << "density=" << static_cast<int>(density)
              << " stride=" << section.rowStride()
              << " pool=" << section.poolCapacity() << '\n';
    RecordProperty("pool_capacity_transition_" + std::to_string(transition++),
                   std::to_string(section.poolCapacity()));
  }
  task.navigation().clear();
}
}  // namespace
}  // namespace roo_windows::material3
