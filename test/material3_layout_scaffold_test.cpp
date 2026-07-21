#include <limits>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

namespace roo_windows::material3 {
namespace {

class TestLayoutScaffold : public LayoutScaffold {
 public:
  using LayoutScaffold::LayoutScaffold;

  int childCount() const { return getChildrenCount(); }
};

class ProbeWidget : public BasicWidget {
 public:
  ProbeWidget(ApplicationContext& context, XDim natural_width,
              YDim natural_height)
      : BasicWidget(context),
        natural_width_(natural_width),
        natural_height_(natural_height) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(natural_width_, natural_height_);
  }

  bool isFocusable() const override { return true; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    return Dimensions(width.resolveSize(natural_width_),
                      height.resolveSize(natural_height_));
  }

 private:
  XDim natural_width_;
  YDim natural_height_;
};

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

void Layout(TestLayoutScaffold& scaffold, XDim width, YDim height) {
  scaffold.measure(WidthSpec::Exactly(width), HeightSpec::Exactly(height));
  scaffold.layout(Rect(0, 0, width - 1, height - 1));
}

// Verifies all inclusive lower boundaries are compared after dp scaling.
TEST(Material3LayoutScaffold, ResolvesExactScaledBreakpointBoundaries) {
  const LayoutBreakpointPolicy& policy = LayoutBreakpointPolicy::Default();
  EXPECT_EQ(LayoutBreakpoint::kCompact, policy.resolveWidthPx(Scaled(600) - 1));
  EXPECT_EQ(LayoutBreakpoint::kMedium, policy.resolveWidthPx(Scaled(600)));
  EXPECT_EQ(LayoutBreakpoint::kMedium, policy.resolveWidthPx(Scaled(840) - 1));
  EXPECT_EQ(LayoutBreakpoint::kExpanded, policy.resolveWidthPx(Scaled(840)));
  EXPECT_EQ(LayoutBreakpoint::kLarge, policy.resolveWidthPx(Scaled(1200)));
  EXPECT_EQ(LayoutBreakpoint::kExtraLarge, policy.resolveWidthPx(Scaled(1600)));
  EXPECT_EQ(LayoutBreakpoint::kCompact, policy.resolveWidthPx(-1));
}

// Verifies the process-lifetime default preserves the documented 4/8/12
// Material ruler rhythm.
TEST(Material3LayoutScaffold, DefaultPolicyUsesDocumentedRulerTokens) {
  const LayoutBreakpointPolicy& policy = LayoutBreakpointPolicy::Default();
  EXPECT_EQ(&policy, &LayoutBreakpointPolicy::Default());
  EXPECT_EQ(4, policy.tokens(LayoutBreakpoint::kCompact).columns);
  EXPECT_EQ(16, policy.tokens(LayoutBreakpoint::kCompact).outer_margin_dp);
  EXPECT_EQ(16, policy.tokens(LayoutBreakpoint::kMedium).gutter_dp);
  EXPECT_EQ(8, policy.tokens(LayoutBreakpoint::kMedium).columns);
  EXPECT_EQ(12, policy.tokens(LayoutBreakpoint::kExpanded).columns);
  EXPECT_EQ(32, policy.tokens(LayoutBreakpoint::kLarge).outer_margin_dp);
  EXPECT_EQ(40, policy.tokens(LayoutBreakpoint::kExtraLarge).outer_margin_dp);
}

// Verifies ranges are inclusive and inverted ranges deliberately participate
// in no breakpoint.
TEST(Material3LayoutScaffold, BreakpointRangeHandlesBoundariesAndInversion) {
  const BreakpointRange medium_to_large = {LayoutBreakpoint::kMedium,
                                           LayoutBreakpoint::kLarge};
  EXPECT_FALSE(medium_to_large.contains(LayoutBreakpoint::kCompact));
  EXPECT_TRUE(medium_to_large.contains(LayoutBreakpoint::kMedium));
  EXPECT_TRUE(medium_to_large.contains(LayoutBreakpoint::kLarge));
  EXPECT_FALSE(medium_to_large.contains(LayoutBreakpoint::kExtraLarge));
  const BreakpointRange inverted = {LayoutBreakpoint::kLarge,
                                    LayoutBreakpoint::kMedium};
  EXPECT_FALSE(inverted.contains(LayoutBreakpoint::kLarge));
}

// Verifies narrow rectangles clamp margin, reduce columns, and never emit
// invalid ruler geometry.
TEST(Material3LayoutScaffold, DegradesRulerForTinyAndEmptyGeometry) {
  const LayoutBreakpointPolicy& policy = LayoutBreakpointPolicy::Default();
  const LayoutMetrics tiny = policy.resolveMetrics(Rect(10, 2, 12, 8));
  EXPECT_EQ(LayoutBreakpoint::kCompact, tiny.breakpoint);
  EXPECT_EQ(1, tiny.columns);
  EXPECT_EQ(0, tiny.gutter);
  EXPECT_EQ(1, tiny.column_width);
  EXPECT_EQ(Rect(11, 2, 11, 8), tiny.content_bounds);

  const LayoutMetrics empty = policy.resolveMetrics(Rect(0, 0, -1, -1));
  EXPECT_TRUE(empty.safe_bounds.empty());
  EXPECT_TRUE(empty.content_bounds.empty());
  EXPECT_EQ(1, empty.columns);
  EXPECT_EQ(0, empty.column_width);
  EXPECT_EQ(0, empty.columnStart(0));
  EXPECT_TRUE(empty.spanBounds(0, 1, 0, 1).empty());
}

// Verifies ruler remainder is centered from the logical leading edge and
// logical columns mirror rather than renumber in RTL.
TEST(Material3LayoutScaffold, MirrorsRulerRemainderAndSpansForRtl) {
  LayoutBreakpointPolicy policy(BreakpointThresholds(), {4, 0, 1}, {4, 0, 1},
                                {4, 0, 1}, {4, 0, 1}, {4, 0, 1});
  const Rect safe(0, 10, 11, 20);
  const LayoutMetrics ltr = policy.resolveMetrics(safe);
  const LayoutMetrics rtl =
      policy.resolveMetrics(safe, LayoutDirection::kRightToLeft);

  EXPECT_EQ(Rect(0, 10, 10, 20), ltr.content_bounds);
  EXPECT_EQ(Rect(1, 10, 11, 20), rtl.content_bounds);
  EXPECT_EQ(0, ltr.columnStart(0));
  EXPECT_EQ(10, rtl.columnStart(0));
  EXPECT_EQ(Rect(3, 12, 7, 15), ltr.spanBounds(1, 2, 12, 4));
  EXPECT_EQ(Rect(4, 12, 8, 15), rtl.spanBounds(1, 2, 12, 4));
  EXPECT_TRUE(ltr.spanBounds(4, 1, 0, 1).empty());
  EXPECT_TRUE(ltr.spanBounds(0, 0, 0, 1).empty());
  EXPECT_TRUE(ltr.spanBounds(0, 1, 0, 0).empty());
}

// Verifies spans clamp at the final logical column without overflowing it.
TEST(Material3LayoutScaffold, ClampsOversizedSpansAtFinalColumn) {
  const LayoutMetrics metrics =
      LayoutBreakpointPolicy::Default().resolveMetrics(
          Rect(0, 0, Scaled(600) - 2, 9));
  ASSERT_EQ(4, metrics.columns);
  EXPECT_EQ(metrics.content_bounds,
            metrics.spanBounds(0, std::numeric_limits<uint8_t>::max(), 0, 10));
}

// Verifies full-width bars, safety insets, rails, and body/chrome geometry
// resolve together without forcing the ruler margin into the body.
TEST(Material3LayoutScaffold, PlacesChromeAndPublishesBodyGeometry) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget top(context, 1, 10);
  ProbeWidget bottom(context, 1, 12);
  ProbeWidget leading(context, 20, 1);
  ProbeWidget trailing(context, 30, 1);
  ProbeWidget body(context, 1, 1);
  TestLayoutScaffold scaffold(context);
  scaffold.setSafetyInsets(Insets(5, 3, 7, 4));
  scaffold.setTopBar(WidgetRef(top));
  scaffold.setBottomBar(WidgetRef(bottom));
  scaffold.setLeadingRail(WidgetRef(leading), BreakpointRange());
  scaffold.setTrailingRail(WidgetRef(trailing), BreakpointRange());
  scaffold.setBody(WidgetRef(body));

