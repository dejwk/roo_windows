#pragma once

#include <inttypes.h>

namespace roo_windows {

// Represents numbers in the range of [0, 4095.9375], with fixed-point precision
// of 0.0625 (1/16th).
class SmallNumber {
 public:
  SmallNumber(uint16_t val) : val_(val << 4) {}

  SmallNumber(int val)
      : val_(val >= 4096 ? 65535
             : val < 0   ? 0
                         : ((uint16_t)val) << 4) {}

  SmallNumber(unsigned int val)
      : val_(val >= 4096 ? 65535 : ((uint16_t)val) << 4) {}

  SmallNumber(float val)
      : val_(std::min(std::max(val * 16.0f, 0.0f), 65535.0f)) {}

  SmallNumber(double val)
      : val_(std::min(std::max(val * 16.0, 0.0), 65535.0)) {}

  static SmallNumber Of16ths(uint16_t val) { return SmallNumber(val, true); }

  operator float() const { return ((float)val_) / 16; }

  // Returns the integer part of the number.
  uint16_t floor() const { return val_ >> 4; }

  // Returns the integer ceiling of the number.
  uint16_t ceil() const { return 4096 - ((65536 - val_) >> 4); }

  // Returns 16 * the fractional part of the number.
  uint8_t frac_16ths() const { return val_ & 15; }

  bool operator==(SmallNumber other) const { return val_ == other.val_; }

  bool operator!=(SmallNumber other) const { return val_ != other.val_; }

  bool operator<(SmallNumber other) const { return val_ < other.val_; }

  bool operator<=(SmallNumber other) const { return val_ <= other.val_; }

  bool operator>(SmallNumber other) const { return val_ > other.val_; }

  bool operator>=(SmallNumber other) const { return val_ >= other.val_; }

  SmallNumber operator+(SmallNumber other) const {
    return SmallNumber(val_ + other.val_, false);
  }

  SmallNumber operator-(SmallNumber other) const {
    return SmallNumber(val_ - other.val_, false);
  }

  SmallNumber& operator+=(SmallNumber other) {
    val_ += other.val_;
    return *this;
  }

  SmallNumber& operator-=(SmallNumber other) {
    val_ -= other.val_;
    return *this;
  }

 private:
  SmallNumber(uint16_t val, bool dummy) : val_(val) {}

  uint16_t val_;
};

}  // namespace roo_windows