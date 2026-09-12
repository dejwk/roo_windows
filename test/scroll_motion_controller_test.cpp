#include "roo_windows/containers/scroll_motion_controller.h"

#include <cstdint>

#include "gtest/gtest.h"

namespace roo_windows {
namespace scroll_motion {
namespace {

// Verifies that the immediate fling kick does not overwrite the active fling
// parameters stored in the motion-state union.
TEST(ScrollMotionController, FlingKickDoesNotCorruptAnimationState) {
  State state;
  Geometry geometry{-500, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = UINT32_MAX - 5;

  Result kicked = state.onFling(geometry, -100, 0, -1200, 0, kEpoch);

  EXPECT_TRUE(kicked.changed);
  EXPECT_TRUE(kicked.needs_tick);
  EXPECT_FALSE(kicked.in_overshoot);
  EXPECT_LT(kicked.x, -100);

  Result ticked = state.tick(geometry, kicked.x, kicked.y, kEpoch + 10);

  EXPECT_TRUE(ticked.changed);
  EXPECT_TRUE(ticked.needs_tick);
  EXPECT_FALSE(ticked.in_overshoot);
  EXPECT_LT(ticked.x, -100);
  EXPECT_GT(ticked.x, -200);
}

// Verifies fling completion is based on elapsed duration rather than ordering
// the wrapped current timestamp against an absolute end timestamp.
TEST(ScrollMotionController, FlingCompletesAcrossTimestampWrap) {
  State state;
  Geometry geometry{-5000, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = UINT32_MAX - 500;

  Result kicked = state.onFling(geometry, -1000, 0, -300, 0, kEpoch);
  ASSERT_TRUE(kicked.needs_tick);

  Result halfway = state.tick(geometry, kicked.x, kicked.y, kEpoch + 500);
  EXPECT_TRUE(halfway.needs_tick);
  EXPECT_TRUE(state.isAnimating());

  Result finished = state.tick(geometry, halfway.x, halfway.y, kEpoch + 1000);
  EXPECT_FALSE(finished.needs_tick);
  EXPECT_FALSE(state.isAnimating());
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

// Verifies programmatic motion keeps its cubic trajectory while its compact
// timestamp wraps from UINT32_MAX back to zero.
TEST(ScrollMotionController, ProgrammaticTrajectoryCrossesTimestampWrap) {
  State state;
  Geometry geometry{-1000, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = UINT32_MAX - 100;

  Result started = state.animateTo(geometry, 0, 0, -100, 0, kEpoch);
  EXPECT_FALSE(started.changed);
  EXPECT_TRUE(started.needs_tick);

  Result halfway = state.tick(geometry, started.x, started.y, kEpoch + 125);
  EXPECT_EQ(-88, halfway.x);
  EXPECT_TRUE(halfway.needs_tick);

  Result finished = state.tick(geometry, halfway.x, halfway.y, kEpoch + 250);
  EXPECT_EQ(-100, finished.x);
  EXPECT_FALSE(finished.needs_tick);
  EXPECT_FALSE(state.isAnimating());
}

// Verifies a nearby clock value before the motion epoch is interpreted through
// the half-range rule rather than as a huge forward elapsed duration.
TEST(ScrollMotionController, NearbyBackwardClockIsTreatedAsZeroElapsed) {
  State state;
  Geometry geometry{-1000, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = 1000;
  state.animateTo(geometry, 0, 0, -100, 0, kEpoch);

  Result result = state.tick(geometry, 0, 0, kEpoch - 1);

  EXPECT_EQ(0, result.x);
  EXPECT_FALSE(result.changed);
  EXPECT_TRUE(result.needs_tick);
  EXPECT_TRUE(state.isAnimating());
}

// Verifies spring-back retains its quadratic phase across timestamp wrap and
// reaches the clamped endpoint using the same caller-supplied epoch.
TEST(ScrollMotionController, SpringBackTrajectoryCrossesTimestampWrap) {
  State state;
  Geometry geometry{-500, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = UINT32_MAX - 200;
  state.onDown(geometry, 0, 0);
  Result dragged = state.onDrag(geometry, 0, 0, 100, 0);
  ASSERT_GT(dragged.x, 0);

  Result released =
      state.onTouchUp(geometry, dragged.x, dragged.y, kEpoch);
  ASSERT_TRUE(released.needs_tick);
  Result halfway =
      state.tick(geometry, released.x, released.y, kEpoch + 250);
  EXPECT_GT(halfway.x, 0);
  EXPECT_LT(halfway.x, released.x);
  EXPECT_TRUE(halfway.needs_tick);

  Result finished = state.tick(geometry, halfway.x, halfway.y, kEpoch + 500);
  EXPECT_EQ(0, finished.x);
  EXPECT_FALSE(finished.needs_tick);
  EXPECT_FALSE(state.isAnimating());
}

// Records the compact clock width and host-side state bound.
TEST(ScrollMotionController, CompactClockAndStateSizeAreExplicit) {
  EXPECT_EQ(4U, sizeof(TimestampMillis));
  EXPECT_EQ(4U, sizeof(DurationMillis));
  EXPECT_LE(sizeof(State), 40U);
}

}  // namespace
}  // namespace scroll_motion
}  // namespace roo_windows
