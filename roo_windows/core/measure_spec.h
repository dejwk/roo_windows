#pragma once

#include "roo_glog/logging.h"
#include "roo_windows/core/preferred_size.h"

namespace roo_windows {

class MeasureSpec {
 public:
  enum Kind {
    UNSPECIFIED = 0,
    AT_MOST = 1,
    EXACTLY = 2,
  };

  static constexpr MeasureSpec Unspecified(int16_t hint) {
    return MeasureSpec(UNSPECIFIED | ((uint16_t)hint << 2));
  }

  static constexpr MeasureSpec AtMost(int16_t value) {
    return MeasureSpec(AT_MOST | ((uint16_t)value << 2));
  }

  static constexpr MeasureSpec Exactly(int16_t value) {
    return MeasureSpec(EXACTLY | ((uint16_t)value << 2));
  }

  Kind kind() const { return static_cast<Kind>(value_ & 0x3); }

  int16_t value() const { return value_ >> 2; }

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
      case MeasureSpec::EXACTLY:
      default: {
        return value();
      }
    }
  }

 private:
  friend constexpr bool operator==(MeasureSpec a, MeasureSpec b);

  constexpr MeasureSpec(uint16_t value) : value_(value) {}

  uint16_t value_;
};

inline constexpr bool operator==(MeasureSpec a, MeasureSpec b) {
  return a.value_ == b.value_;
}

}  // namespace roo_windows

#if defined(__linux__) || defined(__linux) || defined(linux)
#include <ostream>

namespace roo_windows {
inline std::ostream& operator<<(std::ostream& os,
                                const roo_windows::MeasureSpec& spec) {
  switch (spec.kind()) {
    case MeasureSpec::UNSPECIFIED: {
      os << "UNSPECIFIED";
      break;
    }
    case MeasureSpec::AT_MOST: {
      os << "AT_MOST";
      break;
    }
    case MeasureSpec::EXACTLY: {
      os << "EXACTLY";
      break;
    }
    default: {
      os << "UNKNOWN";
      break;
    }
  }
  os << "(" << spec.value() << ")";
  return os;
}
}  // namespace roo_windows
#endif  // defined(__linux__)
