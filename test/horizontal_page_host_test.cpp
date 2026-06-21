#include "roo_windows/containers/horizontal_page_host.h"

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

// Verifies that the host auto-selects the first page and only the selected
// page is attached to the child tree in phase 1.
TEST(HorizontalPageHost, CurrentPageAttachmentTracksSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  HorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();

  EXPECT_EQ(-1, host.currentIndex());
  EXPECT_EQ(0, host.pageCount());

  host.addPage(std::move(first));
  host.addPage(std::move(second));

  EXPECT_EQ(0, host.currentIndex());
  EXPECT_EQ(2, host.pageCount());
  EXPECT_EQ(&host, first_ptr->parent());
  EXPECT_EQ(nullptr, second_ptr->parent());

  EXPECT_TRUE(host.setCurrentIndex(1, false));
  EXPECT_EQ(1, host.currentIndex());
  EXPECT_EQ(nullptr, first_ptr->parent());
  EXPECT_EQ(&host, second_ptr->parent());

  EXPECT_FALSE(host.setCurrentIndex(1, false));
}

// Verifies out-of-range programmatic selection is rejected without changing
// current index.
TEST(HorizontalPageHost, RejectsInvalidSelection) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  HorizontalPageHost host(context);
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

  HorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(80, 40));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  Dimensions d1 =
      host.measure(WidthSpec::Exactly(90), HeightSpec::Exactly(50));
  EXPECT_EQ(90, d1.width());
  EXPECT_EQ(50, d1.height());
  EXPECT_EQ(1, first_ptr->measureCount());
  EXPECT_EQ(0, second_ptr->measureCount());

  EXPECT_TRUE(host.setCurrentIndex(1, false));
  Dimensions d2 =
      host.measure(WidthSpec::Exactly(90), HeightSpec::Exactly(50));
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

  HorizontalPageHost host(context);
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

  HorizontalPageHost host(context);
  auto first = std::make_unique<ProbeWidget>(context, Dimensions(20, 10));
  auto second = std::make_unique<ProbeWidget>(context, Dimensions(30, 12));
  ProbeWidget* first_ptr = first.get();
  ProbeWidget* second_ptr = second.get();
  host.addPage(std::move(first));
  host.addPage(std::move(second));

  host.measure(WidthSpec::Exactly(64), HeightSpec::Exactly(40));
  host.layout(Rect(10, 20, 73, 59));
  EXPECT_EQ(1, first_ptr->layoutCount());
  EXPECT_EQ(Rect(0, 0, 63, 39), first_ptr->lastLayout());
  EXPECT_EQ(0, second_ptr->layoutCount());

  EXPECT_TRUE(host.setCurrentIndex(1, false));
  host.measure(WidthSpec::Exactly(64), HeightSpec::Exactly(40));
  host.layout(Rect(10, 20, 73, 59));
  EXPECT_EQ(1, second_ptr->layoutCount());
  EXPECT_EQ(Rect(0, 0, 63, 39), second_ptr->lastLayout());
}

// Verifies the checked Phase 1 size budget with pointer-size-aware limits to
// catch accidental permanent RAM growth in the base host type.
TEST(HorizontalPageHost, PhaseOneSizeBudget) {
  constexpr size_t kHorizontalPageHostBudget =
      sizeof(Container) + sizeof(std::vector<WidgetRef>) +
      sizeof(std::vector<int8_t>) + 8 * sizeof(void*) + 24;
  EXPECT_LE(sizeof(HorizontalPageHost), kHorizontalPageHostBudget);
}

}  // namespace
}  // namespace roo_windows