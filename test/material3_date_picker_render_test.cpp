#include <algorithm>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"

namespace roo_windows::material3 {
namespace {
using roo_time::CivilDay;

class WriteCountingDevice final
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  WriteCountingDevice(int width, int height, roo::byte* raster)
      : OffscreenDevice(width, height, raster, roo_display::Argb4444()),
        width_(width),
        writes_(width * height, 0) {}

  void reset() { std::fill(writes_.begin(), writes_.end(), 0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(roo_display::Color* colors, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(roo_display::Color color, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::fill(color, count);
  }

  void writePixels(roo_display::BlendingMode mode, roo_display::Color* values,
                   int16_t* x, int16_t* y, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(roo_display::BlendingMode mode, roo_display::Color color,
                  int16_t* x, int16_t* y, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(roo_display::BlendingMode mode, roo_display::Color* values,
                  int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(roo_display::BlendingMode mode, roo_display::Color color,
                 int16_t* x0, int16_t* y0, int16_t* x1, int16_t* y1,
                 uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  uint16_t writesAt(int x, int y) const { return writes_[y * width_ + x]; }

 private:
  void countPixels(uint32_t count) {
    while (count-- != 0) {
      if (x_ < width_ && y_ < static_cast<int>(writes_.size() / width_)) {
        ++writes_[y_ * width_ + x_];
      }
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  int width_;
  std::vector<uint16_t> writes_;
  int x_ = 0;
  int y_ = 0;
  int left_ = 0;
  int right_ = 0;
};

class DatePickerRender : public testing::Test {
 protected:
  DatePickerRender()
      : raster_(400 * 576 * 2),
        device_(400, 576, raster_.data()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        content_(app_.context()),
        owner_(app_.addTaskFullScreen(content_)),
        picker_(app_.context()) {
    picker_.setValue(CivilDay::FromYmd(2024, 2, 14));
  }

  void open() {
    app_.refresh();
    ASSERT_EQ(PresentationStartResult::kStarted, picker_.open(owner_));
    device_.reset();
    app_.refresh();
  }

  internal::DatePickerPanel& panel() {
    return *static_cast<internal::DatePickerPanel*>(owner_.focus().scopeRoot());
  }

  internal::DatePickerHeader& header() {
    return *static_cast<internal::DatePickerHeader*>(
        static_cast<Widget&>(panel()).focusChildAt(0));
  }

  internal::DatePickerBody& body() {
    Widget* viewport = static_cast<Widget&>(panel()).focusChildAt(1);
    return *static_cast<internal::DatePickerBody*>(viewport->focusChildAt(0));
  }

  Color pixelAt(int x, int y) const {
    int16_t px = x;
    int16_t py = y;
    Color color;
    device_.raster().readColors(&px, &py, 1, &color);
    return color;
  }

  Color localPixel(const Widget& widget, int x, int y) const {
    XDim dx;
    YDim dy;
    widget.getAbsoluteOffset(dx, dy);
    return pixelAt(dx + x, dy + y);
  }

  void expectSingleWrites() const {
    int excess = 0;
    int written = 0;
    for (int y = 0; y < 576; ++y) {
      for (int x = 0; x < 400; ++x) {
        if (device_.writesAt(x, y) > 1) ++excess;
        if (device_.writesAt(x, y) != 0) ++written;
      }
    }
    EXPECT_EQ(0, excess);
    EXPECT_GT(written, 0);
  }

  std::vector<roo::byte> raster_;
  WriteCountingDevice device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment environment_;
  Application app_;
  Panel content_;
  Task& owner_;
  ModalDatePicker picker_;
};

// Verifies focus never adds a header-center halo or tints the whole day grid.
TEST_F(DatePickerRender, FocusFeedbackBelongsToIndividualControls) {
  open();
  const Color background = localPixel(panel(), 20, 4);
  EXPECT_EQ(background, localPixel(body(), 2, 2));
  const Rect next = header().controlBounds(3);
  header().onSingleTapUp(next.xMin() + next.width() / 2,
                         next.yMin() + next.height() / 2);
  app_.refresh();
  EXPECT_EQ(background,
            localPixel(header(), header().width() / 2, header().height() / 2));
  EXPECT_EQ(background, localPixel(body(), 2, 2));
}

// Verifies initial, selection-dirty, and invalidated repaint settle each pixel
// once; unchanged cells remain untouched by selection-only changes.
TEST_F(DatePickerRender, InitialDirtyAndInvalidatedPaintWritePixelsOnce) {
  open();
  expectSingleWrites();
  device_.reset();
  panel().selectDate(CivilDay::FromYmd(2024, 2, 15));
  app_.refresh();
  expectSingleWrites();
  XDim dx;
  YDim dy;
  body().getAbsoluteOffset(dx, dy);
  EXPECT_EQ(0, device_.writesAt(dx + 2, dy + 2));
  const std::vector<roo::byte> incremental = raster_;
  device_.reset();
  panel().invalidateInterior();
  app_.refresh();
  expectSingleWrites();
  EXPECT_EQ(incremental, raster_);
  EXPECT_EQ(1, device_.writesAt(dx + 2, dy + 2));
}
}  // namespace
}  // namespace roo_windows::material3