  Layout(scaffold, 200, 100);

  EXPECT_EQ(Rect(5, 3, 192, 12), top.parent_bounds());
  EXPECT_EQ(Rect(5, 84, 192, 95), bottom.parent_bounds());
  EXPECT_EQ(Rect(5, 13, 24, 83), leading.parent_bounds());
  EXPECT_EQ(Rect(163, 13, 192, 83), trailing.parent_bounds());
  EXPECT_EQ(Rect(25, 13, 162, 83), body.parent_bounds());
  EXPECT_EQ(body.parent_bounds(), scaffold.bodyBounds());
  EXPECT_EQ(Insets(25, 13, 37, 16), scaffold.contentInsets());
  EXPECT_EQ(bottom.parent_bounds(), scaffold.bottomBarBounds());
  EXPECT_EQ(LayoutBreakpoint::kCompact, scaffold.metrics().breakpoint);
  EXPECT_EQ(Rect(25, 13, 162, 83), scaffold.metrics().safe_bounds);
}

// Verifies a scaffold retains the outer breakpoint for ruler tokens after its
// rails narrow the body below that breakpoint.
TEST(Material3LayoutScaffold, UsesOuterBreakpointForBodyRulerMetrics) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context, 500, 1);
  ProbeWidget body(context, 1, 1);
  TestLayoutScaffold scaffold(context);
  scaffold.setLeadingRail(WidgetRef(leading), BreakpointRange());
  scaffold.setBody(WidgetRef(body));

  Layout(scaffold, Scaled(840), 80);

  EXPECT_EQ(LayoutBreakpoint::kExpanded, scaffold.metrics().breakpoint);
  EXPECT_EQ(12, scaffold.metrics().columns);
  EXPECT_EQ(Scaled(24), scaffold.metrics().outer_margin);
  EXPECT_EQ(Scaled(24), scaffold.metrics().gutter);
}

