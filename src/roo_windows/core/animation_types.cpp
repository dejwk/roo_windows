#include "roo_windows/core/animation_types.h"

namespace roo_windows {

AnimationSpec AnimationSpec::Value(float from, float to,
                                   roo_time::Duration duration) {
  AnimationSpec spec;
  spec.duration = duration;
  spec.minimum_interval = roo_time::Millis(20);
  spec.from = from;
  spec.to = to;
  return spec;
}

AnimationSpec AnimationSpec::CustomTime() {
  AnimationSpec spec;
  spec.minimum_interval = roo_time::Millis(20);
  spec.legs = 0;
  spec.kind = AnimationKind::kCustomTime;
  return spec;
}

}  // namespace roo_windows
