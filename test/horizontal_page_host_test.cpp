#include "roo_windows/containers/horizontal_page_host.h"

#include <vector>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

ApplicationContext MakeContext(Environment& bootstrap) {
  return ApplicationContext(bootstrap.scheduler(), bootstrap.theme(),
                            bootstrap.keyboardColorTheme());
}

class ProbeWidget : public BasicWidget {
 public:
  ProbeWidget(ApplicationContext& context, Dimensions natural)
      : BasicWidget(context),
        natural_(natural),
        measure_count_(0),
        layout_count_(0),
        last_layout_(0, 0, -1, -1) {}

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  Dimensions getSuggestedMinimumDimensions() const override { return natural_; }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    ++measure_count_;
    return Dimensions(width.resolveSize(natural_.width()),
                      height.resolveSize(natural_.height()));
  }

  void onLayout(bool changed, const Rect& rect) override {
    (void)changed;
    ++layout_count_;
    last_layout_ = rect;
  }

  int measureCount() const { return measure_count_; }
  int layoutCount() const { return layout_count_; }
  const Rect& lastLayout() const { return last_layout_; }

 private:
  Dimensions natural_;
  int measure_count_;
  int layout_count_;
  Rect last_layout_;
};

class TestHorizontalPageHost : public HorizontalPageHost {
 public:
  explicit TestHorizontalPageHost(ApplicationContext& context)
      : HorizontalPageHost(context) {}

  using HorizontalPageHost::onDrag;
  using HorizontalPageHost::onDragStart;

  struct IndexChange {
    int old_index;
    int new_index;
  };

  const std::vector<IndexChange>& targetChanges() const {
    return target_changes_;
  }

 protected:
  void onTargetIndexChanged(int old_index, int new_index) override {
    target_changes_.push_back(IndexChange{old_index, new_index});
  }

 private:
  std::vector<IndexChange> target_changes_;
};

const Container* HostForPage(const Widget* page) {
  const Container* wrapper = page->parent();
  return wrapper == nullptr ? nullptr : wrapper->parent();
}

Rect SlotBoundsForPage(const Widget* page) {
  const Container* wrapper = page->parent();
  if (wrapper == nullptr) return Rect(0, 0, -1, -1);
  return wrapper->parent_bounds();
}

// Verifies that the host auto-selects the first page and keeps the current
// page plus immediate neighbor attached in phase 2.
TEST(HorizontalPageHost, AdjacentAttachmentTracksSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  auto third = std::make_unique<ProbeWidget>(context, Dimensions(40, 14));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();
  ProbeWidget* third_ptr = third.get();

  EXPECT_EQ(-1, host.currentIndex());
  EXPECT_EQ(0, host.pageCount());

  host.addPage(std::move(first));
  host.addPage(std::move(second));
  host.addPage(std::move(third));

  EXPECT_EQ(0, host.currentIndex());
  EXPECT_EQ(3, host.pageCount());
  EXPECT_EQ(&host, HostForPage(first_ptr));
  EXPECT_EQ(&host, HostForPage(second_ptr));
  EXPECT_EQ(nullptr, third_ptr->parent());

  EXPECT_TRUE(host.setCurrentIndex(1, false));
  EXPECT_EQ(1, host.currentIndex());
  EXPECT_EQ(&host, HostForPage(first_ptr));
  EXPECT_EQ(&host, HostForPage(second_ptr));
  EXPECT_EQ(&host, HostForPage(third_ptr));

  EXPECT_TRUE(host.setCurrentIndex(2, false));
  EXPECT_EQ(nullptr, first_ptr->parent());
  EXPECT_EQ(&host, HostForPage(second_ptr));
  EXPECT_EQ(&host, HostForPage(third_ptr));

  EXPECT_FALSE(host.setCurrentIndex(2, false));
}

// Verifies out-of-range programmatic selection is rejected without changing
// current index.
TEST(HorizontalPageHost, RejectsInvalidSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  host.addPage(std::make_unique<ProbeWidget>(context, Dimensions(20, 10)));

  EXPECT_FALSE(host.setCurrentIndex(-1, false));
  EXPECT_FALSE(host.setCurrentIndex(2, false));
  EXPECT_EQ(0, host.currentIndex());
}

// Verifies exact viewport measurement reports the viewport size and only
// measures the currently active page.
TEST(HorizontalPageHost, ExactMeasureUsesViewportAndCurrentPageOnly) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(80, 40));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  Dimensions d1 = host.measure(WidthSpec::Exactly(90), HeightSpec::Exactly(50));
  EXPECT_EQ(90, d1.width());
  EXPECT_EQ(50, d1.height());
  EXPECT_EQ(1, first_ptr->measureCount());
  EXPECT_EQ(0, second_ptr->measureCount());

  EXPECT_TRUE(host.setCurrentIndex(1, false));
  Dimensions d2 = host.measure(WidthSpec::Exactly(90), HeightSpec::Exactly(50));
  EXPECT_EQ(90, d2.width());
  EXPECT_EQ(50, d2.height());
  EXPECT_EQ(1, second_ptr->measureCount());
}

