#pragma once

// Benchmark-only direct four-member damage filter. Production ClipperOutput
// still compiles damage gaps into the existing ExclusionFilter.
#include <algorithm>
#include <cstdint>

#include "roo_windows/core/damage_region.h"
#include "roo_windows/core/exclusion_filter.h"

namespace roo_windows::benchmarks {

/// Filters output to the borrowed damage union before semantic exclusions.
/// Borrowed inputs and the underlying output must outlive this adapter.
class NativeDamageFilter final : public roo_display::DisplayOutput {
 public:
  /// Creates a damage filter, optionally forwarding visible output through the
  /// same semantic/rounded exclusion union used by the compiled-gap path.
  NativeDamageFilter(roo_display::DisplayOutput& output,
                     const internal::DamageRegion& damage,
                     const internal::ExclusionUnion& semantic_exclusions,
                     bool has_semantic_exclusions)
      : output_(output),
        damage_(damage),
        semantic_filter_(output, &semantic_exclusions),
        downstream_(
            has_semantic_exclusions
                ? static_cast<roo_display::DisplayOutput*>(&semantic_filter_)
                : &output),
        capabilities_(output.getCapabilities().supportsBlending(), false) {}

  /// Opens a transaction on the underlying output.
  void begin() override { output_.begin(); }

  /// Completes a transaction on the underlying output.
  void end() override { output_.end(); }

  /// Waits for the underlying output to complete deferred drawing.
  void flush() override { output_.flush(); }

  /// Records the input stream window without exposing damage gaps downstream.
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override {
    window_ = roo_display::Box(x0, y0, x1, y1);
    mode_ = mode;
    cursor_x_ = x0;
    cursor_y_ = y0;
    run_remaining_ = 0;
    run_visible_ = false;
    cached_band_end_ = -1;
  }

  /// Forwards visible colors while consuming every input stream position.
  void write(roo_display::Color* colors, uint32_t count) override {
    uint32_t consumed = 0;
    while (consumed < count) {
      primeRun();
      const uint32_t n = std::min(run_remaining_, count - consumed);
      if (run_visible_) downstream_->write(colors + consumed, n);
      advance(n);
      consumed += n;
    }
  }

  /// Fills visible stream runs while consuming excluded input positions.
  void fill(roo_display::Color color, uint32_t count) override {
    uint32_t consumed = 0;
    while (consumed < count) {
      primeRun();
      const uint32_t n = std::min(run_remaining_, count - consumed);
      if (run_visible_) downstream_->fill(color, n);
      advance(n);
      consumed += n;
    }
  }

  /// Filters colored sparse pixels before applying semantic exclusions.
  void writePixels(roo_display::BlendingMode mode, roo_display::Color* colors,
                   int16_t* x, int16_t* y, uint16_t count) override {
    uint16_t kept = 0;
    for (uint16_t i = 0; i < count; ++i) {
      if (!containsDamage(x[i], y[i])) continue;
      colors[kept] = colors[i];
      x[kept] = x[i];
      y[kept] = y[i];
      ++kept;
    }
    if (kept != 0) downstream_->writePixels(mode, colors, x, y, kept);
  }

  /// Filters monochrome sparse pixels before semantic exclusions.
  void fillPixels(roo_display::BlendingMode mode, roo_display::Color color,
                  int16_t* x, int16_t* y, uint16_t count) override {
    uint16_t kept = 0;
    for (uint16_t i = 0; i < count; ++i) {
      if (!containsDamage(x[i], y[i])) continue;
      x[kept] = x[i];
      y[kept] = y[i];
      ++kept;
    }
    if (kept != 0) downstream_->fillPixels(mode, color, x, y, kept);
  }

  /// Emits one batch of disjoint colored damage portions per input rectangle.
  void writeRects(roo_display::BlendingMode mode, roo_display::Color* colors,
                  int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      RectBatch batch;
      forEachVisible(
          roo_display::Box(x0[i], y0[i], x1[i], y1[i]),
          [&](const roo_display::Box& box) { batch.add(box, colors[i]); });
      if (batch.count != 0) {
        downstream_->writeRects(mode, batch.colors, batch.x0, batch.y0,
                                batch.x1, batch.y1, batch.count);
      }
    }
  }

