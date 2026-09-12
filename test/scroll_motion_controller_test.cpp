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

// Verifies programmatic motion keeps its cubic trajectory when the shared
// epoch is well beyond a 32-bit millisecond counter.
TEST(ScrollMotionController, ProgrammaticTrajectorySupportsLongUptime) {
  State state;
  Geometry geometry{-1000, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = INT64_C(7776000000);  // 90 days.

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

// Verifies a clock value earlier than the motion epoch is treated as zero
// elapsed time instead of wrapping into a completed transition.
TEST(ScrollMotionController, BackwardClockDoesNotWrapElapsedTime) {
  State state;
  Geometry geometry{-1000, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = INT64_C(7776000000);
  state.animateTo(geometry, 0, 0, -100, 0, kEpoch);

  Result result = state.tick(geometry, 0, 0, kEpoch - 1);

  EXPECT_EQ(0, result.x);
  EXPECT_FALSE(result.changed);
  EXPECT_TRUE(result.needs_tick);
  EXPECT_TRUE(state.isAnimating());
}

// Verifies spring-back retains its quadratic phase and reaches the clamped
// endpoint using the same caller-supplied epoch.
TEST(ScrollMotionController, SpringBackTrajectoryUsesSharedEpoch) {
  State state;
  Geometry geometry{-500, 0, 0, 0, Axis::kHorizontal};
  constexpr TimestampMillis kEpoch = INT64_C(7776000000);
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

// Records the explicit clock width and the host-side state bound after the
// timestamp migration. ESP32 grows from roughly 32 to 48 bytes because its
// former unsigned long fields were 32-bit; the host was already 64-bit.
TEST(ScrollMotionController, SignedClockAndStateSizeAreExplicit) {
  EXPECT_EQ(8U, sizeof(TimestampMillis));
  EXPECT_LE(sizeof(State), 48U);
}

}  // namespace
}  // namespace scroll_motion
}  // namespace roo_windows
