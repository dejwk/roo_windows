#include "roo_windows/core/animation_evaluator.h"

#include <cmath>
#include <limits>

#include "gtest/gtest.h"
#include "roo_time.h"

namespace roo_windows::internal {
namespace {

TEST(AnimationSpecTest, ProvidesDocumentedDefaults) {
  AnimationSpec value = AnimationSpec::value(2.0f, 5.0f, roo_time::Millis(80));
  EXPECT_EQ(value.kind, AnimationKind::kValue);
  EXPECT_EQ(value.legs, 1u);
  EXPECT_EQ(value.minimum_interval, roo_time::Millis(20));
  EXPECT_EQ(value.playback, AnimationPlayback::kRestart);
  EXPECT_EQ(value.easing.kind, AnimationEasingKind::kLinear);

  AnimationSpec custom = AnimationSpec::customTime();
  EXPECT_EQ(custom.kind, AnimationKind::kCustomTime);
  EXPECT_EQ(custom.legs, 0u);
  EXPECT_EQ(custom.minimum_interval, roo_time::Millis(20));
  EXPECT_TRUE(isValidAnimationSpec(custom));
}

// Verifies invalid input and overflowing finite lengths are rejected before a
// registry can replace an existing channel.
TEST(AnimationSpecTest, RejectsInvalidFieldsAndDurationOverflow) {
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(10));
  EXPECT_TRUE(isValidAnimationSpec(spec));

  spec.from = std::numeric_limits<float>::infinity();
  EXPECT_FALSE(isValidAnimationSpec(spec));
  spec.from = 0.0f;
  spec.minimum_interval = roo_time::Micros(-1);
  EXPECT_FALSE(isValidAnimationSpec(spec));
  spec.minimum_interval = roo_time::Millis(20);
  spec.playback = static_cast<AnimationPlayback>(99);
  EXPECT_FALSE(isValidAnimationSpec(spec));
  spec.playback = AnimationPlayback::kRestart;
  spec.kind = static_cast<AnimationKind>(99);
  EXPECT_FALSE(isValidAnimationSpec(spec));

  spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Duration::Max());
  spec.delay = roo_time::Micros(1);
  EXPECT_FALSE(isValidAnimationSpec(spec));
  spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Duration::Max());
  spec.legs = 2;
  EXPECT_FALSE(isValidAnimationSpec(spec));
}

// Verifies zero-duration and indefinite specifications follow the explicit
// finite/infinite boundary in the design.
TEST(AnimationSpecTest, ValidatesFiniteAndInfiniteZeroDuration) {
  AnimationSpec finite = AnimationSpec::value(3.0f, 9.0f, roo_time::Duration());
  EXPECT_TRUE(isValidAnimationSpec(finite));
  AnimationSample terminal =
      evaluateAnimation(finite, roo_time::Duration(), roo_time::Duration());
  EXPECT_TRUE(terminal.terminal);
  EXPECT_FLOAT_EQ(terminal.value, 9.0f);

  finite.legs = 0;
  EXPECT_FALSE(isValidAnimationSpec(finite));

  AnimationSpec repeating =
      AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(10));
  repeating.legs = 0;
  EXPECT_TRUE(isValidAnimationSpec(repeating));
  EXPECT_EQ(animationEnd(repeating), roo_time::Duration::Max());
}

// Verifies exact boundaries start a new leg while a finite end delivers the
// last leg's exact endpoint.
TEST(AnimationEvaluatorTest, EvaluatesLegBoundariesAndTerminalEndpoints) {
  AnimationSpec spec =
      AnimationSpec::value(20.0f, 100.0f, roo_time::Millis(200));
  spec.legs = 2;
  spec.playback = AnimationPlayback::kReverse;

  AnimationSample quarter =
      evaluateAnimation(spec, roo_time::Millis(50), roo_time::Millis(50));
  EXPECT_EQ(quarter.leg, 0u);
  EXPECT_FLOAT_EQ(quarter.fraction, 0.25f);
  EXPECT_FLOAT_EQ(quarter.value, 40.0f);

  AnimationSample boundary =
      evaluateAnimation(spec, roo_time::Millis(200), roo_time::Millis(150));
  EXPECT_EQ(boundary.leg, 1u);
  EXPECT_TRUE(boundary.reverse);
  EXPECT_FLOAT_EQ(boundary.fraction, 1.0f);
  EXPECT_FLOAT_EQ(boundary.value, 100.0f);

  AnimationSample terminal =
      evaluateAnimation(spec, roo_time::Millis(400), roo_time::Millis(200));
  EXPECT_TRUE(terminal.terminal);
  EXPECT_EQ(terminal.leg, 1u);
  EXPECT_TRUE(terminal.reverse);
  EXPECT_FLOAT_EQ(terminal.fraction, 0.0f);
  EXPECT_FLOAT_EQ(terminal.value, 20.0f);
}

