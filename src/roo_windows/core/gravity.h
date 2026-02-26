#pragma once

#include <inttypes.h>

#include "roo_display/ui/alignment.h"

namespace roo_windows {

static const uint8_t kGravityAxisSpecified = 1;
static const uint8_t kGravityAxisLeft = 2;
static const uint8_t kGravityAxisRight = 4;
static const uint8_t kGravityAxisClip = 8;

enum GravityAxisAlignment {
  NONE = 0x0,
  MIDDLE = 0x1,
  MIN = 0x3,
  MAX = 0x5,
  //   FILL = 0x7,
  //   CLIPPED_MIDDLE = 0x9,
  //   CLIPPED_MIN = 0xB,
  //   CLIPPED_MAX = 0xD,
  //   CLIPPED_FILL = 0xF,
};

class GravityAxis {
 public:
  constexpr GravityAxis() : GravityAxis(NONE) {}

  constexpr GravityAxis(GravityAxisAlignment alignment)
      : alignment_(alignment) {}

  constexpr bool isSet() const {
    return (alignment_ & kGravityAxisSpecified) != 0;
  }

  constexpr bool isAxisMiddle() const {
    return (alignment_ & ~kGravityAxisClip) == MIDDLE;
  }

  constexpr bool isAxisMin() const {
    return (alignment_ & ~kGravityAxisClip) == MIN;
  }

  constexpr bool isAxisMax() const {
    return (alignment_ & ~kGravityAxisClip) == MAX;
  }

  //   constexpr bool isFill() const { return (align_ & ~kGravityAxisClip) ==
  //   FILL; }

  //   constexpr bool isClipped() const { return (align_ & kGravityAxisClip) !=
  //   0; }

 private:
  friend class Gravity;

  GravityAxisAlignment alignment_;
};

class HorizontalGravity : public GravityAxis {
 public:
  constexpr HorizontalGravity() : GravityAxis(NONE) {}

  constexpr HorizontalGravity(GravityAxisAlignment alignment)
      : GravityAxis(alignment) {}

  constexpr bool isCenter() const { return isAxisMiddle(); }

  constexpr bool isLeft() const { return isAxisMin(); }

  constexpr bool isRight() const { return isAxisMax(); }

  constexpr roo_display::HAlign asAlignment() const {
    return isLeft()     ? roo_display::kLeft
           : isCenter() ? roo_display::kCenter
           : isRight()  ? roo_display::kRight
                        : roo_display::kOrigin;
  }

 private:
  friend class Gravity;
};

class VerticalGravity : public GravityAxis {
 public:
  constexpr VerticalGravity() : GravityAxis(NONE) {}

  constexpr VerticalGravity(GravityAxisAlignment alignment)
      : GravityAxis(alignment) {}

  constexpr bool isMiddle() const { return isAxisMiddle(); }

  constexpr bool isTop() const { return isAxisMin(); }

  constexpr bool isBottom() const { return isAxisMax(); }

  constexpr roo_display::VAlign asAlignment() const {
    return isTop()      ? roo_display::kTop
           : isMiddle() ? roo_display::kMiddle
           : isBottom() ? roo_display::kBottom
                        : roo_display::kBaseline;
  }

 private:
  friend class Gravity;
};

class Gravity {
 public:
  constexpr Gravity() : value_(0) {}

  constexpr Gravity(HorizontalGravity x, VerticalGravity y)
      : value_(x.alignment_ | (y.alignment_ << 4)) {}

  constexpr Gravity(HorizontalGravity x) : Gravity(x, VerticalGravity()) {}

  constexpr Gravity(VerticalGravity y) : Gravity(HorizontalGravity(), y) {}

  constexpr HorizontalGravity x() const {
    return HorizontalGravity((GravityAxisAlignment)(value_ & 0xF));
  }
  constexpr VerticalGravity y() const {
    return VerticalGravity((GravityAxisAlignment)(value_ >> 4));
  }

  roo_display::Alignment asAlignment() const {
    return x().asAlignment() | y().asAlignment();
  }

 private:
  uint8_t value_;
};

/// Convenience operators to combine horizontal and vertical gravity via `|`.
inline constexpr Gravity operator|(HorizontalGravity x, VerticalGravity y) {
  return Gravity(x, y);
}

inline constexpr Gravity operator|(VerticalGravity y, HorizontalGravity x) {
  return Gravity(x, y);
}

/// Convenience gravity constants.

static constexpr HorizontalGravity kHorizontalGravityNone =
    HorizontalGravity(GravityAxisAlignment::NONE);

static constexpr HorizontalGravity kGravityLeft =
    HorizontalGravity(GravityAxisAlignment::MIN);

static constexpr HorizontalGravity kGravityRight =
    HorizontalGravity(GravityAxisAlignment::MAX);

static constexpr HorizontalGravity kGravityCenter =
    HorizontalGravity(GravityAxisAlignment::MIDDLE);

// static constexpr HorizontalGravity kHorizontalGravityFill =
//     HorizontalGravity(GravityAxisAlignment::FILL);

static constexpr VerticalGravity kVerticalGravityNone =
    VerticalGravity(GravityAxisAlignment::NONE);

static constexpr VerticalGravity kGravityTop =
    VerticalGravity(GravityAxisAlignment::MIN);

static constexpr VerticalGravity kGravityBottom =
    VerticalGravity(GravityAxisAlignment::MAX);

static constexpr VerticalGravity kGravityMiddle =
    VerticalGravity(GravityAxisAlignment::MIDDLE);

// static constexpr VerticalGravity kVerticalGravityFill =
//     VerticalGravity(GravityAxisAlignment::FILL);

}  // namespace roo_windows
