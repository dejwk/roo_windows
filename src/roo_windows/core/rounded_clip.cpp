#include "roo_windows/core/rounded_clip.h"

#include <algorithm>

namespace roo_windows {
namespace internal {

using roo_display::AlphaBlend;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;

namespace {

bool SameRadii(BorderStyle::CornerRadii a, BorderStyle::CornerRadii b) {
  return a.top_left == b.top_left && a.top_right == b.top_right &&
         a.bottom_left == b.bottom_left && a.bottom_right == b.bottom_right;
}

bool OpaqueThrough(const RoundedClip* clip, int16_t x, int16_t y) {
  for (; clip != nullptr; clip = clip->parent) {
    if (clip->coverage(x, y) != 255) return false;
  }
  return true;
}

}  // namespace

void RoundedClip::reset(const void* owner, Box bounds, BorderStyle style) {
  const BorderStyle trimmed = style.trim(bounds.width(), bounds.height());
  const bool changed = bounds != bounds_ ||
                       !SameRadii(radii_, trimmed.corner_radii()) ||
                       outline_ != trimmed.outline_width();
  owner_ = owner;
  bounds_ = bounds;
  radii_ = trimmed.corner_radii();
  outline_ = trimmed.outline_width();
  const int inset = outline_.ceil();
  viewport_ = Box(bounds.xMin() + inset, bounds.yMin() + inset,
                  bounds.xMax() - inset, bounds.yMax() - inset);
  if (changed) buildRows();
  std::fill(colors_.begin(), colors_.end(), Color(0));
  direct_press = nullptr;
  completed = false;
  published = false;
}

void RoundedClip::buildRows() {
  rows_.clear();
  colors_.clear();
  top_rows_ = 0;
  bottom_rows_ = 0;
  if (viewport_.empty()) return;
  const int inset = outline_.ceil();
  const int top = std::max(radii_.top_left, radii_.top_right);
  const int bottom = std::max(radii_.bottom_left, radii_.bottom_right);
  top_rows_ = std::min<int>(viewport_.height(), std::max(0, top - inset + 1));
  bottom_rows_ = std::min<int>(viewport_.height() - top_rows_,
                               std::max(0, bottom - inset + 1));
  rows_.reserve(top_rows_ + bottom_rows_);
  size_t samples = 0;
  for (size_t i = 0; i < top_rows_ + bottom_rows_; ++i) {
    const int16_t y = rowY(i);
    Row row{viewport_.xMin(), viewport_.xMax(), viewport_.xMin(),
            viewport_.xMax(), static_cast<uint16_t>(samples)};
    // Only corner runs are scanned. The wide, opaque middle is skipped.
    while (row.visible_min <= row.visible_max &&
           RoundedFillCoverage(bounds_, radii_, outline_, row.visible_min, y) ==
               0) {
      ++row.visible_min;
    }
    while (row.visible_max >= row.visible_min &&
           RoundedFillCoverage(bounds_, radii_, outline_, row.visible_max, y) ==
               0) {
      --row.visible_max;
    }
    row.opaque_min = row.visible_min;
    while (row.opaque_min <= row.visible_max &&
           RoundedFillCoverage(bounds_, radii_, outline_, row.opaque_min, y) !=
               255) {
      ++row.opaque_min;
    }
    row.opaque_max = row.visible_max;
    while (row.opaque_max >= row.opaque_min &&
           RoundedFillCoverage(bounds_, radii_, outline_, row.opaque_max, y) !=
               255) {
      --row.opaque_max;
    }
    samples +=
        row.opaque_min - row.visible_min + row.visible_max - row.opaque_max;
    rows_.push_back(row);
  }
  colors_.resize(samples);
}

int16_t RoundedClip::rowY(size_t index) const {
  return index < top_rows_
             ? viewport_.yMin() + index
             : viewport_.yMax() - bottom_rows_ + 1 + index - top_rows_;
}

const RoundedClip::Row* RoundedClip::row(int16_t y) const {
  if (y < viewport_.yMin() || y > viewport_.yMax()) return nullptr;
  const int index = y - viewport_.yMin();
  if (index < top_rows_) return &rows_[index];
  const int from_bottom = viewport_.yMax() - y;
  if (from_bottom < bottom_rows_) {
    return &rows_[top_rows_ + bottom_rows_ - 1 - from_bottom];
  }
  return nullptr;
}

uint8_t RoundedClip::coverage(int16_t x, int16_t y) const {
  if (!viewport_.contains(x, y)) return 0;
  const Row* scan = row(y);
  if (scan == nullptr) return 255;
  if (x < scan->visible_min || x > scan->visible_max) return 0;
  if (x >= scan->opaque_min && x <= scan->opaque_max) return 255;
  // Consumers only distinguish fractional from zero/full. Exact coverage is
  // applied once by Decoration, so routing performs no square roots.
  return 1;
}

void RoundedClip::opaqueSpan(int16_t y, int16_t& x0, int16_t& x1) const {
  if (y < viewport_.yMin() || y > viewport_.yMax()) {
    x0 = 0;
    x1 = -1;
    return;
  }
  const Row* scan = row(y);
  x0 = scan == nullptr ? viewport_.xMin() : scan->opaque_min;
  x1 = scan == nullptr ? viewport_.xMax() : scan->opaque_max;
}

int16_t RoundedClip::opaqueSpanBandEnd(int16_t y) const {
  if (viewport_.empty() || y > viewport_.yMax()) {
    return std::numeric_limits<int16_t>::max();
  }
  if (y < viewport_.yMin()) return viewport_.yMin() - 1;
  return row(y) == nullptr ? viewport_.yMax() - bottom_rows_ : y;
}

bool RoundedClip::containsOpaque(const Box& box) const {
  if (!viewport_.contains(box)) return false;
  // A rounded rectangle is convex and its narrowest row is at an endpoint.
  return coverage(box.xMin(), box.yMin()) == 255 &&
         coverage(box.xMax(), box.yMin()) == 255 &&
         coverage(box.xMin(), box.yMax()) == 255 &&
         coverage(box.xMax(), box.yMax()) == 255;
}

int RoundedClip::sampleIndex(int16_t x, int16_t y) const {
  const Row* scan = row(y);
  if (scan == nullptr || x < scan->visible_min || x > scan->visible_max) {
    return -1;
  }
  if (x < scan->opaque_min) return scan->offset + x - scan->visible_min;
  if (x > scan->opaque_max) {
    return scan->offset + scan->opaque_min - scan->visible_min + x -
           scan->opaque_max - 1;
  }
  return -1;
}

void RoundedClip::accumulate(int16_t x, int16_t y, Color color) {
  const int index = sampleIndex(x, y);
  if (index < 0 || colors_[index].isOpaque()) return;
  // Traversal is foreground first: every new contribution goes underneath.
  colors_[index] = AlphaBlend(color, colors_[index]);
}

void RoundedClip::accumulateDirect(int16_t x, int16_t y, Color color) {
  if (sampleIndex(x, y) < 0) return;
  if (direct_press != nullptr && direct_press_clip.contains(x, y) &&
      direct_press->extents().contains(x, y)) {
    Color press;
    direct_press->readColors(&x, &y, 1, &press);
    color = AlphaBlend(color, press);
  }
  accumulate(x, y, color);
}

void RoundedClip::accumulateSpan(int16_t y, int16_t x0, int16_t x1,
                                 const Color* colors, Color fill) {
  const Row* scan = row(y);
  if (scan == nullptr) return;
  for (int side = 0; side < 2; ++side) {
    const int first = side == 0 ? scan->visible_min : scan->opaque_max + 1;
    const int last = side == 0 ? scan->opaque_min - 1 : scan->visible_max;
    for (int x = std::max<int>(x0, first); x <= std::min<int>(x1, last); ++x) {
      accumulateDirect(x, y, colors == nullptr ? fill : colors[x - x0]);
    }
  }
}

void RoundedClip::accumulateOverlay(const roo_display::Rasterizable& source,
                                    Box clip, int16_t dx, int16_t dy,
                                    const RoundedClip* active) {
  const Box extents = Box::Intersect(source.extents().translate(dx, dy), clip);
  if (!extents.intersects(viewport_)) return;
  for (size_t i = 0; i < rows_.size(); ++i) {
    const int16_t y = rowY(i);
    if (y < extents.yMin() || y > extents.yMax()) continue;
    const Row& scan = rows_[i];
    for (int side = 0; side < 2; ++side) {
      const int first = side == 0 ? scan.visible_min : scan.opaque_max + 1;
      const int last = side == 0 ? scan.opaque_min - 1 : scan.visible_max;
      for (int16_t x = std::max<int>(first, extents.xMin());
           x <= std::min<int>(last, extents.xMax()); ++x) {
        bool reaches = true;
        for (const RoundedClip* inner = active; inner != this;
             inner = inner->parent) {
          if (inner->coverage(x, y) != 255) {
            reaches = false;
            break;
          }
        }
        const int index = sampleIndex(x, y);
        if (!reaches || colors_[index].isOpaque()) continue;
        const int16_t sx = x - dx;
        const int16_t sy = y - dy;
        Color value;
        source.readColors(&sx, &sy, 1, &value);
        accumulate(x, y, value);
      }
    }
  }
}

bool RoundedClip::contentAt(int16_t x, int16_t y, Color background,
                            Color& color) const {
  const int index = sampleIndex(x, y);
  if (index < 0) return false;
  color = AlphaBlend(background, colors_[index]);
  return true;
}

size_t RoundedClip::storageBytes() const {
  return rows_.capacity() * sizeof(Row) + colors_.capacity() * sizeof(Color);
}

RoundedClipOutput::RoundedClipOutput(roo_display::DisplayOutput& output,
                                     RoundedClip& clip)
    : output_(output),
      clip_(clip),
      capabilities_(output.getCapabilities().supportsBlending(), false) {}

const roo_display::DisplayOutput::ColorFormat&
RoundedClipOutput::getColorFormat() const {
  return output_.getColorFormat();
}

void RoundedClipOutput::setAddress(uint16_t x0, uint16_t y0, uint16_t x1,
                                   uint16_t y1, BlendingMode mode) {
  address_ = Box(x0, y0, x1, y1);
  mode_ = mode;
  x_ = x0;
  y_ = y0;
  direct_ = clip_.containsOpaque(address_);
  if (direct_) output_.setAddress(x0, y0, x1, y1, mode);
}

void RoundedClipOutput::write(Color* colors, uint32_t count) {
  if (direct_) {
    output_.write(colors, count);
    return;
  }
  writeSpan(colors, Color(0), count);
}

void RoundedClipOutput::fill(Color color, uint32_t count) {
  if (direct_) {
    output_.fill(color, count);
    return;
  }
  writeSpan(nullptr, color, count);
}

void RoundedClipOutput::writeSpan(Color* colors, Color fill, uint32_t count) {
  uint32_t pos = 0;
  while (pos < count) {
    const int16_t end =
        x_ + std::min<uint32_t>(count - pos, address_.xMax() - x_ + 1) - 1;
    int16_t lo;
    int16_t hi;
    clip_.opaqueSpan(y_, lo, hi);
    lo = std::max(lo, x_);
    hi = std::min(hi, end);
    clip_.accumulateSpan(y_, x_, end,
                         colors == nullptr ? nullptr : colors + pos, fill);
    if (hi >= lo) {
      output_.setAddress(lo, y_, hi, y_, mode_);
      if (colors == nullptr) {
        output_.fill(fill, hi - lo + 1);
      } else {
        output_.write(colors + pos + lo - x_, hi - lo + 1);
      }
    }
    pos += end - x_ + 1;
    x_ = end + 1;
    if (x_ > address_.xMax()) {
      x_ = address_.xMin();
      ++y_;
    }
  }
}

void RoundedClipOutput::writePixels(BlendingMode mode, Color* colors,
                                    int16_t* x, int16_t* y, uint16_t count) {
  uint16_t kept = 0;
  for (uint16_t i = 0; i < count; ++i) {
    if (clip_.coverage(x[i], y[i]) == 255) {
      x[kept] = x[i];
      y[kept] = y[i];
      colors[kept++] = colors[i];
    } else {
      clip_.accumulateDirect(x[i], y[i], colors[i]);
    }
  }
  if (kept != 0) output_.writePixels(mode, colors, x, y, kept);
}

void RoundedClipOutput::fillPixels(BlendingMode mode, Color color, int16_t* x,
                                   int16_t* y, uint16_t count) {
  uint16_t kept = 0;
  for (uint16_t i = 0; i < count; ++i) {
    if (clip_.coverage(x[i], y[i]) == 255) {
      x[kept] = x[i];
      y[kept++] = y[i];
    } else {
      clip_.accumulateDirect(x[i], y[i], color);
    }
  }
  if (kept != 0) output_.fillPixels(mode, color, x, y, kept);
}

void RoundedClipOutput::fillRect(BlendingMode mode, Color color, Box box) {
  if (clip_.containsOpaque(box)) {
    int16_t x0 = box.xMin();
    int16_t y0 = box.yMin();
    int16_t x1 = box.xMax();
    int16_t y1 = box.yMax();
    output_.fillRects(mode, color, &x0, &y0, &x1, &y1, 1);
    return;
  }
  box.clip(clip_.viewport());
  if (box.empty()) return;
  // Emit coalesced runs directly: a full rectangle batch would add 256 bytes
  // of stack per nested filter on the target, for little benefit here.
  auto emit = [&](const Box& rect) {
    int16_t x0 = rect.xMin();
    int16_t y0 = rect.yMin();
    int16_t x1 = rect.xMax();
    int16_t y1 = rect.yMax();
    output_.fillRects(mode, color, &x0, &y0, &x1, &y1, 1);
  };
  Box run(0, 0, -1, -1);
  for (int16_t y = box.yMin(); y <= box.yMax(); ++y) {
    int16_t lo;
    int16_t hi;
    clip_.opaqueSpan(y, lo, hi);
    lo = std::max(lo, box.xMin());
    hi = std::min(hi, box.xMax());
    clip_.accumulateSpan(y, box.xMin(), box.xMax(), nullptr, color);
    if (!run.empty() && (lo != run.xMin() || hi != run.xMax())) {
      emit(run);
      run = Box(0, 0, -1, -1);
    }
    if (hi < lo) continue;
    run = Box(lo, run.empty() ? y : run.yMin(), hi, y);
  }
  if (!run.empty()) {
    emit(run);
  }
}

void RoundedClipOutput::writeRects(BlendingMode mode, Color* colors,
                                   int16_t* x0, int16_t* y0, int16_t* x1,
                                   int16_t* y1, uint16_t count) {
  for (uint16_t i = 0; i < count; ++i) {
    fillRect(mode, colors[i], Box(x0[i], y0[i], x1[i], y1[i]));
  }
}

void RoundedClipOutput::fillRects(BlendingMode mode, Color color, int16_t* x0,
                                  int16_t* y0, int16_t* x1, int16_t* y1,
                                  uint16_t count) {
  for (uint16_t i = 0; i < count; ++i) {
    fillRect(mode, color, Box(x0[i], y0[i], x1[i], y1[i]));
  }
}

RoundedOverlay::RoundedOverlay(const roo_display::Rasterizable* source,
                               Box clip, int16_t dx, int16_t dy,
                               const RoundedClip* mask)
    : source_(source),
      extents_(Box::Intersect(clip, source->extents().translate(dx, dy))),
      dx_(dx),
      dy_(dy),
      mask_(mask) {}

void RoundedOverlay::readColors(const int16_t* x, const int16_t* y,
                                uint32_t count, Color* result) const {
  for (uint32_t i = 0; i < count; ++i) {
    if (!extents_.contains(x[i], y[i]) || !OpaqueThrough(mask_, x[i], y[i])) {
      result[i] = Color(0);
    } else {
      const int16_t sx = x[i] - dx_;
      const int16_t sy = y[i] - dy_;
      source_->readColors(&sx, &sy, 1, &result[i]);
    }
  }
}

bool RoundedOverlay::readUniformColorRect(int16_t x0, int16_t y0, int16_t x1,
                                          int16_t y1, Color* result) const {
  const Box box(x0, y0, x1, y1);
  if (!extents_.contains(box)) return false;
  for (const RoundedClip* mask = mask_; mask != nullptr; mask = mask->parent) {
    if (!mask->containsOpaque(box)) return false;
  }
  return source_->readUniformColorRect(x0 - dx_, y0 - dy_, x1 - dx_, y1 - dy_,
                                       result);
}

RoundedDecoration::RoundedDecoration(Decoration decoration,
                                     const RoundedClip* clip, Color background,
                                     Color tint)
    : decoration_(std::move(decoration)),
      clip_(clip),
      background_(background),
      tint_(tint) {}

void RoundedDecoration::readColors(const int16_t* x, const int16_t* y,
                                   uint32_t count, Color* result) const {
  for (uint32_t i = 0; i < count; ++i) {
    Color content;
    if (clip_->contentAt(x[i], y[i], background_, content)) {
      result[i] =
          decoration_.readWithContent(x[i], y[i], AlphaBlend(content, tint_));
    } else {
      decoration_.readColors(x + i, y + i, 1, result + i);
    }
  }
}

bool RoundedDecoration::readUniformColorRect(int16_t x0, int16_t y0, int16_t x1,
                                             int16_t y1, Color* result) const {
  return clip_->containsOpaque(Box(x0, y0, x1, y1)) &&
         decoration_.readUniformColorRect(x0, y0, x1, y1, result);
}

}  // namespace internal
}  // namespace roo_windows
