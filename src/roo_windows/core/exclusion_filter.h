#pragma once

#include "roo_display/core/buffered_drawing.h"
#include "roo_display/core/device.h"
#include "roo_windows/core/exclusion.h"

namespace roo_windows::internal {

using roo_display::BlendingMode;
using roo_display::BufferedRectFiller;
using roo_display::BufferedRectWriter;
using roo_display::Color;
using roo_display::DisplayOutput;

/// Filters ordinary output through rectangular and shared rounded exclusions.
/// The rectangle-only paths and streamed run batching are adapted from
/// roo_display/filter/clip_exclude_rects.h. Masked fills emit visible bands
/// directly, sharing constant spans through the straight middle of a mask.
class ExclusionFilter : public DisplayOutput {
 public:
  /// Creates an adapter suppressing pixels in @p exclusion before forwarding
  /// to @p output. Both borrowed objects must outlive this filter.
  ExclusionFilter(DisplayOutput& output, const ExclusionUnion* exclusion)
      : output_(&output),
        exclusion_(exclusion),
        address_window_(0, 0, 0, 0),
        cursor_x_(0),
        cursor_y_(0),
        capabilities_(output.getCapabilities().supportsBlending(),
                      /*supports_blit_copy=*/false) {}

  ~ExclusionFilter() override = default;

  /// Replace the underlying output.
  void setOutput(DisplayOutput& output) {
    output_ = &output;
    capabilities_ = Capabilities(output.getCapabilities().supportsBlending(),
                                 /*supports_blit_copy=*/false);
    resetRunState();
  }

  /// Opens a streamed window, skipping fully excluded runs.
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    address_window_ = Box(x0, y0, x1, y1);
    blending_mode_ = mode;
    cursor_x_ = x0;
    cursor_y_ = y0;
    resetRunState();
    if (!exclusion_->intersects(address_window_)) {
      run_excluded_ = false;
      run_remaining_ = static_cast<uint32_t>(address_window_.area());
    } else if (exclusion_->contains(address_window_)) {
      run_excluded_ = true;
      run_remaining_ = static_cast<uint32_t>(address_window_.area());
    }
  }

  /// Forwards visible streamed colors and consumes excluded colors.
  void write(Color* color, uint32_t pixel_count) override {
    // Route the address window through cached runs. Row starts may promote a
    // row-local answer to a multi-line batch when the same state persists.
    uint32_t i = 0;
    while (i < pixel_count) {
      primeRun();
      uint32_t run = pixel_count - i;
      if (run > run_remaining_) run = run_remaining_;

      if (!run_excluded_) {
        output_->write(color + i, run);
      }
      consumeRun(run);
      i += run;
    }
  }

  /// Fills only visible portions of the current streamed window.
  void fill(Color color, uint32_t pixel_count) override {
    // Route the address window through cached runs. Row starts may promote a
    // row-local answer to a multi-line batch when the same state persists.
    uint32_t i = 0;
    while (i < pixel_count) {
      primeRun();
      uint32_t run = pixel_count - i;
      if (run > run_remaining_) run = run_remaining_;

      if (!run_excluded_) {
        output_->fill(color, run);
      }
      consumeRun(run);
      i += run;
    }
  }

  /// Filters colored rectangles while retaining buffered output order.
  void writeRects(BlendingMode mode, Color* color, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    BufferedRectWriter writer(*output_, mode);
    while (count-- > 0) {
      if (exclusion_->intersectsMaskedBounds(Box(*x0, *y0, *x1, *y1))) {
        roo_display::BufferedRectWriterFillAdapter<BufferedRectWriter> filler(
            writer, *color);
        fillMaskedRect(Box(*x0, *y0, *x1, *y1), &filler);
      } else {
        writeRect(*color, *x0, *y0, *x1, *y1, 0, &writer);
      }
      ++color;
      ++x0;
      ++y0;
      ++x1;
      ++y1;
    }
  }