// Verifies non-exact measurement scans all stored pages and resolves to a
// single viewport size that fits the largest page.
TEST(HorizontalPageHost, WrapMeasureUsesLargestPage) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto small = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto large = std::make_unique<ProbeWidget>(context, Dimensions(120, 80));
  ProbeWidget* small_ptr = small.get();
  ProbeWidget* large_ptr = large.get();
  host.addPage(std::move(small));
  host.addPage(std::move(large));

  Dimensions measured =
      host.measure(WidthSpec::AtMost(90), HeightSpec::AtMost(50));

  EXPECT_EQ(90, measured.width());
  EXPECT_EQ(50, measured.height());
  EXPECT_EQ(1, small_ptr->measureCount());
  EXPECT_EQ(1, large_ptr->measureCount());
}

// Verifies layout assigns the selected page to the full local viewport rect.
TEST(HorizontalPageHost, LayoutFillsViewportWithSelectedPage) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  host.measure(WidthSpec::Exactly(64), HeightSpec::Exactly(40));
  host.layout(Rect(10, 20, 73, 59));
  EXPECT_EQ(Rect(0, 0, 63, 39), SlotBoundsForPage(first_ptr));
  EXPECT_EQ(Rect(64, 0, 127, 39), SlotBoundsForPage(second_ptr));

  EXPECT_TRUE(host.setCurrentIndex(1, false));
  host.measure(WidthSpec::Exactly(64), HeightSpec::Exactly(40));
  host.layout(Rect(10, 20, 73, 59));
  EXPECT_EQ(Rect(-64, 0, -1, 39), SlotBoundsForPage(first_ptr));
  EXPECT_EQ(Rect(0, 0, 63, 39), SlotBoundsForPage(second_ptr));
}

// Verifies drag updates fractional page position and lays out current and next
// pages at the expected offsets for partial reveal.
TEST(HorizontalPageHost, DragRepositionsCurrentAndAdjacentPages) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  host.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(40));
  host.layout(Rect(0, 0, 99, 39));

  host.onDragStart(0, 0);
  host.onDrag(0, 0, -40, 0);

  EXPECT_EQ(Rect(-40, 0, 59, 39), SlotBoundsForPage(first_ptr));
  EXPECT_EQ(Rect(60, 0, 159, 39), SlotBoundsForPage(second_ptr));
}

// Verifies drag target notifications move selector surfaces, such as tabs,
// before the page has fully settled.
TEST(HorizontalPageHost, DragUpdatesTargetIndexAtSettleThreshold) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  host.addPage(std::make_unique<ProbeWidget>(context, Dimensions(20, 10)));
  host.addPage(std::make_unique<ProbeWidget>(context, Dimensions(30, 12)));

  host.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(40));
  host.layout(Rect(0, 0, 99, 39));

  EXPECT_EQ(0, host.currentIndex());
  EXPECT_EQ(0, host.targetIndex());

  host.onDragStart(0, 0);
  host.onDrag(0, 0, -49, 0);

  EXPECT_EQ(0, host.currentIndex());
  EXPECT_EQ(0, host.targetIndex());
  EXPECT_TRUE(host.targetChanges().empty());

  host.onDrag(0, 0, -1, 0);

  EXPECT_EQ(0, host.currentIndex());
  EXPECT_EQ(1, host.targetIndex());
  ASSERT_EQ(1u, host.targetChanges().size());
  EXPECT_EQ(0, host.targetChanges()[0].old_index);
  EXPECT_EQ(1, host.targetChanges()[0].new_index);

  host.onDrag(0, 0, 1, 0);

  EXPECT_EQ(0, host.currentIndex());
  EXPECT_EQ(0, host.targetIndex());
  ASSERT_EQ(2u, host.targetChanges().size());
  EXPECT_EQ(1, host.targetChanges()[1].old_index);
  EXPECT_EQ(0, host.targetChanges()[1].new_index);
}

// Verifies edge drag resistance at the first page uses capped scroll-panel
// style damping beyond the boundary.
TEST(HorizontalPageHost, EdgeResistanceDampsOutOfRangeDrag) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  ProbeWidget* first_ptr = first.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  host.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(40));
  host.layout(Rect(0, 0, 99, 39));

  host.onDragStart(0, 0);
  host.onDrag(0, 0, 100, 0);

  EXPECT_EQ(Rect(29, 0, 128, 39), SlotBoundsForPage(first_ptr));
}

// Verifies edge resistance is applied from accumulated raw drag distance, not
// from the previously damped visual page position.
TEST(HorizontalPageHost, EdgeResistanceAccumulatesRawDrag) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestHorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  ProbeWidget* first_ptr = first.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  host.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(40));
  host.layout(Rect(0, 0, 99, 39));

  host.onDragStart(0, 0);
  host.onDrag(0, 0, 100, 0);
  host.onDrag(0, 0, 100, 0);

  EXPECT_EQ(Rect(33, 0, 132, 39), SlotBoundsForPage(first_ptr));
}

// Verifies the phase-3 size budget with pointer-size-aware limits so bounded
// blit-wrapper state and swipe/settle fields stay contained.
TEST(HorizontalPageHost, PhaseThreeSizeBudget) {
  constexpr size_t kHorizontalPageHostBudget =
      sizeof(Container) + sizeof(std::vector<WidgetRef>) +
      sizeof(std::vector<int8_t>) + 17 * sizeof(void*) + 40;
  EXPECT_LE(sizeof(HorizontalPageHost), kHorizontalPageHostBudget);
}

}  // namespace
}  // namespace roo_windows
