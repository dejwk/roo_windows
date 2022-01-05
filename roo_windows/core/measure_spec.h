#pragma once

#include "roo_windows/core/preferred_size.h"

#include "roo_glog/logging.h"

namespace roo_windows {

class MeasureSpec {
 public:
  enum Kind {
    UNSPECIFIED = 0,
    AT_MOST = 1,
    EXACTLY = 2,
  };

  static constexpr MeasureSpec Unspecified(int16_t hint) {
    return MeasureSpec(UNSPECIFIED | (hint << 2));
  }

  static constexpr MeasureSpec AtMost(int16_t value) {
    return MeasureSpec(AT_MOST | (value << 2));
  }

  static constexpr MeasureSpec Exactly(int16_t value) {
    return MeasureSpec(EXACTLY | (value << 2));
  }

  Kind kind() const { return static_cast<Kind>(value_ & 0x3); }

  int16_t value() const {
    return value_ >> 2;
  }

  MeasureSpec getChildMeasureSpec(int16_t padding,
                                  PreferredSize::Dimension childDimension) {
    if (childDimension.isExact()) {
      return MeasureSpec::Exactly(childDimension.value());
    }
    int16_t size = std::max(0, value() - padding);
    switch (kind()) {
      // Parent has imposed an exact size on us
      case MeasureSpec::EXACTLY: {
        if (childDimension.isMatchParent()) {
          // Child wants to be our size. So be it.
          return MeasureSpec::Exactly(size);
        } else {
          CHECK(childDimension.isWrapContent());
          // Child wants to determine its own size. It can't be
          // bigger than us.
          return MeasureSpec::AtMost(size);
        }
        break;
      }
      // Parent has imposed a maximum size on us
      case MeasureSpec::AT_MOST: {
        return MeasureSpec::AtMost(size);
      }
      // Parent asked to see how big we want to be
      case MeasureSpec::UNSPECIFIED: {
        // Child wants to be our size... find out how big it should
        // be
        return MeasureSpec::Unspecified(size);
      }
    }
  }

  int16_t resolveSize(int16_t desired_size) {
    switch (kind()) {
      case MeasureSpec::UNSPECIFIED: {
        return desired_size;
      }
      case MeasureSpec::AT_MOST: {
        return std::min(desired_size, value());
      }
      case MeasureSpec::EXACTLY: {
        return value();
      }
    }
  }

 private:
  friend constexpr bool operator==(MeasureSpec a, MeasureSpec b);

  constexpr MeasureSpec(int16_t value) : value_(value) {}

  int16_t value_;
};

inline constexpr bool operator==(MeasureSpec a, MeasureSpec b) {
  return a.value_ == b.value_;
}

}  // namespace roo_windows
