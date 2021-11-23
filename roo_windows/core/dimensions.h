#pragma once

namespace roo_windows {

// Width and height of a screen object.
class Dimensions {
 public:
  Dimensions() : Dimensions(0, 0) {}
  Dimensions(int16_t width, int16_t height) : width_(width), height_(height) {}

  int16_t width() const { return width_; }
  int16_t height() const { return height_; }

 private:
  int16_t width_;
  int16_t height_;
};

}  // namespace roo_windows
