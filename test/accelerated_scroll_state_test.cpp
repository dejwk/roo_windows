#include "roo_windows/containers/accelerated_scroll_state.h"

#include "gtest/gtest.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using namespace roo_display;
using namespace test_support;

class StateContent : public SurfaceWidget {
 public:
  explicit StateContent(ApplicationContext& ctx) : SurfaceWidget(ctx) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return {96, 400};
  }

  void paint(PaintContext& ctx) const override { ctx.clear(); }
};

class StateHarness : public internal::AcceleratedScrollState {
 public:
  using AcceleratedScrollState::AcceleratedScrollState;
  using AcceleratedScrollState::beginPaint;
  using AcceleratedScrollState::PaintMode;
  using AcceleratedScrollState::viewport;

  PaintMode last_mode = PaintMode::kUnavailable;

 protected:
  void paintWidgetContents(PaintContext& ctx) override {
    last_mode = beginPaint(ctx);
    SimpleScrollablePanel::paintWidgetContents(ctx);
  }
};

class StateParent : public Panel {
 public:
  using Panel::add;
  using Panel::moveTo;
  using Panel::Panel;
};

class AcceleratedScrollStateTest : public RooWindowsRenderTestSized<96, 72> {
 protected:
  void install() {
    auto child = std::make_unique<StateContent>(context());
    content_ = child.get();
    auto panel = std::make_unique<StateHarness>(context(), std::move(child));
    panel_ = panel.get();
    app_.add(std::move(panel), display_.extents());
    app_.window().setAdvisoryPaintBudget(roo_time::Millis(1));
    refresh();
    ASSERT_EQ(StateHarness::PaintMode::kComplete, panel_->last_mode);
  }

  void moveAndExpect(StateHarness::PaintMode mode) {
    panel_->scrollBy(0, -8);
    refresh();
    EXPECT_EQ(mode, panel_->last_mode);
  }

  StateHarness* panel_ = nullptr;
  StateContent* content_ = nullptr;
};

// Verifies scroll-generated damage can accelerate, but simultaneous content,
// callback-driven mutations and external reveal require complete output.
TEST_F(AcceleratedScrollStateTest, DistinguishesMotionFromForeignDamage) {
  install();
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  content_->setDirty();
  moveAndExpect(StateHarness::PaintMode::kComplete);
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  panel_->setOnScrollPositionChanged(
      [&](ScrollPosition, ScrollPosition) { content_->setDirty(); });
  moveAndExpect(StateHarness::PaintMode::kComplete);
  panel_->setOnScrollPositionChanged(nullptr);
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  app_.root().invalidateInterior(Rect(0, 0, 95, 71));
  moveAndExpect(StateHarness::PaintMode::kComplete);
}

// Verifies replacement, resize, hiding and reattachment discard the baseline.
TEST_F(AcceleratedScrollStateTest, LifecycleChangesRequireNewCompleteBaseline) {
  install();
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  auto replacement = std::make_unique<StateContent>(context());
  content_ = replacement.get();
  panel_->setContents(std::move(replacement));
  refresh();
  EXPECT_EQ(StateHarness::PaintMode::kComplete, panel_->last_mode);
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  panel_->layout(Rect(0, 0, 79, 63));
  moveAndExpect(StateHarness::PaintMode::kComplete);
  panel_->setVisibility(Visibility::kInvisible);
  refresh();
  panel_->setVisibility(Visibility::kVisible);
  moveAndExpect(StateHarness::PaintMode::kComplete);
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
}

// Verifies a borrowed scroller cannot carry its admitted baseline across an
// actual detach and reattach, including delivery of presentation callbacks.
TEST_F(AcceleratedScrollStateTest, DetachAndReattachResetAdmission) {
  StateHarness panel(context(), std::make_unique<StateContent>(context()));
  panel_ = &panel;
  app_.add(panel, display_.extents());
  app_.window().setAdvisoryPaintBudget(roo_time::Millis(1));
  refresh();
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  app_.root().removeTask(panel);
  refresh();
  app_.add(panel, display_.extents());
  moveAndExpect(StateHarness::PaintMode::kComplete);
  app_.root().removeTask(panel);
}

// Verifies a narrow damage clip and pre-existing foreign output cannot
// establish a reusable baseline, even if sampled movement continues.
TEST_F(AcceleratedScrollStateTest, PartialAndObscuredPaintsRejectAdmission) {
  install();
  panel_->scrollBy(0, -8);
  Surface surface(offscreen_, 0, 0, display_.extents(), false,
                  panel_->effectiveBackground(), FillMode::kVisible,
                  BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState storage;
  Clipper clipper(storage, offscreen_, roo_time::Uptime::Now());
  PaintContext ctx(canvas, clipper);
  PaintContext partial = ctx.clipped(Rect(0, 0, 95, 15));
  EXPECT_EQ(StateHarness::PaintMode::kPartial, panel_->beginPaint(partial));
  moveAndExpect(StateHarness::PaintMode::kComplete);
  clipper.addExclusion(Box(10, 10, 20, 20));
  EXPECT_EQ(StateHarness::PaintMode::kUnavailable, panel_->beginPaint(ctx));
  moveAndExpect(StateHarness::PaintMode::kComplete);
  PaintContext empty = ctx.clipped(Rect(200, 200, 210, 210));
  EXPECT_NE(StateHarness::PaintMode::kAccelerated, panel_->beginPaint(empty));
}

// Verifies ancestor movement and clipping geometry affect admission
// independently of the damage rectangle; an unclipped direct child keeps the
// ancestor's clips.
TEST_F(AcceleratedScrollStateTest,
       AncestorGeometryAndChildClipModeAreObserved) {
  auto parent = std::make_unique<StateParent>(context());
  StateParent* ancestor = parent.get();
  auto panel = std::make_unique<StateHarness>(
      context(), std::make_unique<StateContent>(context()));
  panel_ = panel.get();
  ancestor->add(std::move(panel), Rect(0, 0, 95, 71));
  app_.add(std::move(parent), Box(10, 5, 79, 64));
  app_.window().setAdvisoryPaintBudget(roo_time::Millis(1));
  refresh();
  EXPECT_EQ(Box(10, 5, 79, 64), panel_->viewport());
  moveAndExpect(StateHarness::PaintMode::kAccelerated);
  ancestor->moveTo(Rect(15, 10, 84, 69));
  moveAndExpect(StateHarness::PaintMode::kComplete);
  panel_->setParentClipMode(ParentClipMode::kUnclipped);
  moveAndExpect(StateHarness::PaintMode::kComplete);
  EXPECT_EQ(Box(15, 10, 95, 71), panel_->viewport());
}

}  // namespace
}  // namespace roo_windows
