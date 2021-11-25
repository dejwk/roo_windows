#pragma once

#include <inttypes.h>

namespace roo_windows {

class TouchEvent {
 public:
  enum Type { PRESSED, RELEASED, MOVED, DRAGGED, SWIPED };

  TouchEvent(Type type, unsigned long duration_ms, int16_t x_start,
             int16_t y_start, int16_t x_end, int16_t y_end)
      : type_(type),
        x_start_(x_start),
        y_start_(y_start),
        x_end_(x_end),
        y_end_(y_end),
        duration_ms_(duration_ms) {}

  Type type() const { return type_; }
  unsigned long duration() const { return duration_ms_; }
  int16_t startX() const { return x_start_; }
  int16_t startY() const { return y_start_; }
  int16_t x() const { return x_end_; }
  int16_t y() const { return y_end_; }

  int16_t dx() const { return x() - startX(); }
  int16_t dy() const { return y() - startY(); }

 private:
  Type type_;
  int16_t x_start_, y_start_, x_end_, y_end_;
  unsigned long duration_ms_;
};

}  // namespace
