#pragma once

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

 private:
  friend class Gravity;
};

class Gravity {
 public:
  constexpr Gravity() : value_(0) {}

  constexpr Gravity(HorizontalGravity x, VerticalGravity y)
      : value_(x.alignment_ | (y.alignment_ << 4)) {}

  constexpr HorizontalGravity x() const {
    return HorizontalGravity((GravityAxisAlignment)(value_ & 0xF));
  }
  constexpr VerticalGravity y() const {
    return VerticalGravity((GravityAxisAlignment)(value_ >> 4));
  }

 private:
  uint8_t value_;
};

static constexpr HorizontalGravity kHorizontalGravityNone =
    HorizontalGravity(GravityAxisAlignment::NONE);

static constexpr HorizontalGravity kHorizontalGravityLeft =
    HorizontalGravity(GravityAxisAlignment::MIN);

static constexpr HorizontalGravity kHorizontalGravityRight =
    HorizontalGravity(GravityAxisAlignment::MAX);

static constexpr HorizontalGravity kHorizontalGravityCenter =
    HorizontalGravity(GravityAxisAlignment::MIDDLE);

// static constexpr HorizontalGravity kHorizontalGravityFill =
//     HorizontalGravity(GravityAxisAlignment::FILL);

static constexpr VerticalGravity kVerticalGravityNone =
    VerticalGravity(GravityAxisAlignment::NONE);

static constexpr VerticalGravity kVerticalGravityTop =
    VerticalGravity(GravityAxisAlignment::MIN);

static constexpr VerticalGravity kVerticalGravityBottom =
    VerticalGravity(GravityAxisAlignment::MAX);

static constexpr VerticalGravity kVerticalGravityMiddle =
    VerticalGravity(GravityAxisAlignment::MIDDLE);

// static constexpr VerticalGravity kVerticalGravityFill =
//     VerticalGravity(GravityAxisAlignment::FILL);

}  // namespace roo_windows
