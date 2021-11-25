#pragma once

namespace roo_windows {

class Padding {
 public:
  Padding() : Padding(0) {}
  Padding(int16_t padding) : value_h_(padding), value_v_(padding) {}
  Padding(int16_t horizontal, int16_t vertical)
      : value_h_(horizontal), value_v_(vertical) {}

  int16_t top() const { return value_v_; }
  int16_t left() const { return value_h_; }
  int16_t right() const { return value_h_; }
  int16_t bottom() const { return value_v_; }

 private:
  friend bool operator==(Padding a, Padding b);

  uint8_t value_h_;
  uint8_t value_v_;
};

inline bool operator==(Padding a, Padding b) {
  return a.value_h_ == b.value_h_ && a.value_v_ == b.value_v_;
}

}  // namespace roo_windows