  /// Fills visible rectangle bands without retaining their fragments.
  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    BufferedRectFiller filler(*output_, color, mode);
    while (count-- > 0) {
      if (exclusion_->intersectsMaskedBounds(Box(*x0, *y0, *x1, *y1))) {
        fillMaskedRect(Box(*x0, *y0, *x1, *y1), &filler);
      } else {
        fillRect(*x0, *y0, *x1, *y1, 0, &filler);
      }
      ++x0;
      ++y0;
      ++x1;
      ++y1;
    }
  }

  /// Compacts sparse colors and coordinates to their visible subset.
  void writePixels(BlendingMode mode, Color* color, int16_t* x, int16_t* y,
                   uint16_t pixel_count) override {
    int16_t* x_out = x;
    int16_t* y_out = y;
    Color* color_out = color;
    uint16_t new_pixel_count = 0;
    for (uint16_t i = 0; i < pixel_count; ++i) {
      if (!exclusion_->contains(x[i], y[i])) {
        *x_out++ = x[i];
        *y_out++ = y[i];
        *color_out++ = color[i];
        new_pixel_count++;
      }
    }
    if (new_pixel_count > 0) {
      output_->writePixels(mode, color, x, y, new_pixel_count);
    }
  }

  /// Compacts sparse coordinates to their visible subset.
  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t pixel_count) override {
    int16_t* x_out = x;
    int16_t* y_out = y;
    uint16_t new_pixel_count = 0;
    for (uint16_t i = 0; i < pixel_count; ++i) {
      if (!exclusion_->contains(x[i], y[i])) {
        *x_out++ = x[i];
        *y_out++ = y[i];
        new_pixel_count++;
      }
    }
    if (new_pixel_count > 0) {
      output_->fillPixels(mode, color, x, y, new_pixel_count);
    }
  }

  /// Uses the underlying output color format.
  const ColorFormat& getColorFormat() const override {
    return output_->getColorFormat();
  }

  /// Reports blending support while disabling unfiltered blit copies.
  const Capabilities& getCapabilities() const override { return capabilities_; }

  /// Routes direct image drawing through the filtered output methods.
  void drawDirectRect(const roo::byte* data, size_t row_width_bytes,
                      int16_t src_x0, int16_t src_y0, int16_t src_x1,
                      int16_t src_y1, int16_t dst_x0, int16_t dst_y0) override {
    DisplayOutput::drawDirectRect(data, row_width_bytes, src_x0, src_y0, src_x1,
                                  src_y1, dst_x0, dst_y0);
  }

 private:
  void resetRunState() {
    run_remaining_ = 0;
    run_excluded_ = false;
    visible_window_open_ = false;
  }

  uint32_t rowRemaining() const {
    return static_cast<uint32_t>(address_window_.xMax() - cursor_x_ + 1);
  }

  /// Ensure a cached run is available and, for visible runs, open the widest
  /// safe address window on the wrapped output.
  void primeRun() {
    uint32_t row_remaining = rowRemaining();
    if (run_remaining_ == 0) {
      size_t same_count = 1;
      run_excluded_ = exclusion_->contains(cursor_x_, cursor_y_, &same_count);
      run_remaining_ = row_remaining;
      if (same_count < run_remaining_) {
        run_remaining_ = static_cast<uint32_t>(same_count);
      } else if (cursor_x_ == address_window_.xMin()) {
        uint32_t batch = run_excluded_ ? exclusion_->excludedPixelsFromRowStart(
                                             address_window_, cursor_y_)
                                       : exclusion_->visiblePixelsFromRowStart(
                                             address_window_, cursor_y_);
        if (batch > run_remaining_) {
          run_remaining_ = batch;
        }
      }
      if (run_excluded_) {
        visible_window_open_ = false;
      }
    }
    if (!run_excluded_ && !visible_window_open_) {
      uint16_t y1 = cursor_y_;
      if (cursor_x_ == address_window_.xMin() &&
          run_remaining_ > row_remaining) {
        uint32_t width = static_cast<uint32_t>(address_window_.width());
        uint32_t rows_spanned = (run_remaining_ + width - 1) / width;
        y1 = static_cast<uint16_t>(cursor_y_ + rows_spanned - 1);
      }
      output_->setAddress(cursor_x_, cursor_y_, address_window_.xMax(), y1,
                          blending_mode_);
      visible_window_open_ = true;
    }
  }

  void consumeRun(uint32_t pixel_count) {
    advanceCursor(pixel_count);
    run_remaining_ -= pixel_count;
    if (run_remaining_ == 0) {
      visible_window_open_ = false;
    }
  }

  void writeRect(Color color, int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                 int mask_idx, BufferedRectWriter* writer) {
    Box rect(x0, y0, x1, y1);
    while (mask_idx < (int)exclusion_->size() &&
           !exclusion_->at(mask_idx).intersects(rect)) {
      ++mask_idx;
    }
    if (mask_idx == (int)exclusion_->size()) {
      writer->writeRect(x0, y0, x1, y1, color);
      return;
    }
    Box intruder = Box::Intersect(exclusion_->at(mask_idx), rect);
    if (intruder.yMin() > y0) {
      writeRect(color, x0, y0, x1, intruder.yMin() - 1, mask_idx + 1, writer);
      y0 = intruder.yMin();
    }
    if (intruder.xMin() > x0) {
      writeRect(color, x0, y0, intruder.xMin() - 1, intruder.yMax(),
                mask_idx + 1, writer);
    }
    if (intruder.xMax() < x1) {
      writeRect(color, intruder.xMax() + 1, y0, x1, intruder.yMax(),
                mask_idx + 1, writer);
    }
    if (intruder.yMax() < y1) {
      writeRect(color, x0, intruder.yMax() + 1, x1, y1, mask_idx + 1, writer);
    }
  }

  void fillRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int mask_idx,
                BufferedRectFiller* filler) {
    {
      Box rect(x0, y0, x1, y1);
      while (mask_idx < (int)exclusion_->size() &&
             !exclusion_->at(mask_idx).intersects(rect)) {
        ++mask_idx;
      }
    }
    if (mask_idx == (int)exclusion_->size()) {
      filler->fillRect(x0, y0, x1, y1);
      return;
    }
    Box intruder =
        Box::Intersect(exclusion_->at(mask_idx), Box(x0, y0, x1, y1));
    if (intruder.yMin() > y0) {
      fillRect(x0, y0, x1, intruder.yMin() - 1, mask_idx + 1, filler);
      y0 = intruder.yMin();
    }
    if (intruder.xMin() > x0) {
      fillRect(x0, y0, intruder.xMin() - 1, intruder.yMax(), mask_idx + 1,
               filler);
    }
    if (intruder.xMax() < x1) {
      fillRect(intruder.xMax() + 1, y0, x1, intruder.yMax(), mask_idx + 1,
               filler);
    }
    if (intruder.yMax() < y1) {
      fillRect(x0, intruder.yMax() + 1, x1, y1, mask_idx + 1, filler);
    }
  }

  // Emits the complement of the union one constant vertical band at a time.
  // The output writer batches rectangles; no fragment list is retained.
  template <typename Filler>
  void fillMaskedRect(const Box& bounds, Filler* filler) {
    for (int32_t y = bounds.yMin(); y <= bounds.yMax();) {
      const int16_t last_y = exclusion_->bandEnd(bounds, y);
      for (int32_t x = bounds.xMin(); x <= bounds.xMax();) {
        size_t count;
        const bool excluded = exclusion_->contains(x, y, &count);
        const int16_t last_x =
            x + std::min<size_t>(count, bounds.xMax() - x + 1) - 1;
        if (!excluded) filler->fillRect(x, y, last_x, last_y);
        x = int32_t(last_x) + 1;
      }
      y = int32_t(last_y) + 1;
    }
  }

  void advanceCursor(uint32_t pixel_count) {
    int16_t w = address_window_.xMax() - address_window_.xMin() + 1;
    int32_t pos = (cursor_y_ - address_window_.yMin()) * (int32_t)w +
                  (cursor_x_ - address_window_.xMin()) + pixel_count;
    cursor_y_ = address_window_.yMin() + pos / w;
    cursor_x_ = address_window_.xMin() + pos % w;
  }

  DisplayOutput* output_;
  const ExclusionUnion* exclusion_;
  Box address_window_;
  BlendingMode blending_mode_;
  int16_t cursor_x_;
  int16_t cursor_y_;
  uint32_t run_remaining_ = 0;
  bool run_excluded_ = false;
  bool visible_window_open_ = false;
  Capabilities capabilities_;
};

}  // namespace roo_windows::internal
