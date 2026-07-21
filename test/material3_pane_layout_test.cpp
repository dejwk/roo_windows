#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

namespace roo_windows::material3 {
namespace {

class TestPaneLayout : public PaneLayout {
 public:
  using PaneLayout::PaneLayout;

  int childCount() const { return getChildrenCount(); }
};

class ProbeWidget : public BasicWidget {
 public:
  explicit ProbeWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  bool isFocusable() const override { return true; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    return Dimensions(width.resolveSize(1), height.resolveSize(1));
  }
};

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

void Layout(TestPaneLayout& panes, XDim width, YDim height) {
  panes.measure(WidthSpec::Exactly(width), HeightSpec::Exactly(height));
  panes.layout(Rect(0, 0, width - 1, height - 1));
}

PaneSpec AllBreakpoints(int16_t minimum, int16_t preferred) {
  PaneSpec spec;
  spec.min_width_dp = minimum;
  spec.preferred_width_dp = preferred;
  spec.simultaneous_visibility = BreakpointRange();
  return spec;
}

// Verifies compact presentation follows the caller-selected pane rather than
// using an implicit list/detail navigation policy.
TEST(Material3PaneLayout, ShowsOnlyTheSelectedPaneOnCompactWidths) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context);
  ProbeWidget main(context);
  TestPaneLayout panes(context);
  panes.setLeadingPane(WidgetRef(leading));
  panes.setMainPane(WidgetRef(main));

  ASSERT_TRUE(panes.setActivePane(PaneRole::kLeading));
  Layout(panes, 300, 80);

  EXPECT_EQ(Rect(0, 0, 299, 79), leading.parent_bounds());
  EXPECT_TRUE(main.parent_bounds().empty());
  EXPECT_TRUE(panes.isLeadingVisible());
  EXPECT_FALSE(panes.isMainVisible());
  EXPECT_EQ(LayoutBreakpoint::kCompact, panes.metrics().breakpoint);

  ASSERT_TRUE(panes.setActivePane(PaneRole::kMain));
  Layout(panes, 300, 80);
  EXPECT_TRUE(leading.parent_bounds().empty());
  EXPECT_EQ(Rect(0, 0, 299, 79), main.parent_bounds());
}

// Verifies a breakpoint-eligible supporting pane docks beside main and leaves
// the policy-scaled gutter between the two tracks.
TEST(Material3PaneLayout, DocksSupportingPaneBesideMain) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget main(context);
  ProbeWidget trailing(context);
  TestPaneLayout panes(context);
  panes.setMainMinWidthDp(100);
  panes.setMainPane(WidgetRef(main));
  panes.setTrailingPane(WidgetRef(trailing), AllBreakpoints(100, 200));

  Layout(panes, 500, 60);

  EXPECT_EQ(Rect(0, 0, 283, 59), main.parent_bounds());
  EXPECT_EQ(Rect(300, 0, 499, 59), trailing.parent_bounds());
  EXPECT_TRUE(panes.isMainVisible());
  EXPECT_TRUE(panes.isTrailingVisible());
}

// Verifies all three panes retain canonical logical order, then collapse in
// reverse priority while the active main pane remains available.
TEST(Material3PaneLayout, LaysOutThreePanesThenCollapsesTrailingFirst) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context);
  ProbeWidget main(context);
  ProbeWidget trailing(context);
  TestPaneLayout panes(context);
  panes.setMainMinWidthDp(200);
  panes.setLeadingPane(WidgetRef(leading), AllBreakpoints(100, 150));
  panes.setMainPane(WidgetRef(main));
  panes.setTrailingPane(WidgetRef(trailing), AllBreakpoints(100, 150));

  Layout(panes, 700, 70);
  EXPECT_EQ(Rect(0, 0, 149, 69), leading.parent_bounds());
  EXPECT_EQ(Rect(166, 0, 533, 69), main.parent_bounds());
  EXPECT_EQ(Rect(550, 0, 699, 69), trailing.parent_bounds());

  Layout(panes, 430, 70);
  EXPECT_EQ(Rect(0, 0, 149, 69), leading.parent_bounds());
  EXPECT_EQ(Rect(166, 0, 429, 69), main.parent_bounds());
  EXPECT_TRUE(trailing.parent_bounds().empty());
  EXPECT_TRUE(trailing.isGone());
}

// Verifies disabling simultaneous panes forces the selected pane to fill the
// local rectangle, even when other panes remain attached.
TEST(Material3PaneLayout, ShortHeightCallerOverrideForcesSinglePane) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context);
  ProbeWidget main(context);
  TestPaneLayout panes(context);
  panes.setLeadingPane(WidgetRef(leading), AllBreakpoints(100, 150));
  panes.setMainPane(WidgetRef(main));
  panes.setMultiPaneEnabled(false);

  Layout(panes, 900, 50);

  EXPECT_EQ(Rect(0, 0, 899, 49), main.parent_bounds());
  EXPECT_TRUE(leading.parent_bounds().empty());
}

// Verifies leading and trailing remain logical identities in RTL and hidden
// panes clear focus before receiving empty bounds.
TEST(Material3PaneLayout, MirrorsLogicalSidesAndClearsHiddenPaneFocus) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context);
  ProbeWidget main(context);
  ProbeWidget trailing(context);
  TestPaneLayout panes(context);
  panes.setMainMinWidthDp(100);
  panes.setLeadingPane(WidgetRef(leading), AllBreakpoints(100, 150));
  panes.setMainPane(WidgetRef(main));
  panes.setTrailingPane(WidgetRef(trailing), AllBreakpoints(100, 150));
  panes.setLayoutDirection(LayoutDirection::kRightToLeft);

  Layout(panes, 500, 50);
  ASSERT_TRUE(context.focus().requestFocus(trailing));
  EXPECT_EQ(Rect(350, 0, 499, 49), leading.parent_bounds());
  EXPECT_EQ(Rect(166, 0, 333, 49), main.parent_bounds());
  EXPECT_EQ(Rect(0, 0, 149, 49), trailing.parent_bounds());

  Layout(panes, 200, 50);
  EXPECT_TRUE(leading.parent_bounds().empty());
  EXPECT_TRUE(trailing.parent_bounds().empty());
  EXPECT_FALSE(trailing.isFocused());
  EXPECT_EQ(Rect(0, 0, 199, 49), main.parent_bounds());
}

// Verifies clearing the selected slot preserves its logical selection and
// produces an intentionally empty presentation until the caller chooses one.
TEST(Material3PaneLayout, ClearingActivePaneDoesNotChooseReplacement) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  ProbeWidget leading(context);
  ProbeWidget main(context);
  TestPaneLayout panes(context);
  panes.setLeadingPane(WidgetRef(leading));
  panes.setMainPane(WidgetRef(main));
  ASSERT_TRUE(panes.setActivePane(PaneRole::kLeading));
  panes.clearLeadingPane();

  Layout(panes, 300, 50);

  EXPECT_EQ(PaneRole::kLeading, panes.activePane());
  EXPECT_EQ(nullptr, leading.parent());
  EXPECT_TRUE(main.parent_bounds().empty());
  EXPECT_FALSE(panes.setActivePane(PaneRole::kTrailing));
  EXPECT_EQ(1, panes.childCount());
}

}  // namespace
}  // namespace roo_windows::material3
