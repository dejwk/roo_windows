#include <algorithm>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/static_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/icon_button.h"
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

class Keys : public KeySource {
 public:
  void send(KeyPhase phase, KeyCode code, PhysicalKey physical) {
    events_.push_back({phase, code, 0, physical, 0});
    notifyReady();
  }

  int drain(KeyEvent* output, int capacity) override {
    const int count = std::min<int>(capacity, events_.size());
    std::copy_n(events_.begin(), count, output);
    events_.erase(events_.begin(), events_.begin() + count);
    return count;
  }

 private:
  bool hasPendingEvents() const override { return !events_.empty(); }
  std::vector<KeyEvent> events_;
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

// Verifies the floating strip's natural dimensions, orientation swap, exact
// expansion, and physical mirroring against three 40 px icon buttons. End
// button centers coincide with the centers of the toolbar's rounded caps.
TEST(Material3Toolbar, FloatingGeometryOrientationAndRtl) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  FloatingToolbar toolbar(context);
  IconButton first(context, ic_outlined_24_action_done());
  IconButton second(context, ic_outlined_24_action_done());
  IconButton third(context, ic_outlined_24_action_done());
  toolbar.add(first);
  toolbar.add(second);
  toolbar.add(third);

  Dimensions measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(168, measured.width());
  EXPECT_EQ(64, measured.height());
  toolbar.layout(Rect(0, 0, 167, 63));
  EXPECT_EQ(12, first.parent_bounds().xMin());
  EXPECT_EQ(64, second.parent_bounds().xMin());
  EXPECT_EQ(116, third.parent_bounds().xMin());
  EXPECT_EQ(12, first.parent_bounds().yMin());
  EXPECT_EQ(40, first.width());
  EXPECT_EQ(40, first.height());
  EXPECT_EQ(12, toolbar.width() - third.parent_bounds().xMax() - 1);
  EXPECT_EQ(32, toolbar.getBorderStyle().top_left_corner_radius());

  measured = toolbar.measure(WidthSpec::Exactly(200), HeightSpec::Exactly(64));
  toolbar.layout(Rect(0, 0, 199, 63));
  EXPECT_EQ(28, first.parent_bounds().xMin());
  toolbar.setLayoutDirection(LayoutDirection::kRightToLeft);
  toolbar.measure(WidthSpec::Exactly(200), HeightSpec::Exactly(64));
  toolbar.layout(Rect(0, 0, 199, 63));
  EXPECT_EQ(132, first.parent_bounds().xMin());
  EXPECT_EQ(28, third.parent_bounds().xMin());

  toolbar.setOrientation(FloatingToolbarOrientation::kVertical);
  measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(64, measured.width());
  EXPECT_EQ(168, measured.height());
  toolbar.layout(Rect(0, 0, 63, 167));
  EXPECT_EQ(12, first.parent_bounds().xMin());
  EXPECT_EQ(12, first.parent_bounds().yMin());
  EXPECT_EQ(64, second.parent_bounds().yMin());
  EXPECT_EQ(116, third.parent_bounds().yMin());
  EXPECT_EQ(12, toolbar.height() - third.parent_bounds().yMax() - 1);

  third.setVisibility(Visibility::kGone);
  second.setVisibility(Visibility::kInvisible);
  measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(116, measured.height());
  toolbar.layout(Rect(0, 0, 63, 115));
  EXPECT_EQ(64, second.parent_bounds().yMin());
  EXPECT_TRUE(third.parent_bounds().empty());
  toolbar.clear();
}

// Verifies empty strips retain the 64 dp floor and user-selected elevation
// and surface roles leave child state untouched.
TEST(Material3Toolbar, FloatingEmptySurfaceAndElevation) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  FloatingToolbar toolbar(context);
  Dimensions measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(64, measured.width());
  EXPECT_EQ(64, measured.height());
  EXPECT_EQ(ColorToken::kSurfaceContainer, toolbar.containerRole());
  EXPECT_GT(toolbar.getElevation(), 0);
  toolbar.setElevated(false);
  EXPECT_EQ(0, toolbar.getElevation());
  toolbar.setColorStyle(ToolbarColorStyle::kVibrant);
  EXPECT_EQ(ColorToken::kPrimaryContainer, toolbar.containerRole());
  toolbar.setOrientation(FloatingToolbarOrientation::kVertical);
  measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(64, measured.width());
  EXPECT_EQ(64, measured.height());
}