// Verifies excluded chrome becomes gone, clears focus, receives empty bounds,
// and returns body space when the breakpoint changes.
TEST(Material3LayoutScaffold, ExcludesChromeByBreakpointWithoutReplacingIt) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget bottom(context, 1, 12);
  ProbeWidget body(context, 1, 1);
  TestLayoutScaffold scaffold(context);
  const BreakpointRange compact_only = {LayoutBreakpoint::kCompact,
                                        LayoutBreakpoint::kCompact};
  scaffold.setBottomBar(WidgetRef(bottom), compact_only);
  scaffold.setBody(WidgetRef(body));

  Layout(scaffold, 200, 80);
  ASSERT_TRUE(context.focus().requestFocus(bottom));
  ASSERT_TRUE(bottom.isFocused());
  ASSERT_TRUE(bottom.isVisible());
  EXPECT_EQ(Rect(0, 0, 199, 67), body.parent_bounds());

  Layout(scaffold, Scaled(600), 80);
  EXPECT_TRUE(bottom.isGone());
  EXPECT_FALSE(bottom.isFocused());
  EXPECT_TRUE(bottom.parent_bounds().empty());
  EXPECT_TRUE(scaffold.bottomBarBounds().empty());
  EXPECT_EQ(Rect(0, 0, Scaled(600) - 1, 79), body.parent_bounds());
}

// Verifies logical rails mirror in RTL while physical safety insets remain on
// their caller-specified sides.
TEST(Material3LayoutScaffold, MirrorsRailsForRtl) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context, 20, 1);
  ProbeWidget trailing(context, 30, 1);
  ProbeWidget body(context, 1, 1);
  TestLayoutScaffold scaffold(context);
  scaffold.setSafetyInsets(Insets(5, 0, 7, 0));
  scaffold.setLeadingRail(WidgetRef(leading), BreakpointRange());
  scaffold.setTrailingRail(WidgetRef(trailing), BreakpointRange());
  scaffold.setBody(WidgetRef(body));
  scaffold.setLayoutDirection(LayoutDirection::kRightToLeft);

  Layout(scaffold, 200, 80);

  EXPECT_EQ(Rect(173, 0, 192, 79), leading.parent_bounds());
  EXPECT_EQ(Rect(5, 0, 34, 79), trailing.parent_bounds());
  EXPECT_EQ(Rect(35, 0, 172, 79), body.parent_bounds());
  EXPECT_EQ(Insets(35, 0, 27, 0), scaffold.contentInsets());
}

// Verifies fixed slots detach borrowed children and clear body geometry.
TEST(Material3LayoutScaffold, ClearsBorrowedSlotsAndEmptyBodyGeometry) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget top(context, 1, 10);
  ProbeWidget body(context, 1, 1);
  TestLayoutScaffold scaffold(context);
  scaffold.setTopBar(WidgetRef(top));
  scaffold.setBody(WidgetRef(body));
  ASSERT_EQ(2, scaffold.childCount());
  Layout(scaffold, 100, 60);

  scaffold.clearTopBar();
  scaffold.setBody(WidgetRef());
  Layout(scaffold, 100, 60);

  EXPECT_EQ(nullptr, top.parent());
  EXPECT_EQ(nullptr, body.parent());
  EXPECT_EQ(0, scaffold.childCount());
  EXPECT_TRUE(scaffold.bodyBounds().empty());
  EXPECT_EQ(Insets::Zero(), scaffold.contentInsets());
}

}  // namespace
}  // namespace roo_windows::material3
