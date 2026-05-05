#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/widgets/icon.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {
namespace {

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(const Environment& env) : Panel(env) {}

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

const roo::byte kDummyPictogramData[] = {roo::byte{0}};

roo_display::Pictogram MakePictogram(roo_display::Box extents,
                                     roo_display::Box anchor_extents) {
  return roo_display::Pictogram(extents, anchor_extents,
                                roo_display::ProgMemPtr(kDummyPictogramData),
                                roo_display::Alpha4(roo_display::color::Black));
}

}  // namespace

TEST(Icon, InkInsetsFollowDrawableBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  roo_display::Pictogram pictogram = MakePictogram(Box(-1, -2, 7, 9),
                                                   Box(0, 0, 5, 7));
  Icon icon(env, pictogram);

  EXPECT_EQ(Insets(-1, -2, -2, -2), icon.getInkInsets());
  EXPECT_EQ(6, icon.getSuggestedMinimumDimensions().width());
  EXPECT_EQ(8, icon.getSuggestedMinimumDimensions().height());
}

TEST(Icon, SwapInvalidatesOldAndNewVisualBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  roo_display::Pictogram old_icon = MakePictogram(Box(-2, 0, 4, 4),
                                                  Box(0, 0, 4, 4));
  roo_display::Pictogram new_icon = MakePictogram(Box(0, 0, 6, 4),
                                                  Box(0, 0, 4, 4));
  Icon icon(env, old_icon);
  RecordingPanel panel(env);

  panel.add(icon, Rect(10, 10, 14, 14));
  Rect old_bounds = icon.maxParentBounds();
  panel.invalidated_regions.clear();

  icon.setIcon(new_icon);

  ASSERT_EQ(1u, panel.invalidated_regions.size());
  EXPECT_EQ(Rect::Extent(old_bounds, icon.maxParentBounds()),
            panel.invalidated_regions.front());
}

}  // namespace roo_windows