// Verifies a tall control grows the cross axis and constrained horizontal
// overflow starts at logical start while retaining the minimum item gap.
TEST(Material3Toolbar, FloatingTallChildAndBoundedOverflow) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  FloatingToolbar toolbar(context);
  Probe tall(context, 40, 80);
  Probe next(context, 40, 40);
  toolbar.add(tall);
  toolbar.add(next);
  Dimensions measured =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(116, measured.width());
  EXPECT_EQ(96, measured.height());
  measured = toolbar.measure(WidthSpec::Exactly(70), HeightSpec::Exactly(96));
  toolbar.layout(Rect(0, 0, 69, 95));
  EXPECT_EQ(12, tall.parent_bounds().xMin());
  EXPECT_TRUE(next.parent_bounds().empty());
  toolbar.clear();
}

// Verifies the toolbar delegates Tab traversal and physical-key activation to
// the existing task path, including cancellation when an armed child detaches.
TEST(Material3Toolbar, FocusActivationAndPendingDetach) {
  std::vector<roo::byte> raster(320 * 240 * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      320, 240, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  StaticLayout host(app.context());
  auto toolbar = std::make_unique<FloatingToolbar>(app.context());
  FloatingToolbar* toolbar_ptr = toolbar.get();
  int activations = 0;
  auto first = std::make_unique<Button>(app.context(), "Pen");
  Button* first_ptr = first.get();
  first->setOnInteractiveChange([&activations] { ++activations; });
  toolbar->add(std::move(first));
  auto second = std::make_unique<Button>(app.context(), "Erase");
  Button* second_ptr = second.get();
  second->setOnInteractiveChange([&activations] { ++activations; });
  toolbar->add(std::move(second));
  host.add(std::move(toolbar), 16, 16);
  Task& task = app.addTaskFullScreen(host);
  Keys keys;
  keys.connect(task);
  app.refresh();
  app.start();

  ASSERT_TRUE(task.focus().requestFocus(*first_ptr));
  keys.send(KeyPhase::kDown, KeyCode::kTab, PhysicalKey::kTab);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  app.refresh();
  EXPECT_EQ(second_ptr, task.focus().focused());
  keys.send(KeyPhase::kUp, KeyCode::kTab, PhysicalKey::kTab);
  keys.send(KeyPhase::kDown, KeyCode::kEnter, PhysicalKey::kEnter);
  keys.send(KeyPhase::kUp, KeyCode::kEnter, PhysicalKey::kEnter);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  app.refresh();
  delay(500);
  app.refresh();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  app.refresh();
  EXPECT_EQ(1, activations);

  keys.send(KeyPhase::kDown, KeyCode::kEnter, PhysicalKey::kEnter);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  toolbar_ptr->clear();
  keys.send(KeyPhase::kUp, KeyCode::kEnter, PhysicalKey::kEnter);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  app.refresh();
  EXPECT_EQ(1, activations);
  EXPECT_EQ(nullptr, task.focus().focused());
  keys.disconnect();
  task.navigation().clear();
}

// Verifies density changes remeasure eligible children while toolbar padding
// and the 64 dp cross-axis floor remain fixed.
TEST(Material3Toolbar, DensityChangesOnlyEligibleChildGeometry) {
  roo_scheduler::SchedulingService scheduler;
  Theme theme = DefaultTheme();
  Material3Theme& material = theme.material3Theme();

  Environment env(scheduler, theme);
  ApplicationContext context = MakeContext(env);
  FloatingToolbar toolbar(context);
  Button child(context, "Save", ButtonVariant::kFilledTonal);
  toolbar.add(child);
  const Dimensions regular =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  toolbar.layout(Rect(0, 0, regular.width() - 1, regular.height() - 1));
  const YDim regular_child_height = child.height();
  EXPECT_EQ(64, regular.height());
  EXPECT_EQ(8, toolbar.getPadding().top());

  material.density = Density::kMinus5;
  toolbar.requestLayoutDescending();
  toolbar.invalidateDescending();
  const Dimensions compact =
      toolbar.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  toolbar.layout(Rect(0, 0, compact.width() - 1, compact.height() - 1));
  EXPECT_EQ(64, compact.height());
  EXPECT_EQ(8, toolbar.getPadding().top());
  EXPECT_LT(child.height(), regular_child_height);
  toolbar.clear();
}

}  // namespace
}  // namespace roo_windows::material3