TEST(AnimationEvaluatorTest, AppliesDelayOnceAndEasingPerLeg) {
  AnimationSpec spec =
      AnimationSpec::value(20.0f, 100.0f, roo_time::Millis(200));
  spec.delay = roo_time::Millis(30);
  spec.easing.kind = AnimationEasingKind::kQuadraticOut;

  AnimationSample delayed =
      evaluateAnimation(spec, roo_time::Millis(20), roo_time::Duration());
  EXPECT_EQ(delayed.elapsed, roo_time::Duration());
  EXPECT_FLOAT_EQ(delayed.value, 20.0f);
  EXPECT_FALSE(delayed.terminal);

  AnimationSample quarter =
      evaluateAnimation(spec, roo_time::Millis(80), roo_time::Millis(60));
  EXPECT_EQ(quarter.elapsed, roo_time::Millis(50));
  EXPECT_FLOAT_EQ(quarter.fraction, 0.4375f);
  EXPECT_FLOAT_EQ(quarter.value, 55.0f);
}

TEST(AnimationEvaluatorTest, EvaluatesPresetCurvesAtExactEndpoints) {
  const AnimationEasingKind kinds[] = {
      AnimationEasingKind::kLinear, AnimationEasingKind::kQuadraticIn,
      AnimationEasingKind::kQuadraticOut, AnimationEasingKind::kSmoothstep,
      AnimationEasingKind::kCubicBezier};
  for (AnimationEasingKind kind : kinds) {
    AnimationEasing easing;
    easing.kind = kind;
    easing.x1 = 0.4f;
    easing.y1 = 0.0f;
    easing.x2 = 0.2f;
    easing.y2 = 1.0f;
    EXPECT_FLOAT_EQ(evaluateAnimationEasing(easing, 0.0f), 0.0f);
    EXPECT_FLOAT_EQ(evaluateAnimationEasing(easing, 1.0f), 1.0f);
  }
}

// Verifies the fixed-iteration Bezier solver stays within half a pixel at the
// largest audited 568-pixel extent.
TEST(AnimationEvaluatorTest, CubicBezierAccuracyFitsPixelBudget) {
  AnimationEasing easing{AnimationEasingKind::kCubicBezier, 0.4f, 0.0f, 0.2f,
                         1.0f};
  for (int i = 1; i < 100; ++i) {
    const float x = i / 100.0f;
    float lower = 0.0f;
    float upper = 1.0f;
    for (int iteration = 0; iteration < 30; ++iteration) {
      const float t = (lower + upper) * 0.5f;
      const float curve_x = ((1.0f + 3.0f * (easing.x1 - easing.x2)) * t +
                             3.0f * (easing.x2 - 2.0f * easing.x1)) *
                                t * t +
                            3.0f * easing.x1 * t;
      if (curve_x < x) {
        lower = t;
      } else {
        upper = t;
      }
    }
    const float t = (lower + upper) * 0.5f;
    const float expected = ((1.0f + 3.0f * (easing.y1 - easing.y2)) * t +
                            3.0f * (easing.y2 - 2.0f * easing.y1)) *
                               t * t +
                           3.0f * easing.y1 * t;
    EXPECT_LT(std::abs(evaluateAnimationEasing(easing, x) - expected) * 568.0f,
              0.5f);
  }
}

// Verifies signed 64-bit duration math does not inherit Arduino millisecond
// rollover when sampling after a month-long uptime.
TEST(AnimationEvaluatorTest, UsesLongUptimeDifferences) {
  const roo_time::Uptime anchor =
      roo_time::Uptime::Start() + roo_time::Hours(24 * 40);
  const roo_time::Uptime now = anchor + roo_time::Millis(75);
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(100));
  AnimationSample sample =
      evaluateAnimation(spec, now - anchor, roo_time::Duration());
  EXPECT_FLOAT_EQ(sample.fraction, 0.75f);
}

TEST(AnimationEvaluatorTest, CustomTimePassesElapsedAndDeltaOnly) {
  AnimationSample sample =
      evaluateAnimation(AnimationSpec::customTime(), roo_time::Hours(24 * 40),
                        roo_time::Millis(33));
  EXPECT_EQ(sample.elapsed, roo_time::Hours(24 * 40));
  EXPECT_EQ(sample.delta, roo_time::Millis(33));
  EXPECT_FLOAT_EQ(sample.fraction, 0.0f);
  EXPECT_FLOAT_EQ(sample.value, 0.0f);
  EXPECT_FALSE(sample.terminal);
}

}  // namespace
}  // namespace roo_windows::internal
