#include "roo_windows/containers/scroll_motion_controller.h"

#include <algorithm>
#include <cmath>

#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {
namespace scroll_motion {

namespace {

constexpr float kDeceleration = 300.0f;
constexpr float kMaxVelocity = 5000.0f;
constexpr int16_t kMaxOvershootPx = Scaled(40);
constexpr unsigned long kSpringBackDurationMs = 500;

template <typename T>
T Clamp(T input, T min_val, T max_val) {
  if (input < min_val) return min_val;
  if (input > max_val) return max_val;
  return input;
}

Result MakeResult(const Geometry& geometry, XDim x, YDim y, bool changed,
                  bool needs_tick) {
  return {x, y, changed, needs_tick, geometry.isInOvershoot(x, y)};
}

// Returns a damped signed overshoot that asymptotically approaches the maximum
// visual bounce distance.
int16_t DampedOvershoot(int excess) {
  if (excess == 0) return 0;
  int abs_excess = excess < 0 ? -excess : excess;
  int16_t result = static_cast<int16_t>(
      std::round(static_cast<float>(kMaxOvershootPx) * abs_excess /
                 (abs_excess + kMaxOvershootPx)));
  return excess < 0 ? -result : result;
}

}  // namespace

bool Geometry::horizontalEnabled() const {
  return axis_ == Axis::kHorizontal || axis_ == Axis::kBoth;
}

bool Geometry::verticalEnabled() const {
  return axis_ == Axis::kVertical || axis_ == Axis::kBoth;
}

bool Geometry::horizontalScrollable() const {
  return horizontalEnabled() && min_x_ < max_x_;
}

bool Geometry::verticalScrollable() const {
  return verticalEnabled() && min_y_ < max_y_;
}

bool Geometry::canScroll() const {
  return horizontalScrollable() || verticalScrollable();
}

bool Geometry::isInOvershoot(XDim x, YDim y) const {
  return x != clampX(x) || y != clampY(y);
}

bool Geometry::shouldClaimDrag(XDim total_dx, YDim total_dy) const {
  int32_t dist_sq = total_dx * total_dx + total_dy * total_dy;
  if (dist_sq <= kTouchSlopSquare) return false;
  switch (axis_) {
    case Axis::kHorizontal:
      return horizontalScrollable() && std::abs(total_dx) > std::abs(total_dy);
    case Axis::kVertical:
      return verticalScrollable() && std::abs(total_dy) > std::abs(total_dx);
    case Axis::kBoth:
      return canScroll();
  }
  return false;
}

XDim Geometry::clampX(XDim x) const {
  return horizontalScrollable() ? Clamp(x, min_x_, max_x_)
                                : static_cast<XDim>(0);
}

YDim Geometry::clampY(YDim y) const {
  return verticalScrollable() ? Clamp(y, min_y_, max_y_) : static_cast<YDim>(0);
}

bool State::isAnimating() const {
  return phase_ == Phase::kFlinging || phase_ == Phase::kSpringBack;
}

Result State::scrollToRaw(const Geometry& geometry, XDim current_x,
                          YDim current_y, XDim x, YDim y) const {
  if (!geometry.horizontalEnabled()) x = 0;
  if (!geometry.verticalEnabled()) y = 0;
  return MakeResult(geometry, x, y, x != current_x || y != current_y,
                    isAnimating());
}

Result State::scrollToDraggedRaw(const Geometry& geometry, XDim current_x,
                                 YDim current_y, XDim raw_x, YDim raw_y) const {
  XDim target_x = current_x;
  if (geometry.horizontalScrollable()) {
    XDim clamped_x = geometry.clampX(raw_x);
    target_x = clamped_x + DampedOvershoot(raw_x - clamped_x);
  } else {
    target_x = 0;
  }

  YDim target_y = current_y;
  if (geometry.verticalScrollable()) {
    YDim clamped_y = geometry.clampY(raw_y);
    target_y = clamped_y + DampedOvershoot(raw_y - clamped_y);
  } else {
    target_y = 0;
  }
  return scrollToRaw(geometry, current_x, current_y, target_x, target_y);
}

Result State::startSpringBack(const Geometry& geometry, XDim current_x,
                              YDim current_y, unsigned long now_ms) {
  if (!geometry.isInOvershoot(current_x, current_y)) {
    return MakeResult(geometry, current_x, current_y, false, false);
  }
  XDim target_x = geometry.clampX(current_x);
  YDim target_y = geometry.clampY(current_y);
  phase_ = Phase::kSpringBack;
  anim_.springback.start_time_ms = now_ms;
  anim_.springback.start_ox = current_x - target_x;
  anim_.springback.start_oy = current_y - target_y;
  anim_.springback.target_x = target_x;
  anim_.springback.target_y = target_y;
  return MakeResult(geometry, current_x, current_y, false, true);
}

Result State::scrollTo(const Geometry& geometry, XDim current_x, YDim current_y,
                       XDim target_x, YDim target_y) {
  phase_ = Phase::kIdle;
  return scrollToRaw(geometry, current_x, current_y, geometry.clampX(target_x),
                     geometry.clampY(target_y));
}

Result State::onDown(const Geometry& geometry, XDim current_x, YDim current_y) {
  XDim x = geometry.clampX(current_x);
  YDim y = geometry.clampY(current_y);
  phase_ = Phase::kDragging;
  anim_.drag.raw_x = x;
  anim_.drag.raw_y = y;
  return MakeResult(geometry, x, y, x != current_x || y != current_y, false);
}

Result State::onDrag(const Geometry& geometry, XDim current_x, YDim current_y,
                     XDim dx, YDim dy) {
  if (!geometry.canScroll()) {
    return MakeResult(geometry, current_x, current_y, false, false);
  }
  if (phase_ != Phase::kDragging) {
    phase_ = Phase::kDragging;
    anim_.drag.raw_x = current_x;
    anim_.drag.raw_y = current_y;
  }
  if (geometry.horizontalEnabled()) anim_.drag.raw_x += dx;
  if (geometry.verticalEnabled()) anim_.drag.raw_y += dy;

  Result result = scrollToDraggedRaw(geometry, current_x, current_y,
                                     anim_.drag.raw_x, anim_.drag.raw_y);
  if (!geometry.horizontalScrollable()) anim_.drag.raw_x = 0;
  if (!geometry.verticalScrollable()) anim_.drag.raw_y = 0;
  return result;
}

Result State::onFling(const Geometry& geometry, XDim current_x, YDim current_y,
                      XDim vx, YDim vy, unsigned long now_ms) {
  if (!geometry.canScroll()) {
    return MakeResult(geometry, current_x, current_y, false, false);
  }
  if (geometry.isInOvershoot(current_x, current_y)) {
    return startSpringBack(geometry, current_x, current_y, now_ms);
  }

  phase_ = Phase::kFlinging;
  anim_.fling.start_time_ms = now_ms;
  anim_.fling.start_vx = geometry.horizontalEnabled() ? vx : 0;
  anim_.fling.start_vy = geometry.verticalEnabled() ? vy : 0;
  anim_.fling.x_start = current_x;
  anim_.fling.y_start = current_y;

  if (anim_.fling.start_vx < 0 && current_x == geometry.maxX()) {
    anim_.fling.start_vx = 0;
  }
  if (anim_.fling.start_vx > 0 && current_x == geometry.minX()) {
    anim_.fling.start_vx = 0;
  }
  if (anim_.fling.start_vy < 0 && current_y == geometry.maxY()) {
    anim_.fling.start_vy = 0;
  }
  if (anim_.fling.start_vy > 0 && current_y == geometry.minY()) {
    anim_.fling.start_vy = 0;
  }

  if (anim_.fling.start_vx == 0 && anim_.fling.start_vy == 0) {
    phase_ = Phase::kIdle;
    return MakeResult(geometry, current_x, current_y, false, false);
  }

  float v_abs = std::sqrt(anim_.fling.start_vx * anim_.fling.start_vx +
                          anim_.fling.start_vy * anim_.fling.start_vy);
  if (v_abs > kMaxVelocity) {
    float mult = kMaxVelocity / v_abs;
    anim_.fling.start_vx *= mult;
    anim_.fling.start_vy *= mult;
    v_abs = kMaxVelocity;
  }

  unsigned long duration_ms =
      static_cast<unsigned long>(1000.0f * v_abs / kDeceleration);
  anim_.fling.end_time_ms = anim_.fling.start_time_ms + duration_ms;
  anim_.fling.decel_x = -kDeceleration * anim_.fling.start_vx / v_abs;
  anim_.fling.decel_y = -kDeceleration * anim_.fling.start_vy / v_abs;

  int16_t kick_dx = static_cast<int16_t>(anim_.fling.start_vx * 0.02f);
  int16_t kick_dy = static_cast<int16_t>(anim_.fling.start_vy * 0.02f);
  Result result = scrollToDraggedRaw(geometry, current_x, current_y,
                                     current_x + kick_dx, current_y + kick_dy);
  result.needs_tick = true;
  return result;
}

Result State::onTouchUp(const Geometry& geometry, XDim current_x,
                        YDim current_y, unsigned long now_ms) {
  if (phase_ != Phase::kFlinging) {
    phase_ = Phase::kIdle;
    return startSpringBack(geometry, current_x, current_y, now_ms);
  }
  return MakeResult(geometry, current_x, current_y, false, true);
}

Result State::tick(const Geometry& geometry, XDim current_x, YDim current_y,
                   unsigned long now_ms) {
  if (phase_ == Phase::kSpringBack) {
    float t =
        static_cast<float>(now_ms - anim_.springback.start_time_ms) / 1000.0f;
    const float duration = kSpringBackDurationMs / 1000.0f;
    if (t >= duration) {
      phase_ = Phase::kIdle;
      return scrollTo(geometry, current_x, current_y, anim_.springback.target_x,
                      anim_.springback.target_y);
    }
    float frac = 1.0f - t / duration;
    float ease = frac * frac;
    XDim ox = static_cast<XDim>(std::round(anim_.springback.start_ox * ease));
    YDim oy = static_cast<YDim>(std::round(anim_.springback.start_oy * ease));
    Result result = scrollToRaw(geometry, current_x, current_y,
                                anim_.springback.target_x + ox,
                                anim_.springback.target_y + oy);
    result.needs_tick = true;
    return result;
  }

  if (phase_ != Phase::kFlinging) {
    return MakeResult(geometry, current_x, current_y, false, false);
  }

  bool scroll_in_progress = true;
  unsigned long t_end = now_ms;
  if (static_cast<long>(t_end - anim_.fling.end_time_ms) >= 0) {
    t_end = anim_.fling.end_time_ms;
    scroll_in_progress = false;
  }

  float t = (t_end - anim_.fling.start_time_ms) / 1000.0f;
  XDim target_x = current_x;
  YDim target_y = current_y;
  if (anim_.fling.start_vx != 0) {
    int32_t total_x = static_cast<int32_t>(anim_.fling.start_vx * t +
                                           anim_.fling.decel_x * t * t / 2);
    target_x = anim_.fling.x_start + total_x;
    if (target_x < geometry.minX() - kMaxOvershootPx) {
      target_x = geometry.minX() - kMaxOvershootPx;
      anim_.fling.start_vx = 0;
    }
    if (target_x > geometry.maxX() + kMaxOvershootPx) {
      target_x = geometry.maxX() + kMaxOvershootPx;
      anim_.fling.start_vx = 0;
    }
  }
  if (anim_.fling.start_vy != 0) {
    int32_t total_y = static_cast<int32_t>(anim_.fling.start_vy * t +
                                           anim_.fling.decel_y * t * t / 2);
    target_y = anim_.fling.y_start + total_y;
    if (target_y < geometry.minY() - kMaxOvershootPx) {
      target_y = geometry.minY() - kMaxOvershootPx;
      anim_.fling.start_vy = 0;
    }
    if (target_y > geometry.maxY() + kMaxOvershootPx) {
      target_y = geometry.maxY() + kMaxOvershootPx;
      anim_.fling.start_vy = 0;
    }
  }

  Result result =
      scrollToRaw(geometry, current_x, current_y, target_x, target_y);
  if (anim_.fling.start_vx == 0 && anim_.fling.start_vy == 0) {
    scroll_in_progress = false;
  }
  if (scroll_in_progress) {
    result.needs_tick = true;
    return result;
  }
  phase_ = Phase::kIdle;
  Result spring = startSpringBack(geometry, result.x, result.y, now_ms);
  spring.changed = result.changed || spring.changed;
  return spring;
}

}  // namespace scroll_motion
}  // namespace roo_windows
