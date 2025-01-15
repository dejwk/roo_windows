#pragma once

#include "roo_windows/core/preferred_size.h"

#include "roo_logging.h"

namespace roo_windows {

enum MeasureSpecKind {
  UNSPECIFIED = 0,
  AT_MOST = 1,
  EXACTLY = 2,
};

class WidthSpec {
 public:
  static constexpr WidthSpec Unspecified(XDim hint) {
    return WidthSpec(UNSPECIFIED | ((uint16_t)hint << 2));
  }

  static constexpr WidthSpec AtMost(XDim value) {
    return WidthSpec(AT_MOST | ((uint16_t)value << 2));
  }

  static constexpr WidthSpec Exactly(XDim value) {
    return WidthSpec(EXACTLY | ((uint16_t)value << 2));
  }

  MeasureSpecKind kind() const {
    return static_cast<MeasureSpecKind>(value_ & 0x3);
  }

  XDim value() const { return value_ >> 2; }

  WidthSpec getChildWidthSpec(int16_t padding,
                              PreferredSize::Width childWidth) {
    if (childWidth.isExact()) {
      return WidthSpec::Exactly(childWidth.value());
    }
    XDim size = std::max(0, value() - padding);
    switch (kind()) {
      // Parent has imposed an exact size on us
      case EXACTLY: {
        if (childWidth.isMatchParent()) {
          // Child wants to be our size. So be it.
          return WidthSpec::Exactly(size);
        } else {
          // CHECK(childWidth.isWrapContent());
          // Child wants to determine its own size. It can't be
          // bigger than us.
          return WidthSpec::AtMost(size);
        }
        break;
      }
      // Parent has imposed a maximum size on us
      case AT_MOST: {
        return WidthSpec::AtMost(size);
      }
      // Parent asked to see how big we want to be
      case UNSPECIFIED:
      default: {
        // Child wants to be our size... find out how big it should
        // be
        return WidthSpec::Unspecified(size);
      }
    }
  }

  XDim resolveSize(XDim desired_size) {
    switch (kind()) {
      case UNSPECIFIED: {
        return desired_size;
      }
      case AT_MOST: {
        return std::min(desired_size, value());
      }
      case EXACTLY:
      default: {
        return value();
      }
    }
  }

 private:
  friend constexpr bool operator==(WidthSpec a, WidthSpec b);

  constexpr WidthSpec(uint16_t value) : value_(value) {}

  uint16_t value_;
};

inline constexpr bool operator==(WidthSpec a, WidthSpec b) {
  return a.value_ == b.value_;
}

roo_logging::Stream& operator<<(roo_logging::Stream& os, WidthSpec spec);

class HeightSpec {
 public:
  static constexpr HeightSpec Unspecified(YDim hint) {
    return HeightSpec(UNSPECIFIED | ((uint32_t)hint << 2));
  }

  static constexpr HeightSpec AtMost(YDim value) {
    return HeightSpec(AT_MOST | ((uint32_t)value << 2));
  }

  static constexpr HeightSpec Exactly(YDim value) {
    return HeightSpec(EXACTLY | ((uint32_t)value << 2));
  }

  MeasureSpecKind kind() const {
    return static_cast<MeasureSpecKind>(value_ & 0x3);
  }

  YDim value() const { return value_ >> 2; }

  HeightSpec getChildHeightSpec(int16_t padding,
                                PreferredSize::Height childHeight) {
    if (childHeight.isExact()) {
      return HeightSpec::Exactly(childHeight.value());
    }
    YDim size = std::max((YDim)0, value() - padding);
    switch (kind()) {
      // Parent has imposed an exact size on us
      case EXACTLY: {
        if (childHeight.isMatchParent()) {
          // Child wants to be our size. So be it.
          return HeightSpec::Exactly(size);
        } else {
          // CHECK(childHeight.isWrapContent());
          // Child wants to determine its own size. It can't be
          // bigger than us.
          return HeightSpec::AtMost(size);
        }
        break;
      }
      // Parent has imposed a maximum size on us
      case AT_MOST: {
        return HeightSpec::AtMost(size);
      }
      // Parent asked to see how big we want to be
      case UNSPECIFIED:
      default: {
        // Child wants to be our size... find out how big it should
        // be
        return HeightSpec::Unspecified(size);
      }
    }
  }

  YDim resolveSize(YDim desired_size) {
    switch (kind()) {
      case UNSPECIFIED: {
        return desired_size;
      }
      case AT_MOST: {
        return std::min(desired_size, value());
      }
      case EXACTLY:
      default: {
        return value();
      }
    }
  }

 private:
  friend constexpr bool operator==(HeightSpec a, HeightSpec b);

  constexpr HeightSpec(uint32_t value) : value_(value) {}

  uint32_t value_;
};

inline constexpr bool operator==(HeightSpec a, HeightSpec b) {
  return a.value_ == b.value_;
}

roo_logging::Stream& operator<<(roo_logging::Stream& os, HeightSpec spec);

}  // namespace roo_windows
