#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class NaturalProbe : public Widget {
 public:
  NaturalProbe(ApplicationContext& context, YDim& natural_height)
      : Widget(context), natural_height_(natural_height) {}

  int requests = 0;
  int measurements = 0;

  Dimensions getSuggestedMinimumDimensions() const override {
    return {100, natural_height_};
  }

  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }

  void requestLayoutDescending() override {
    ++requests;
    Widget::requestLayoutDescending();
  }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    ++measurements;
    return {width.resolveSize(100), height.resolveSize(natural_height_)};
  }

 private:
  YDim& natural_height_;
};

class StaticPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

class LayoutRefreshTest : public ::testing::Test {
 protected:
  LayoutRefreshTest()
      : environment(scheduler),
        context(scheduler, environment.theme(),
                environment.keyboardColorTheme()) {}

  void layout(Widget& widget) {
    widget.measure(WidthSpec::Exactly(200), HeightSpec::Exactly(160));
    widget.layout(Rect(0, 0, 199, 159));
  }

  roo_scheduler::SchedulingService scheduler;
  Environment environment;
  ApplicationContext context;
};

// Verifies a static container refreshes nested cached measurements and visits
// gone/invisible children even when the ancestor already requests layout.
TEST_F(LayoutRefreshTest, RefreshesEveryStructuralDescendant) {
  YDim height = 60;
  NaturalProbe visible(context, height);
  NaturalProbe gone(context, height);
  NaturalProbe invisible(context, height);
  VerticalLayout column(context);
  column.add(visible);
  column.add(gone);
  column.add(invisible);
  StaticPanel root(context);
  root.add(column, Rect(0, 0, 199, 159));
  layout(root);
  gone.setVisibility(Visibility::kGone);
  invisible.setVisibility(Visibility::kInvisible);
  layout(root);
  EXPECT_FALSE(visible.isLayoutRequested());
  int previous_measurements = visible.measurements;
  height = 24;
  root.requestLayout();
  int gone_requests = gone.requests;
  int invisible_requests = invisible.requests;
  root.requestLayoutDescending();
  EXPECT_EQ(visible.measurements, previous_measurements);
  EXPECT_EQ(gone.requests, gone_requests + 1);
  EXPECT_EQ(invisible.requests, invisible_requests + 1);
  EXPECT_TRUE(gone.isLayoutRequested());
  layout(root);
  EXPECT_GT(visible.measurements, previous_measurements);
  EXPECT_EQ(visible.height(), 24);
  gone.setVisibility(Visibility::kVisible);
  layout(root);
  EXPECT_EQ(gone.height(), 24);
}

// Verifies retained detached roots can refresh directly and attaching a clean
// subtree recursively invalidates descendant measurements, even when gone.
TEST_F(LayoutRefreshTest, RefreshesDetachedAndReattachedRoots) {
  YDim height = 60;
  NaturalProbe child(context, height);
  VerticalLayout column(context);
  column.add(child);
  StaticPanel root(context);
  root.add(column, Rect(0, 0, 199, 159));
  layout(root);
  root.removeAll();
  height = 24;
  column.requestLayoutDescending();
  layout(column);
  EXPECT_EQ(child.height(), 24);
  EXPECT_FALSE(child.isLayoutRequested());
  height = 48;
  column.setVisibility(Visibility::kGone);
  root.add(column, Rect(0, 0, 199, 159));
  EXPECT_TRUE(child.isLayoutRequested());
  column.setVisibility(Visibility::kVisible);
  layout(root);
  EXPECT_EQ(child.height(), 48);
  root.removeAll();
  height = 36;
  EXPECT_FALSE(child.isLayoutRequested());
  root.add(column, Rect(0, 0, 199, 159));
  EXPECT_TRUE(child.isLayoutRequested());
  layout(root);
  EXPECT_EQ(child.height(), 36);
}

class RefreshRow : public VerticalLayout {
 public:
  RefreshRow(ApplicationContext& context, YDim& height)
      : VerticalLayout(context), content(context, height) {
    add(content);
  }

  ~RefreshRow() override { removeAll(); }

  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }

  NaturalProbe content;
};

class RefreshModel : public ListModel {
 public:
  int elementCount() const override { return 100; }

  void set(int index, Widget&) const override {
    EXPECT_GE(index, 0);
    EXPECT_LT(index, elementCount());
  }
};

class RefreshList : public ListLayout {
 public:
  using ListLayout::ListLayout;
  using ListLayout::poolCapacity;
  using ListLayout::prototype;
  using ListLayout::rowStride;

  RefreshRow* row(int index) {
    return static_cast<RefreshRow*>(materializedRow(index));
  }
};

// Verifies recursive refresh remeasures the detached nested prototype, visits
// each allocated row once, grows the pool for smaller strides, and lays out
// recycled offscreen rows with current geometry without model notifications.
TEST_F(LayoutRefreshTest, RefreshesVirtualStrideAndRetainedRows) {
  roo::byte pixels[240 * 320 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      240, 320, pixels, roo_display::Argb4444());
  roo_display::Display display(device);
  Application app(&environment, display);
  YDim height = 64;
  RefreshModel model;
  std::vector<RefreshRow*> allocations;
  RefreshList list(app.context(), model, [&]() {
    auto row = std::make_unique<RefreshRow>(app.context(), height);
    allocations.push_back(row.get());
    return row;
  });
  SimpleScrollablePanel scroll(app.context(), list);
  Task& task = app.addTaskFullScreen(scroll);
  app.refresh();
  EXPECT_EQ(list.rowStride(), 64);
  scroll.scrollTo(0, -640);
  app.refresh();
  const size_t initial_capacity = list.poolCapacity();
  for (YDim next_height : {32, 80, 64}) {
    std::vector<int> requests;
    for (RefreshRow* row : allocations) {
      requests.push_back(row->content.requests);
    }
    height = next_height;
    scroll.requestLayoutDescending();
    for (size_t i = 0; i < allocations.size(); ++i) {
      EXPECT_EQ(allocations[i]->content.requests, requests[i] + 1);
      EXPECT_TRUE(allocations[i]->content.isLayoutRequested());
    }
    scroll.measure(WidthSpec::Exactly(240), HeightSpec::Exactly(320));
    scroll.layout(scroll.parent_bounds());
    EXPECT_EQ(list.first(), 640 / height);
    EXPECT_EQ(list.last(), 959 / height);
    scroll.invalidateInterior();
    app.refresh();
    EXPECT_EQ(list.rowStride(), height);
    EXPECT_EQ(list.height(), 100 * height);
    EXPECT_EQ(list.offsetTop(), -640);
    EXPECT_EQ(list.first(), 640 / height);
    EXPECT_EQ(list.last(), 959 / height);
    if (height == 32) {
      EXPECT_GT(list.poolCapacity(), initial_capacity);
    }
    for (int index = list.first(); index <= list.last(); ++index) {
      ASSERT_NE(list.row(index), nullptr);
      EXPECT_EQ(list.row(index)->offsetTop(), index * height);
      EXPECT_EQ(list.row(index)->content.height(), height);
    }
    scroll.scrollTo(0, -1600);
    app.refresh();
    for (int index = list.first(); index <= list.last(); ++index) {
      ASSERT_NE(list.row(index), nullptr);
      EXPECT_EQ(list.row(index)->content.height(), height);
    }
    scroll.scrollTo(0, -640);
    app.refresh();
  }
  task.navigation().clear();
}

}  // namespace
}  // namespace roo_windows
