#include <limits>

#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

namespace roo_windows::material3 {
namespace {

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

}  // namespace
}  // namespace roo_windows::material3
