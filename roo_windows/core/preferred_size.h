#pragma once

#include "roo_glog/logging.h"

namespace roo_windows {

// PreferredSize are used by views to tell their parents what is their preferred
// width and height. For each dimension, it can specify one of:
//
// + MATCH_PARENT, which means that the view wants to be as big as its parent
//   (minus padding)
// + WRAP_CONTENT, which means that the view wants to be just big enough to
//   enclose its content (plus padding)
// + an exact number
//
class PreferredSize {
 public:
  class Dimension {
   public:
    int16_t value() const { return value_; }
    bool isExact() const { return value_ >= 0; }
    bool isMatchParent() const { return value_ == -1; }
    bool isWrapContent() const { return value_ == -2; }

   private:
    friend class PreferredSize;
    friend bool operator==(Dimension a, Dimension b);

#if defined(__linux__) || defined(__linux) || defined(linux)
    friend std::ostream& operator<<(std::ostream& os, Dimension dim);
#endif

    constexpr Dimension(int16_t value) : value_(value) {}

    int16_t value_;
  };

  constexpr static inline Dimension Exact(int16_t value) {
    return Dimension(value);
  }

  constexpr static inline Dimension MatchParent() { return Dimension(-1); }

  constexpr static inline Dimension WrapContent() { return Dimension(-2); }

  // Creates a new preferred size with the specified width and height.
  PreferredSize(Dimension width, Dimension height)
      : width_(width), height_(height) {}

  Dimension width() const { return width_; }
  Dimension height() const { return height_; }

 private:
  Dimension width_;
  Dimension height_;
};

inline bool operator==(PreferredSize::Dimension a, PreferredSize::Dimension b) {
  return a.value_ == b.value_;
}

#if defined(__linux__) || defined(__linux) || defined(linux)

inline std::ostream& operator<<(std::ostream& os, PreferredSize::Dimension dim) {
  if (dim == PreferredSize::MatchParent()) {
    LOG(INFO) << "match-parent";
  } else if (dim == PreferredSize::WrapContent()) {
    LOG(INFO) << "wrap-content";
  } else {
    os << dim.value_;
  }
  return os;
}

#endif  // defined(__linux__)

}  // namespace roo_windows
