#include "roo_windows/core/clipper.h"

namespace roo_windows {
namespace internal {

using namespace roo_display;

static const int kMaxBufSize = 32;

void OverlayStack::reset(const ClippedOverlay* begin,
                         const ClippedOverlay* end) {
  begin_ = begin;
  end_ = end;
  if (begin == end) {
    extents_ = Box(0, 0, -1, -1);
    return;
  }
  extents_ = begin->extents();
  ++begin;
  for (const ClippedOverlay* r = begin; r != end; ++r) {
    extents_ = Box::extent(extents_, r->extents());
  }
}

void OverlayStack::ReadColors(const int16_t* x, const int16_t* y,
                              uint32_t count, Color* result) const {
  Color::Fill(result, count, color::Transparent);
  int16_t newx[kMaxBufSize];
  int16_t newy[kMaxBufSize];
  Color newresult[kMaxBufSize];
  uint32_t offsets[kMaxBufSize];
  for (const ClippedOverlay* r = end_; r != begin_;) {
    --r;
    Box bounds = r->extents();
    uint32_t offset = 0;
    while (offset < count) {
      int buf_size = 0;
      uint32_t start_offset = offset;
      do {
        if (bounds.contains(x[offset], y[offset])) {
          newx[buf_size] = x[offset];
          newy[buf_size] = y[offset];
          offsets[buf_size] = offset;
          ++buf_size;
        }
        offset++;
      } while (offset < count && buf_size < kMaxBufSize);
      r->ReadColors(newx, newy, buf_size, newresult);
      int buf_idx = 0;
      for (uint32_t i = start_offset; i < offset; ++i) {
        if (buf_idx < buf_size && offsets[buf_idx] == i) {
          // Found point in the bounds for which we have the color.
          result[i] = alphaBlend(result[i], newresult[buf_idx]);
          ++buf_idx;
        }
      }
    }
  }
}

bool OverlayStack::ReadColorRect(int16_t xMin, int16_t yMin, int16_t xMax,
                                 int16_t yMax, Color* result) const {
  const ClippedOverlay* r = end_;
  bool is_uniform_color = true;
  *result = color::Transparent;
  Box box(xMin, yMin, xMax, yMax);
  Color buffer[box.area()];
  for (const ClippedOverlay* r = end_; r != begin_;) {
    --r;
    Box bounds = r->extents();
    Box clipped = Box::intersect(bounds, box);
    if (clipped.empty()) {
      // This rect does not contribute to the outcome.
      continue;
    }
    if (is_uniform_color && !clipped.contains(box)) {
      // This rect does not fill the entire box; we can no longer use fast path.
      is_uniform_color = false;
      Color::Fill(&result[1], box.area() - 1, *result);
    }
    if (r->ReadColorRect(clipped.xMin(), clipped.yMin(), clipped.xMax(),
                         clipped.yMax(), buffer)) {
      if (is_uniform_color) {
        *result = alphaBlend(*result, *buffer);
      } else {
        for (int16_t y = clipped.yMin(); y <= clipped.yMax(); ++y) {
          Color* row = &result[(y - yMin) * box.width()];
          for (int16_t x = clipped.xMin(); x <= clipped.xMax(); ++x) {
            row[x - xMin] = alphaBlend(row[x - xMin], *buffer);
          }
        }
      }
    } else {
      if (is_uniform_color) {
        is_uniform_color = false;
        Color::Fill(&result[1], box.area() - 1, *result);
      }
      uint32_t i = 0;
      for (int16_t y = clipped.yMin(); y <= clipped.yMax(); ++y) {
        Color* row = &result[(y - yMin) * box.width()];
        for (int16_t x = clipped.xMin(); x <= clipped.xMax(); ++x) {
          row[x - xMin] = alphaBlend(row[x - xMin], buffer[i++]);
        }
      }
    }
  }
  return is_uniform_color;
}

}  // namespace internal
}  // namespace roo_windows