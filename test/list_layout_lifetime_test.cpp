#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {
class Rows : public ListModel {
 public:
  int elementCount() const override { return 40; }
  void set(int, Widget&) const override {}
};
class Row : public BasicWidget {
 public:
  explicit Row(ApplicationContext& context) : BasicWidget(context) {}
  Dimensions getSuggestedMinimumDimensions() const override {
    return {100, 72};
  }
  void paint(PaintContext& context) const override { context.clear(); }
};

class SpacedRow : public Row {
 public:
  explicit SpacedRow(ApplicationContext& context) : Row(context) {}

  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(), PreferredSize::ExactHeight(72)};
  }

  Margins getDefaultMargins() const override { return Margins(8, 1); }
};

// Verifies a parent's preferred-height constraint preserves row margins instead
// of squeezing them into the fixed row height.
TEST(ListLayoutLifetime, PreferredHeightIncludesRowMargins) {
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  ApplicationContext context(scheduler, environment.theme(),
                             environment.keyboardColorTheme());
  Rows model;
  ListLayout list(context, model,
                  [&]() { return std::make_unique<SpacedRow>(context); });
  const PreferredSize preferred = static_cast<Widget&>(list).getPreferredSize();
  ASSERT_TRUE(preferred.height().isExact());
  EXPECT_EQ(preferred.height().value(), 40 * 74);
  EXPECT_EQ(list.measure(WidthSpec::Exactly(320), HeightSpec::Unspecified(0))
                .height(),
            preferred.height().value());
}

// Verifies a populated row pool detaches before destruction; under ASan this
// catches Panel reading row ownership flags after the pool has freed its rows.
TEST(ListLayoutLifetime, DestroysPopulatedPoolAfterNavigationClear) {
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  roo::byte pixels[240 * 320 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      240, 320, pixels, roo_display::Argb4444());
  roo_display::Display display(device);
  Application app(&environment, display);
  Rows model;
  ListLayout list(app.context(), model,
                  [&]() { return std::make_unique<Row>(app.context()); });
  SimpleScrollablePanel scroll(app.context(), list);
  Task& task = app.addTaskFullScreen(scroll);
  ASSERT_TRUE(app.refresh());
  scroll.scrollTo(0, -200);
  ASSERT_TRUE(app.refresh());
  EXPECT_GT(list.first(), 0);
  task.navigation().clear();
}

class MutableRows : public ListModel {
 public:
  int count = 0;
  mutable int binds = 0;
  int elementCount() const override { return count; }
  void set(int index, Widget&) const override {
    EXPECT_GE(index, 0);
    EXPECT_LT(index, count);
    ++binds;
  }
};

class InspectableList : public ListLayout {
 public:
  using ListLayout::ListLayout;
  using ListLayout::poolCapacity;
  int releases = 0;

 protected:
  void unbindRow(Widget&) override { ++releases; }
};

// Verifies empty/offscreen ranges never fabricate a binding and shrink releases
// every old binding while retaining the viewport-sized allocation budget.
TEST(ListLayoutLifetime, EmptyOffscreenShrinkAndRetainedCapacity) {
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  roo::byte pixels[240 * 320 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      240, 320, pixels, roo_display::Argb4444());
  roo_display::Display display(device);
  Application app(&environment, display);
  MutableRows model;
  int allocations = 0;
  InspectableList list(app.context(), model, [&]() {
    ++allocations;
    return std::make_unique<Row>(app.context());
  });
  SimpleScrollablePanel scroll(app.context(), list);
  Task& task = app.addTaskFullScreen(scroll);
  app.refresh();
  EXPECT_EQ(model.binds, 0);
  model.count = 100;
  list.modelChanged();
  app.refresh();
  EXPECT_GT(model.binds, 0);
  const int warm_allocations = allocations;
  scroll.scrollTo(0, -3000);
  app.refresh();
  EXPECT_GT(list.first(), 30);
  EXPECT_EQ(allocations, warm_allocations);
  int binds = model.binds;
  list.invalidateInterior();
  app.refresh();
  EXPECT_EQ(model.binds, binds);
  model.count = 0;
  list.modelChanged();
  app.refresh();
  EXPECT_GT(list.releases, 0);
  EXPECT_LT(list.last(), list.first());
  EXPECT_EQ(allocations, warm_allocations);
  model.count = 2;
  list.modelChanged();
  scroll.scrollTo(0, 0);
  app.refresh();
  EXPECT_LE(list.last(), 1);
  EXPECT_EQ(allocations, warm_allocations);
  task.navigation().clear();
}

// Verifies checked extents reject overflow rather than compressing row heights.
TEST(ListLayoutLifetime, RejectsUnrepresentableContentExtent) {
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  ApplicationContext context(scheduler, environment.theme(),
                             environment.keyboardColorTheme());
  MutableRows model;
  model.count = 1000000;
  ListLayout list(context, model,
                  [&]() { return std::make_unique<Row>(context); });
  EXPECT_DEATH(
      list.measure(WidthSpec::Exactly(240), HeightSpec::Unspecified(0)), "");
}

}  // namespace
}  // namespace roo_windows
