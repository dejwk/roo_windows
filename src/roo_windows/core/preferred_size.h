#pragma once

#include "roo_windows/core/rect.h"

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
  class Width {
   public:
    XDim value() const { return value_; }
    bool isExact() const { return value_ >= 0; }
    bool isMatchParent() const { return value_ == -1; }
    bool isWrapContent() const { return value_ == -2; }
    bool isZero() const { return value_ == 0; }

   private:
    friend class PreferredSize;
    friend bool operator==(Width a, Width b);

#if defined(__linux__) || defined(__linux) || defined(linux)
    friend std::ostream& operator<<(std::ostream& os, Width w);
#endif

    constexpr Width(XDim value) : value_(value) {}

    XDim value_;
  };

  class Height {
   public:
    YDim value() const { return value_; }
    bool isExact() const { return value_ >= 0; }
    bool isMatchParent() const { return value_ == -1; }
    bool isWrapContent() const { return value_ == -2; }
    bool isZero() const { return value_ == 0; }

   private:
    friend class PreferredSize;
    friend bool operator==(Height a, Height b);

#if defined(__linux__) || defined(__linux) || defined(linux)
    friend std::ostream& operator<<(std::ostream& os, Height h);
#endif

    constexpr Height(YDim value) : value_(value) {}

    YDim value_;
  };

  constexpr static inline Width ExactWidth(XDim value) {
    return PreferredSize::Width(value);
  }

  constexpr static inline Width MatchParentWidth() { return Width(-1); }

  constexpr static inline Width WrapContentWidth() { return Width(-2); }

  constexpr static inline Height ExactHeight(YDim value) {
    return PreferredSize::Height(value);
  }

  constexpr static inline Height MatchParentHeight() { return Height(-1); }

  constexpr static inline Height WrapContentHeight() { return Height(-2); }

  // Creates a new preferred size with the specified width and height.
  PreferredSize(Width width, Height height) : width_(width), height_(height) {}

  Width width() const { return width_; }
  Height height() const { return height_; }

 private:
  Width width_;
  Height height_;
};

inline bool operator==(PreferredSize::Width a, PreferredSize::Width b) {
  return a.value_ == b.value_;
}

inline bool operator==(PreferredSize::Height a, PreferredSize::Height b) {
  return a.value_ == b.value_;
}

#if defined(__linux__) || defined(__linux) || defined(linux)

inline std::ostream& operator<<(std::ostream& os, PreferredSize::Width w) {
  if (w == PreferredSize::MatchParentWidth()) {
    os << "match-parent-width";
  } else if (w == PreferredSize::WrapContentWidth()) {
    os << "wrap-content-width";
  } else {
    os << "width:" << w.value_;
  }
  return os;
}

inline std::ostream& operator<<(std::ostream& os, PreferredSize::Height h) {
  if (h == PreferredSize::MatchParentHeight()) {
    os << "match-parent-height";
  } else if (h == PreferredSize::WrapContentHeight()) {
    os << "wrap-content-height";
  } else {
    os << "height:" << h.value_;
  }
  return os;
}

#endif  // defined(__linux__)

}  // namespace roo_windows