  /// Emits one batch of disjoint filled damage portions per input rectangle.
  void fillRects(roo_display::BlendingMode mode, roo_display::Color color,
                 int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                 uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      RectBatch batch;
      forEachVisible(
          roo_display::Box(x0[i], y0[i], x1[i], y1[i]),
          [&](const roo_display::Box& box) { batch.add(box, color); });
      if (batch.count != 0) {
        downstream_->fillRects(mode, color, batch.x0, batch.y0, batch.x1,
                               batch.y1, batch.count);
      }
    }
  }

  /// Uses the downstream native color format for direct images.
  const ColorFormat& getColorFormat() const override {
    return output_.getColorFormat();
  }

  /// Preserves blending capability while forbidding unfiltered copies.
  const Capabilities& getCapabilities() const override { return capabilities_; }

  /// Tiles direct images through this filter's streamed output methods.
  void drawDirectRect(const roo::byte* data, size_t row_width_bytes,
                      int16_t src_x0, int16_t src_y0, int16_t src_x1,
                      int16_t src_y1, int16_t dst_x0, int16_t dst_y0) override {
    roo_display::DisplayOutput::drawDirectRect(
        data, row_width_bytes, src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0);
  }

 private:
  // At most nine Y bands can contain at most four disjoint X runs each.
  static constexpr int kMaxOutputRects =
      internal::DamageRegion::kCapacity *
      (2 * internal::DamageRegion::kCapacity + 1);

  struct RectBatch {
    roo_display::Color colors[kMaxOutputRects];
    int16_t x0[kMaxOutputRects];
    int16_t y0[kMaxOutputRects];
    int16_t x1[kMaxOutputRects];
    int16_t y1[kMaxOutputRects];
    uint16_t count = 0;

    void add(const roo_display::Box& box, roo_display::Color color) {
      colors[count] = color;
      x0[count] = box.xMin();
      y0[count] = box.yMin();
      x1[count] = box.xMax();
      y1[count] = box.yMax();
      ++count;
    }
  };

  struct Interval {
    int32_t first;
    int32_t end;
  };

  /// Sorts the bounded four-member interval list without a general sort.
  static void SortIntervals(Interval* values, int count) {
    for (int i = 1; i < count; ++i) {
      const Interval value = values[i];
      int j = i;
      while (j > 0 && values[j - 1].first > value.first) {
        values[j] = values[j - 1];
        --j;
      }
      values[j] = value;
    }
  }

  /// Sorts at most ten Y boundaries.
  static void SortBoundaries(int32_t* values, int count) {
    for (int i = 1; i < count; ++i) {
      const int32_t value = values[i];
      int j = i;
      while (j > 0 && values[j - 1] > value) {
        values[j] = values[j - 1];
        --j;
      }
      values[j] = value;
    }
  }

  /// Reports point membership without constructing complement rectangles.
  bool containsDamage(int16_t x, int16_t y) const {
    for (uint8_t i = 0; i < damage_.size(); ++i) {
      if (damage_.at(i).contains(x, y)) return true;
    }
    return false;
  }

  /// Caches disjoint visible X runs for one stable damage Y band.
  void prepareBand(int32_t y) {
    cached_band_end_ = window_.yMax();
    Interval active[internal::DamageRegion::kCapacity];
    int active_count = 0;
    for (uint8_t i = 0; i < damage_.size(); ++i) {
      const roo_display::Box& box = damage_.at(i);
      if (y < box.yMin()) {
        cached_band_end_ = std::min<int32_t>(cached_band_end_, box.yMin() - 1);
        continue;
      }
      if (y > box.yMax()) continue;
      cached_band_end_ = std::min<int32_t>(cached_band_end_, box.yMax());
      const int32_t first = std::max<int32_t>(window_.xMin(), box.xMin());
      const int32_t end = std::min<int32_t>(int32_t{window_.xMax()} + 1,
                                            int32_t{box.xMax()} + 1);
      if (first < end) active[active_count++] = {first, end};
    }
    SortIntervals(active, active_count);
    visible_count_ = 0;
    for (int i = 0; i < active_count; ++i) {
      if (visible_count_ != 0 &&
          active[i].first <= visible_intervals_[visible_count_ - 1].end) {
        visible_intervals_[visible_count_ - 1].end =
            std::max(visible_intervals_[visible_count_ - 1].end, active[i].end);
      } else {
        visible_intervals_[visible_count_++] = active[i];
      }
    }
  }

  /// Determines one safe streamed run, batching complete identical rows.
  void primeRun() {
    if (run_remaining_ != 0) return;
    if (cursor_y_ > cached_band_end_) prepareBand(cursor_y_);
    const int32_t row_remaining = window_.xMax() - cursor_x_ + 1;
    if (cursor_x_ == window_.xMin() &&
        (visible_count_ == 0 ||
         (visible_count_ == 1 &&
          visible_intervals_[0].first == window_.xMin() &&
          visible_intervals_[0].end == int32_t{window_.xMax()} + 1))) {
      run_visible_ = visible_count_ == 1;
      run_remaining_ = window_.width() * (cached_band_end_ - cursor_y_ + 1);
      if (run_visible_) {
        downstream_->setAddress(window_.xMin(), cursor_y_, window_.xMax(),
                                static_cast<int16_t>(cached_band_end_), mode_);
      }
      return;
    }
    run_visible_ = false;
    int32_t next = window_.xMax() + 1;
    for (int i = 0; i < visible_count_; ++i) {
      if (cursor_x_ < visible_intervals_[i].first) {
        next = visible_intervals_[i].first;
        break;
      }
      if (cursor_x_ < visible_intervals_[i].end) {
        run_visible_ = true;
        next = visible_intervals_[i].end;
        break;
      }
    }
    run_remaining_ = std::min<int32_t>(row_remaining, next - cursor_x_);
    if (run_visible_) {
      downstream_->setAddress(
          cursor_x_, cursor_y_,
          static_cast<int16_t>(cursor_x_ + run_remaining_ - 1), cursor_y_,
          mode_);
    }
  }

  /// advances the input cursor even when a run was entirely discarded.
  void advance(uint32_t count) {
    const uint32_t width = window_.width();
    const uint32_t offset = cursor_x_ - window_.xMin() + count;
    cursor_y_ += offset / width;
    cursor_x_ = window_.xMin() + offset % width;
    run_remaining_ -= count;
  }

  /// Sweeps damage members into disjoint rectangles before downstream output.
  template <typename Emit>
  void forEachVisible(const roo_display::Box& requested, Emit emit) const {
    if (requested.empty()) return;
    int32_t y_boundaries[2 * internal::DamageRegion::kCapacity + 2];
    int boundary_count = 0;
    y_boundaries[boundary_count++] = requested.yMin();
    y_boundaries[boundary_count++] = int32_t{requested.yMax()} + 1;
    for (uint8_t i = 0; i < damage_.size(); ++i) {
      const roo_display::Box clipped =
          roo_display::Box::Intersect(requested, damage_.at(i));
      if (clipped.empty()) continue;
      y_boundaries[boundary_count++] = clipped.yMin();
      y_boundaries[boundary_count++] = int32_t{clipped.yMax()} + 1;
    }
    SortBoundaries(y_boundaries, boundary_count);
    boundary_count = static_cast<int>(
        std::unique(y_boundaries, y_boundaries + boundary_count) -
        y_boundaries);

    roo_display::Box pending[internal::DamageRegion::kCapacity];
    int pending_count = 0;
    for (int band = 0; band + 1 < boundary_count; ++band) {
      const int32_t y = y_boundaries[band];
      const int32_t y_end = y_boundaries[band + 1] - 1;
      Interval intervals[internal::DamageRegion::kCapacity];
      int interval_count = 0;
      for (uint8_t i = 0; i < damage_.size(); ++i) {
        const roo_display::Box clipped =
            roo_display::Box::Intersect(requested, damage_.at(i));
        if (clipped.empty() || clipped.yMin() > y || clipped.yMax() < y) {
          continue;
        }
        intervals[interval_count++] = {clipped.xMin(),
                                       int32_t{clipped.xMax()} + 1};
      }
      SortIntervals(intervals, interval_count);
      roo_display::Box current[internal::DamageRegion::kCapacity];
      int current_count = 0;
      for (int i = 0; i < interval_count; ++i) {
        int32_t x0 = intervals[i].first;
        int32_t x_end = intervals[i].end;
        while (i + 1 < interval_count && intervals[i + 1].first <= x_end) {
          ++i;
          x_end = std::max(x_end, intervals[i].end);
        }
        current[current_count++] = roo_display::Box(
            static_cast<int16_t>(x0), static_cast<int16_t>(y),
            static_cast<int16_t>(x_end - 1), static_cast<int16_t>(y_end));
      }
      bool extend = current_count == pending_count && current_count != 0;
      for (int i = 0; extend && i < current_count; ++i) {
        extend = pending[i].xMin() == current[i].xMin() &&
                 pending[i].xMax() == current[i].xMax() &&
                 pending[i].yMax() + 1 == current[i].yMin();
      }
      if (!extend) {
        for (int i = 0; i < pending_count; ++i) emit(pending[i]);
        pending_count = current_count;
        for (int i = 0; i < current_count; ++i) pending[i] = current[i];
      } else {
        for (int i = 0; i < pending_count; ++i) {
          pending[i] = roo_display::Box(pending[i].xMin(), pending[i].yMin(),
                                        pending[i].xMax(), current[i].yMax());
        }
      }
    }
    for (int i = 0; i < pending_count; ++i) emit(pending[i]);
  }

  roo_display::DisplayOutput& output_;
  const internal::DamageRegion& damage_;
  internal::ExclusionFilter semantic_filter_;
  roo_display::DisplayOutput* downstream_;
  Capabilities capabilities_;
  roo_display::Box window_{0, 0, -1, -1};
  roo_display::BlendingMode mode_ = roo_display::BlendingMode::kSource;
  int16_t cursor_x_ = 0;
  int16_t cursor_y_ = 0;
  uint32_t run_remaining_ = 0;
  bool run_visible_ = false;
  int32_t cached_band_end_ = -1;
  Interval visible_intervals_[internal::DamageRegion::kCapacity];
  int visible_count_ = 0;
};

}  // namespace roo_windows::benchmarks
