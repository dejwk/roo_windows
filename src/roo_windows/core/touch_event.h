#pragma once

#include <inttypes.h>

namespace roo_windows {

/// Touch sample carrying screen coordinates and a microsecond timestamp.
///
/// Used by the gesture detector to reconstruct drag velocity and fling motion.
class TouchPoint {
 public:
  TouchPoint() : TouchPoint(0, 0, 0) {}
  TouchPoint(int16_t x, int16_t y, unsigned long when_micros)
      : x_(x), y_(y), when_micros_(when_micros) {}

  int16_t x() const { return x_; }
  int16_t y() const { return y_; }
  unsigned long when_micros() const { return when_micros_; }

 private:
  int16_t x_;
  int16_t y_;
  unsigned long when_micros_;
};

/// Touch event dispatched to widgets: kind (`DOWN`, `UP`, `MOVE`) plus the
/// touch location in the receiver's coordinate space.
class TouchEvent {
 public:
  enum Type { DOWN, UP, MOVE };

  TouchEvent(Type type, int16_t x, int16_t y) : type_(type), x_(x), y_(y) {}

  Type type() const { return type_; }
  int16_t x() const { return x_; }
  int16_t y() const { return y_; }

 private:
  Type type_;
  int16_t x_, y_;
};

}  // namespace roo_windows
