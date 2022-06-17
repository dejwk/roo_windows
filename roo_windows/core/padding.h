#pragma once

namespace roo_windows {

class Padding {
 public:
  Padding() : Padding(0) {}
  Padding(int16_t padding) : value_h_(padding + 32), value_v_(padding + 32) {}
  Padding(int16_t horizontal, int16_t vertical)
      : value_h_(horizontal + 32), value_v_(vertical + 32) {}

  int16_t top() const { return (int16_t)value_v_ - 32; }
  int16_t left() const { return (int16_t)value_h_ - 32; }
  int16_t right() const { return (int16_t)value_h_ - 32; }
  int16_t bottom() const { return (int16_t)value_v_ - 32; }

 private:
  friend bool operator==(Padding a, Padding b);

  uint8_t value_h_;
  uint8_t value_v_;
};

inline bool operator==(Padding a, Padding b) {
  return a.value_h_ == b.value_h_ && a.value_v_ == b.value_v_;
}

}  // namespace roo_windows
