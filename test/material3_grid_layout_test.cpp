#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

namespace roo_windows::material3 {
namespace {

class TestGridLayout : public GridLayout {
 public:
  using GridLayout::GridLayout;

  int childCount() const { return getChildrenCount(); }
};

class ProbeWidget : public BasicWidget {
 public:
  ProbeWidget(ApplicationContext& context, YDim natural_height)
      : BasicWidget(context), natural_height_(natural_height) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, natural_height_);
  }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    return Dimensions(width.resolveSize(1),
                      height.resolveSize(natural_height_));
  }

 private:
  YDim natural_height_;
};

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

void Layout(TestGridLayout& grid, XDim width, YDim height) {
  grid.measure(WidthSpec::Exactly(width), HeightSpec::Exactly(height));
  grid.layout(Rect(0, 0, width - 1, height - 1));
}

GridSpan Span(uint8_t compact, uint8_t medium = 4, uint8_t expanded = 4) {
  GridSpan span;
  span.compact = compact;
  span.medium = medium;
  span.expanded = expanded;
  span.large = expanded;
  span.extra_large = expanded;
  return span;
}

GridLayout::Params Params(GridSpan span,
                          VerticalGravity gravity = kGravityTop) {
  GridLayout::Params params;
  params.span = span;
  params.gravity = gravity;
  return params;
}

LayoutBreakpointPolicy FourColumnPolicy() {
  return LayoutBreakpointPolicy(BreakpointThresholds(), {4, 0, 2}, {4, 0, 2},
                                {4, 0, 2}, {4, 0, 2}, {4, 0, 2});
}

// Verifies row-major spans share the tallest row height, and bottom gravity
// aligns the shorter peer without creating a waterfall column.
TEST(Material3GridLayout, PacksMixedHeightRowsWithSharedRhythm) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget short_card(context, 10);
  ProbeWidget tall_card(context, 20);
  ProbeWidget next_row(context, 8);
  TestGridLayout grid(context);
  LayoutBreakpointPolicy policy = FourColumnPolicy();
  grid.setBreakpointPolicy(policy);
  grid.add(WidgetRef(short_card), Params(Span(2), kGravityBottom));
  grid.add(WidgetRef(tall_card), Params(Span(2)));
  grid.add(WidgetRef(next_row), Params(Span(4)));

  Layout(grid, 42, 60);

  EXPECT_EQ(Rect(0, 10, 19, 19), short_card.parent_bounds());
  EXPECT_EQ(Rect(22, 0, 41, 19), tall_card.parent_bounds());
  EXPECT_EQ(Rect(0, 22, 41, 29), next_row.parent_bounds());
}

// Verifies spans larger than the local ruler clamp to a full row instead of
// overflowing or creating an invalid zero-width cell.
TEST(Material3GridLayout, ClampsOverWideSpansToAvailableColumns) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget card(context, 10);
  TestGridLayout grid(context);
  LayoutBreakpointPolicy policy = FourColumnPolicy();
  grid.setBreakpointPolicy(policy);
  grid.add(WidgetRef(card), Params(Span(99)));

  Layout(grid, 42, 20);

  EXPECT_EQ(Rect(0, 0, 41, 9), card.parent_bounds());
}

// Verifies logical columns mirror in RTL while preserving row-major insertion
// order and the shared local ruler metrics.
TEST(Material3GridLayout, MirrorsLogicalColumnsForRtl) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context, 10);
  ProbeWidget trailing(context, 10);
  TestGridLayout grid(context);
  LayoutBreakpointPolicy policy = FourColumnPolicy();
  grid.setBreakpointPolicy(policy);
  grid.setLayoutDirection(LayoutDirection::kRightToLeft);
  grid.add(WidgetRef(leading), Params(Span(2)));
  grid.add(WidgetRef(trailing), Params(Span(2)));

  Layout(grid, 42, 20);

  EXPECT_EQ(Rect(22, 0, 41, 9), leading.parent_bounds());
  EXPECT_EQ(Rect(0, 0, 19, 9), trailing.parent_bounds());
  EXPECT_EQ(LayoutDirection::kRightToLeft, grid.metrics().direction);
}

// Verifies each grid resolves spans from its own compact, medium, and expanded
// width class rather than inheriting an outer scaffold's breakpoint.
TEST(Material3GridLayout, ResolvesSpansFromLocalBreakpoint) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget card(context, 10);
  TestGridLayout grid(context);
  grid.add(WidgetRef(card), Params(Span(4, 2, 3)));

  Layout(grid, 500, 30);
  EXPECT_EQ(LayoutBreakpoint::kCompact, grid.metrics().breakpoint);
  EXPECT_EQ(grid.metrics().spanBounds(0, 4, 0, 10), card.parent_bounds());

  Layout(grid, Scaled(600), 30);

  EXPECT_EQ(LayoutBreakpoint::kMedium, grid.metrics().breakpoint);
  EXPECT_EQ(8, grid.metrics().columns);
  EXPECT_EQ(grid.metrics().spanBounds(0, 2, 0, 10), card.parent_bounds());

  Layout(grid, Scaled(840), 30);
  EXPECT_EQ(LayoutBreakpoint::kExpanded, grid.metrics().breakpoint);
  EXPECT_EQ(12, grid.metrics().columns);
  EXPECT_EQ(grid.metrics().spanBounds(0, 3, 0, 10), card.parent_bounds());
}

// Verifies clearing borrowed children removes them from the specialized item
// vector and detaches each parent relationship.
TEST(Material3GridLayout, ClearsBorrowedChildren) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget first(context, 10);
  ProbeWidget second(context, 10);
  TestGridLayout grid(context);
  grid.add(WidgetRef(first));
  grid.add(WidgetRef(second));
  ASSERT_EQ(2, grid.childCount());

  grid.clear();

  EXPECT_EQ(nullptr, first.parent());
  EXPECT_EQ(nullptr, second.parent());
  EXPECT_EQ(0, grid.childCount());
}

}  // namespace
}  // namespace roo_windows::material3
