#pragma once

namespace roo_windows {

class Margins {
 public:
  Margins() : Margins(0) {}
  Margins(int16_t margins) : value_(margins) {}
  Margins(int16_t horizontal, int16_t vertical)
      : value_((horizontal + vertical) / 2) {}

  int16_t top() const { return value_; }
  int16_t left() const { return value_; }
  int16_t right() const { return value_; }
  int16_t bottom() const { return value_; }

 private:
  friend bool operator==(Margins a, Margins b);

  int8_t value_;
};

inline bool operator==(Margins a, Margins b) {
  return a.value_ == b.value_;
}

}  // namespace roo_windows
