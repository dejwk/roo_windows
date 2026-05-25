#include "roo_windows/widgets/image.h"

#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/panel.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {
namespace {

ApplicationContext MakeContext(Environment& context) {
  return ApplicationContext(context.scheduler(), context.theme(),
                            context.keyboardColorTheme());
}

class TestDrawable : public roo_display::Drawable {
 public:
  TestDrawable(roo_display::Box extents, roo_display::Box anchor_extents)
      : extents_(extents), anchor_extents_(anchor_extents) {}

  roo_display::Box extents() const override { return extents_; }

  roo_display::Box anchorExtents() const override { return anchor_extents_; }

 private:
  roo_display::Box extents_;
  roo_display::Box anchor_extents_;
};

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(ApplicationContext& context) : Panel(context) {}

  using Panel::add;

  std::vector<Rect> invalidated_regions;

 protected:
  void childShown(const Widget* child) override { (void)child; }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    (void)child;
    (void)rect;
  }

  void childInvalidatedRegion(const Widget* child, Rect rect) override {
    (void)child;
    invalidated_regions.push_back(rect);
  }
};

}  // namespace

// Verifies that an Image's ink insets mirror how the drawable's natural
// extents overhang its anchor box, and that suggested minimum dimensions
// come from the anchor box.
TEST(Image, InkInsetsFollowDrawableBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TestDrawable drawable(Box(-2, 1, 6, 8), Box(0, 0, 4, 7));
  Image image(context, drawable);

  EXPECT_EQ(Insets(-2, 1, -2, -1), image.getInkInsets());
  EXPECT_EQ(5, image.getSuggestedMinimumDimensions().width());
  EXPECT_EQ(8, image.getSuggestedMinimumDimensions().height());

  image.layout(Rect(10, 20, 14, 27));
  EXPECT_EQ(Rect(-2, 1, 6, 8), image.getContentBounds());
  EXPECT_EQ(Rect(8, 21, 16, 28), image.getParentContentBounds());
}

// Verifies that swapping the wrapped drawable through setImage() emits a
// single invalidation region covering the union of the previous and new
// parent-space visual bounds, so partial repaints cover both directions of
// size change.
TEST(Image, SwapInvalidatesOldAndNewVisualBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TestDrawable old_drawable(Box(-2, 0, 4, 4), Box(0, 0, 4, 4));
  TestDrawable new_drawable(Box(0, 0, 6, 4), Box(0, 0, 4, 4));
  Image image(context, old_drawable);
  RecordingPanel panel(context);

  panel.add(image, Rect(10, 10, 14, 14));
  Rect old_bounds = image.maxParentBounds();
  panel.invalidated_regions.clear();

  image.setImage(&new_drawable);

  ASSERT_EQ(1u, panel.invalidated_regions.size());
  EXPECT_EQ(Rect::Extent(old_bounds, image.maxParentBounds()),
            panel.invalidated_regions.front());
}

}  // namespace roo_windows