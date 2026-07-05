#include "roo_windows/containers/scroll_motion_controller.h"

#include "gtest/gtest.h"

namespace roo_windows {
namespace scroll_motion {
namespace {

// Verifies that the immediate fling kick does not overwrite the active fling
// parameters stored in the motion-state union.
TEST(ScrollMotionController, FlingKickDoesNotCorruptAnimationState) {
  State state;
  Geometry geometry{-500, 0, 0, 0, Axis::kHorizontal};

  Result kicked = state.onFling(geometry, -100, 0, -1200, 0, 1000);

  EXPECT_TRUE(kicked.changed);
  EXPECT_TRUE(kicked.needs_tick);
  EXPECT_FALSE(kicked.in_overshoot);
  EXPECT_LT(kicked.x, -100);

  Result ticked = state.tick(geometry, kicked.x, kicked.y, 1010);

  EXPECT_TRUE(ticked.changed);
  EXPECT_TRUE(ticked.needs_tick);
  EXPECT_FALSE(ticked.in_overshoot);
  EXPECT_LT(ticked.x, -100);
  EXPECT_GT(ticked.x, -200);
}

// Verifies that a fling directed out of an already reached boundary is ignored
// instead of scheduling a no-op animation.
TEST(ScrollMotionController, FlingIntoBlockedEdgeDoesNotAnimate) {
  State state;
  Geometry geometry{-500, 0, 0, 0, Axis::kHorizontal};

  Result result = state.onFling(geometry, -500, 0, 1200, 0, 1000);

  EXPECT_FALSE(result.changed);
  EXPECT_FALSE(result.needs_tick);
  EXPECT_FALSE(state.isAnimating());
  EXPECT_EQ(-500, result.x);
  EXPECT_EQ(0, result.y);
}

}  // namespace
}  // namespace scroll_motion
}  // namespace roo_windows
