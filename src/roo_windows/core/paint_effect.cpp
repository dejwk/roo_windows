#include "roo_windows/core/paint_effect.h"

#include <algorithm>

namespace roo_windows::internal {

using roo_display::AlphaBlend;
using roo_display::ApplyBlendingInPlace;
using roo_display::ApplyBlendingSingleSourceInPlace;
using roo_display::Blender;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;
using roo_display::FillColor;

namespace {
constexpr int kBatchSize = 64;
constexpr BlendingMode kOver = BlendingMode::kSourceOver;
constexpr BlendingMode kTint = BlendingMode::kSourceAtop;

// Checks whether materialized pixels can use the compact raster contract.
bool IsUniform(const Color* colors, int32_t count) {
  for (int32_t i = 1; i < count; ++i) {
    if (colors[i] != colors[0]) return false;
  }
  return true;
}

// Applies the resolved tint through roo_display's bulk SourceAtop operators.
// Chunking keeps their int16_t count contract safe for large rectangles.
void Modulate(Color* colors, const Color* tints, int count, bool uniform) {
  if (uniform && tints[0].a() == 0) return;
  Blender<kTint> blender;
  while (count > 0) {
    const int16_t batch = std::min(count, 32767);
    if (uniform) {
      blender.applySingleSourceInPlace(colors, tints[0], batch);
    } else {
      blender.applyInPlace(colors, tints, batch);
      tints += batch;
    }
    colors += batch;
    count -= batch;
  }
}
}  // namespace

Color PaintEffectStack::apply(int16_t x, int16_t y, Color color) const {
  if (color.a() == 0) return color;
  Color tint(0);
  for (const PaintEffect* e = first_; e != nullptr && e != limit_;
       e = e->parent()) {
    if (!e->bounds().contains(x, y)) continue;
    tint = AlphaBlend(
        tint, e->press() == nullptr ? e->tint() : e->press()->get(x, y));
  }
  return Blender<kTint>().apply(color, tint);
}

Box PaintEffectStack::extents() const {
  Box result(0, 0, -1, -1);
  for (const PaintEffect* e = first_; e != nullptr && e != limit_;
       e = e->parent()) {
    result = Box::Extent(result, e->bounds());
  }
  return result;
}

void PaintEffectStack::readColors(const int16_t* x, const int16_t* y,
                                  uint32_t count, Color* result) const {
  FillColor(result, count, Color(0));
  // Gather only surviving points, so even alternating in/out coordinates need
  // one virtual ripple read per batch, rather than one per surviving pixel.
  constexpr int kPointBatchSize = 32;
  int16_t sx[kPointBatchSize];
  int16_t sy[kPointBatchSize];
  uint32_t indices[kPointBatchSize];
  Color buffer[kPointBatchSize];
  for (const PaintEffect* e = first_; e != nullptr && e != limit_;
       e = e->parent()) {
    uint32_t i = 0;
    while (i < count) {
      if (e->press() == nullptr) {
        if (!e->bounds().contains(x[i], y[i])) {
          ++i;
          continue;
        }
        const uint32_t begin = i++;
        while (i < count && i - begin < 32767 &&
               e->bounds().contains(x[i], y[i])) {
          ++i;
        }
        ApplyBlendingSingleSourceInPlace(kOver, result + begin, e->tint(),
                                         i - begin);
        continue;
      }
      int size = 0;
      do {
        if (e->bounds().contains(x[i], y[i])) {
          sx[size] = x[i];
          sy[size] = y[i];
          indices[size++] = i;
        }
        ++i;
      } while (i < count && size < kPointBatchSize);
      if (size == 0) continue;
      e->press()->readColors(sx, sy, size, buffer);
      roo_display::ApplyBlendingInPlaceIndexed(kOver, result, buffer, size,
                                               indices);
    }
  }
}

bool PaintEffectStack::readUniformColorRect(int16_t x0, int16_t y0, int16_t x1,
                                            int16_t y1, Color* result) const {
  Color accumulated = roo_display::color::Transparent;
  const Box box(x0, y0, x1, y1);
  bool uniform = true;
  for (const PaintEffect* e = first_; e != nullptr && e != limit_;
       e = e->parent()) {
    const Box clipped = Box::Intersect(box, e->bounds());
    if (clipped.empty()) continue;
    Color layer = e->tint();
    if (e->press() != nullptr && !e->press()->readUniformColorRect(
                                     clipped.xMin(), clipped.yMin(),
                                     clipped.xMax(), clipped.yMax(), &layer)) {
      uniform = false;
      continue;
    }
    if (layer.a() == 0) continue;
    if (clipped == box && layer.a() == 255) {
      accumulated = layer;
      uniform = true;
    } else if (uniform) {
      const Color inside = AlphaBlend(accumulated, layer);
      uniform = clipped == box || inside == accumulated;
      accumulated = inside;
    }
  }
  if (uniform) *result = accumulated;
  return uniform;
}

bool PaintEffectStack::readTile(Box box, Color* result) const {
  result[0] = Color(0);
  bool uniform = true;
  Color buffer[kBatchSize];
  for (const PaintEffect* e = first_; e != nullptr && e != limit_;
       e = e->parent()) {
    const Box clipped = Box::Intersect(box, e->bounds());
    if (clipped.empty()) continue;
    buffer[0] = e->tint();
    const bool layer_uniform =
        e->press() == nullptr ||
        e->press()->readColorRect(clipped.xMin(), clipped.yMin(),
                                  clipped.xMax(), clipped.yMax(), buffer);
    if (layer_uniform) {
      if (buffer[0].a() == 0) continue;
      if (clipped == box && buffer[0].a() == 255) {
        result[0] = buffer[0];
        uniform = true;
        continue;
      }
      if (uniform) {
        const Color inside = AlphaBlend(result[0], buffer[0]);
        if (clipped == box || inside == result[0]) {
          result[0] = inside;
          continue;
        }
      }
    }
    if (uniform) {
      FillColor(result + 1, box.area() - 1, result[0]);
      uniform = false;
    }
    // Source rows are packed in the clipped buffer; destination rows retain
    // the caller's stride. No pixel needs to re-evaluate scope membership.
    int dst = (clipped.yMin() - box.yMin()) * box.width() + clipped.xMin() -
              box.xMin();
    // Full-width strips are contiguous in both buffers, including the common
    // whole-rectangle case. Blend them in one bulk call instead of per row.
    if (clipped.width() == box.width()) {
      if (layer_uniform) {
        ApplyBlendingSingleSourceInPlace(kOver, result + dst, buffer[0],
                                         clipped.area());
      } else {
        ApplyBlendingInPlace(kOver, result + dst, buffer, clipped.area());
      }
      continue;
    }
    int src = 0;
    for (int32_t y = clipped.yMin(); y <= clipped.yMax(); ++y) {
      if (layer_uniform) {
        ApplyBlendingSingleSourceInPlace(kOver, result + dst, buffer[0],
                                         clipped.width());
      } else {
        ApplyBlendingInPlace(kOver, result + dst, buffer + src,
                             clipped.width());
      }
      dst += box.width();
      src += clipped.width();
    }
  }
  return uniform || IsUniform(result, box.area());
}

bool PaintEffectStack::readColorRect(int16_t x0, int16_t y0, int16_t x1,
                                     int16_t y1, Color* result) const {
  const Box box(x0, y0, x1, y1);
  if (box.area() <= kBatchSize) return readTile(box, result);
  if (readUniformColorRect(x0, y0, x1, y1, result)) return true;
  // Large queries use bounded horizontal tiles. An identical uniform prefix
  // stays implicit until the first differing tile, then expands just once.
  Color tile[kBatchSize];
  bool uniform = true;
  int offset = 0;
  for (int32_t y = y0; y <= y1; ++y) {
    for (int32_t x = x0; x <= x1; x += kBatchSize) {
      const int end = std::min<int32_t>(x1, x + kBatchSize - 1);
      const int count = end - x + 1;
      const bool one = readTile(Box(x, y, end, y), tile);
      if (offset == 0) result[0] = tile[0];
      if (!one || tile[0] != result[0]) {
        if (uniform) FillColor(result, offset, result[0]);
        uniform = false;
      }
      if (!uniform) {
        if (one)
          FillColor(result + offset, count, tile[0]);
        else
          std::copy_n(tile, count, result + offset);
      }
      offset += count;
    }
  }
  return uniform;
}

void PaintEffectStack::applyColors(const int16_t* x, const int16_t* y,
                                   uint32_t count, Color* colors) const {
  if (first_ == nullptr || first_ == limit_) return;
  Color tints[kBatchSize];
  for (uint32_t i = 0; i < count; i += kBatchSize) {
    const int size = std::min<uint32_t>(kBatchSize, count - i);
    readColors(x + i, y + i, size, tints);
    Modulate(colors + i, tints, size, false);
  }
}

bool PaintEffectStack::applyRect(Box box, Color* colors, bool uniform) const {
  if (first_ == nullptr || first_ == limit_ || box.empty()) return uniform;
  const Color source = colors[0];
  if (uniform && source.a() == 0) return true;
  Color tint;
  if (readUniformColorRect(box.xMin(), box.yMin(), box.xMax(), box.yMax(),
                           &tint)) {
    if (uniform) {
      colors[0] = Blender<kTint>().apply(source, tint);
      return true;
    }
    Modulate(colors, &tint, box.area(), true);
    return IsUniform(colors, box.area());
  }
  if (uniform) FillColor(colors + 1, box.area() - 1, source);
  Color tints[kBatchSize];
  if (box.area() <= kBatchSize) {
    const bool one = readTile(box, tints);
    Modulate(colors, tints, box.area(), one);
  } else {
    int offset = 0;
    for (int32_t y = box.yMin(); y <= box.yMax(); ++y) {
      for (int32_t x = box.xMin(); x <= box.xMax(); x += kBatchSize) {
        const int end = std::min<int32_t>(box.xMax(), x + kBatchSize - 1);
        const int count = end - x + 1;
        const bool one = readTile(Box(x, y, end, y), tints);
        Modulate(colors + offset, tints, count, one);
        offset += count;
      }
    }
  }
  return IsUniform(colors, box.area());
}

}  // namespace roo_windows::internal
