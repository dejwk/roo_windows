#include <memory>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/toolbar/toolbar.h"

namespace roo_windows::material3 {
namespace {

class Probe : public Widget {
 public:
  Probe(ApplicationContext& context, XDim width, YDim height,
        Margins margins = Margins(0), int* deleted = nullptr)
      : Widget(context),
        width_(width),
        height_(height),
        margins_(margins),
        deleted_(deleted) {}

  ~Probe() override {
    if (deleted_ != nullptr) ++*deleted_;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(width_, height_);
  }

  Margins getMargins() const override { return margins_; }

 private:
  XDim width_;
  YDim height_;
  Margins margins_;
  int* deleted_;
};

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

void MeasureAndLayout(DockedToolbar& toolbar, XDim width) {
  const Dimensions dimensions =
      toolbar.measure(WidthSpec::Exactly(width), HeightSpec::Unspecified(0));
  toolbar.layout(Rect(0, 0, dimensions.width() - 1, dimensions.height() - 1));
}

// Verifies grouped spacing, centering, edge placement, and deterministic RTL
// mirroring for the three illustrated 40 px controls.
TEST(Material3Toolbar, DockedArrangementsAndRtl) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar toolbar(context);
  Probe leading(context, 40, 40);
  Probe center(context, 40, 40);
  Probe trailing(context, 40, 40);
  toolbar.add(leading, ToolbarSlot::kLeading);
  toolbar.add(center, ToolbarSlot::kCenter);
  toolbar.add(trailing, ToolbarSlot::kTrailing);

  MeasureAndLayout(toolbar, 320);
  EXPECT_EQ(8, leading.parent_bounds().xMin());
  EXPECT_EQ(140, center.parent_bounds().xMin());
  EXPECT_EQ(272, trailing.parent_bounds().xMin());
  EXPECT_EQ(12, leading.parent_bounds().yMin());

  toolbar.setArrangement(DockedToolbarArrangement::kCentered);
  MeasureAndLayout(toolbar, 320);
  EXPECT_EQ(84, leading.parent_bounds().xMin());
  EXPECT_EQ(140, center.parent_bounds().xMin());
  EXPECT_EQ(196, trailing.parent_bounds().xMin());

  toolbar.setArrangement(DockedToolbarArrangement::kEdgeAnchored);
  MeasureAndLayout(toolbar, 320);
  EXPECT_EQ(8, leading.parent_bounds().xMin());
  EXPECT_EQ(140, center.parent_bounds().xMin());
  EXPECT_EQ(272, trailing.parent_bounds().xMin());
  toolbar.setLayoutDirection(LayoutDirection::kRightToLeft);
  MeasureAndLayout(toolbar, 320);
  EXPECT_EQ(272, leading.parent_bounds().xMin());
  EXPECT_EQ(140, center.parent_bounds().xMin());
  EXPECT_EQ(8, trailing.parent_bounds().xMin());
  toolbar.clear();
}

// Verifies overfull groups retain their gaps and clip at the content edge.
TEST(Material3Toolbar, DockedOverfullClipsWithoutOverlap) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar toolbar(context);
  Probe first(context, 42, 40);
  Probe second(context, 42, 40);
  Probe third(context, 42, 40);
  Probe fourth(context, 42, 40);
  Probe fifth(context, 40, 40);
  toolbar.add(first, ToolbarSlot::kLeading);
  toolbar.add(second, ToolbarSlot::kLeading);
  toolbar.add(third, ToolbarSlot::kCenter);
  toolbar.add(fourth, ToolbarSlot::kCenter);
  toolbar.add(fifth, ToolbarSlot::kTrailing);
  toolbar.setArrangement(DockedToolbarArrangement::kEdgeAnchored);
  MeasureAndLayout(toolbar, 256);
  EXPECT_EQ(8, first.parent_bounds().xMin());
  EXPECT_EQ(54, second.parent_bounds().xMin());
  EXPECT_EQ(112, third.parent_bounds().xMin());
  EXPECT_EQ(158, fourth.parent_bounds().xMin());
  EXPECT_EQ(216, fifth.parent_bounds().xMin());
  EXPECT_EQ(247, fifth.parent_bounds().xMax());
  toolbar.clear();
}

// Verifies edge anchoring falls back to centering when the requested anchors
// collide even though the packed strip itself fits the available width.
TEST(Material3Toolbar, EdgeCollisionCentersWholeStrip) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar toolbar(context);
  Probe leading(context, 100, 40);
  Probe center(context, 100, 40);
  Probe trailing(context, 20, 40);
  toolbar.add(leading, ToolbarSlot::kLeading);
  toolbar.add(center, ToolbarSlot::kCenter);
  toolbar.add(trailing, ToolbarSlot::kTrailing);
  toolbar.setArrangement(DockedToolbarArrangement::kEdgeAnchored);
  MeasureAndLayout(toolbar, 320);
  EXPECT_EQ(34, leading.parent_bounds().xMin());
  EXPECT_EQ(150, center.parent_bounds().xMin());
  EXPECT_EQ(266, trailing.parent_bounds().xMin());
  toolbar.clear();
}

// Verifies unbounded measurement wraps controls, invisible controls retain
// space, and a gone control is refreshed when it becomes visible again.
TEST(Material3Toolbar, UnboundedAndVisibilityChanges) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar toolbar(context);
  Probe first(context, 40, 40);
  Probe second(context, 40, 40);
  toolbar.add(first, ToolbarSlot::kLeading);
  toolbar.add(second, ToolbarSlot::kTrailing);
  second.setVisibility(Visibility::kInvisible);
  Dimensions measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(112, measured.width());
  EXPECT_EQ(64, measured.height());
  second.setVisibility(Visibility::kGone);
  measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(56, measured.width());
  second.setVisibility(Visibility::kVisible);
  measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(112, measured.width());
  toolbar.clear();
}

// Verifies borrowed controls survive removal, adopted controls are deleted,
// and visibility preserves structural enumeration but removes gone spacing.
TEST(Material3Toolbar, OwnershipVisibilityAndNullInsertion) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar toolbar(context);
  Probe borrowed(context, 40, 40);
  int deleted = 0;
  toolbar.add(WidgetRef());
  toolbar.add(borrowed, ToolbarSlot::kLeading);
  toolbar.add(std::make_unique<Probe>(context, 40, 40, Margins(0), &deleted),
              ToolbarSlot::kTrailing);
  EXPECT_EQ(2, toolbar.childCount());
  borrowed.setVisibility(Visibility::kGone);
  MeasureAndLayout(toolbar, 100);
  EXPECT_TRUE(borrowed.parent_bounds().empty());
  toolbar.clear(ToolbarSlot::kTrailing);
  EXPECT_EQ(1, deleted);
  EXPECT_EQ(1, toolbar.childCount());
  toolbar.clear();
  EXPECT_EQ(nullptr, borrowed.parent());
  EXPECT_EQ(0, toolbar.childCount());
}

// Verifies oversized exact children keep their measure while layout trims
// them to the padded interior, including nonzero child margins.
TEST(Material3Toolbar, ExactOversizeAndMargins) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar toolbar(context);
  Probe child(context, 200, 40, Margins(3, 2, 5, 4));
  toolbar.add(child);
  MeasureAndLayout(toolbar, 100);
  EXPECT_EQ(11, child.parent_bounds().xMin());
  EXPECT_EQ(91, child.parent_bounds().xMax());
  EXPECT_EQ(
      40,
      child.measure(WidthSpec::Exactly(200), HeightSpec::AtMost(100)).height());
  toolbar.clear();
}

}  // namespace
}  // namespace roo_windows::material3
