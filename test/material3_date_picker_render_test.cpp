#include <algorithm>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/ui/text_label.h"
#include "roo_testing/system/timer.h"
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

template <int Width, int Height>
class DatePickerRenderSized : public testing::Test {
 protected:
  DatePickerRenderSized()
      : raster_(Width * Height * 2),
        device_(Width, Height, raster_.data()),
        display_(device_),
        environment_(scheduler_, theme_),
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

  void expectSingleWrites(bool expect_output = true) const {
    int excess = 0;
    int written = 0;
    for (int y = 0; y < Height; ++y) {
      for (int x = 0; x < Width; ++x) {
        if (device_.writesAt(x, y) > 1) ++excess;
        if (device_.writesAt(x, y) != 0) ++written;
      }
    }
    EXPECT_EQ(0, excess);
    if (expect_output) EXPECT_GT(written, 0);
  }

  void finishFeedback() {
    system_time_delay_micros((theme_.framework.click.durationMillis + 50) *
                             1000);
    for (int i = 0; i < 2; ++i) {
      device_.reset();
      app_.refresh();
      expectSingleWrites(false);
    }
  }

  void expectFittedCalendar() {
    EXPECT_TRUE(picker_.shrinkToFit());
    open();
    expectSingleWrites();
    Widget* viewport = body().parent();
    ASSERT_LE(body().width(), viewport->width());
    ASSERT_LE(body().height(), viewport->height());
    EXPECT_EQ(0, body().offsetLeft());
    EXPECT_EQ(0, body().offsetTop());
    EXPECT_EQ(body().width() - 1, body().cellBounds(41).xMax());
    EXPECT_EQ(body().height() - 1, body().cellBounds(41).yMax());
    for (int i = 0; i < 42; ++i) {
      const CivilDay day = internal::GridDay(
          panel().session().month, panel().session().firstWeekday(), i);
      if (day.month() != panel().session().month.month()) continue;
      const Rect cell = body().cellBounds(i);
      // Integer division must assign both edges to this cell, including
      // columns and rows that receive the remainder pixel.
      for (int x : {cell.xMin(), cell.xMax()}) {
        for (int y : {cell.yMin(), cell.yMax()}) {
          device_.reset();
          body().onSingleTapUp(x, y);
          app_.refresh();
          EXPECT_EQ(day, panel().session().draft);
          expectSingleWrites(false);
        }
      }
    }
    finishFeedback();
    const std::vector<roo::byte> incremental = raster_;
    device_.reset();
    panel().invalidateInterior();
    app_.refresh();
    expectSingleWrites();
    EXPECT_EQ(incremental, raster_);
  }

  std::vector<roo::byte> raster_;
  WriteCountingDevice device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_;
  Environment environment_;
  Application app_;
  Panel content_;
  Task& owner_;
  ModalDatePicker picker_;
};

using DatePickerRender = DatePickerRenderSized<400, 576>;
using DatePickerPortrait = DatePickerRenderSized<240, 320>;
using DatePickerLandscape = DatePickerRenderSized<320, 240>;
using DatePickerMinimum = DatePickerRenderSized<240, 240>;
using DatePickerUneven = DatePickerRenderSized<257, 321>;
using DatePickerTiny = DatePickerRenderSized<200, 200>;

// Verifies a portrait calendar fits and all cell edges select the right date.
TEST_F(DatePickerPortrait, FittedCalendar) { expectFittedCalendar(); }

// Verifies compact chrome leaves room for all six weeks in landscape.
TEST_F(DatePickerLandscape, FittedCalendar) { expectFittedCalendar(); }

// Verifies the smallest admitted display still paints all calendar rows once.
TEST_F(DatePickerMinimum, FittedCalendar) { expectFittedCalendar(); }

// Verifies remainder pixels in fitted cell dimensions keep correct hit tests.
TEST_F(DatePickerUneven, FittedCalendar) { expectFittedCalendar(); }

// Verifies tighter shared density admits and fits a 200 px display.
TEST_F(DatePickerTiny, TightDensityFitsTinyDisplay) {
  theme_.material3Theme().density = Density::kMinus5;
  expectFittedCalendar();
}

// Verifies all density levels change calendar spacing through the live theme.
TEST_F(DatePickerRender, SharedDensityControlsCalendarGeometry) {
  picker_.setShrinkToFit(false);
  open();
  for (int level = 0; level >= -5; --level) {
    theme_.material3Theme().density = static_cast<Density>(level);
    app_.invalidateTheme(true);
    device_.reset();
    app_.refresh();
    EXPECT_EQ(Scaled(48 + 4 * level), body().cellBounds(0).width());
    EXPECT_EQ(Scaled(48 + 4 * level), body().cellBounds(0).height());
    expectSingleWrites();
  }
}

// Verifies opting out retains the original full-size scrolling day grid.
TEST_F(DatePickerPortrait, FittingCanBeDisabled) {
  picker_.setShrinkToFit(false);
  EXPECT_FALSE(picker_.shrinkToFit());
  open();
  EXPECT_EQ(Scaled(336), body().width());
  EXPECT_EQ(Scaled(336), body().height());
  EXPECT_EQ(Scaled(48), body().cellBounds(0).width());
  EXPECT_EQ(Scaled(48), body().cellBounds(0).height());
  EXPECT_GT(body().width(), body().parent()->width());
  EXPECT_GT(body().height(), body().parent()->height());
}

// Verifies disabled and adjacent-month taps leave selection, focus, and pixels
// unchanged, including the would-be cursor outline.
TEST_F(DatePickerRender, DisabledDatesDoNotAcquireFeedback) {
  picker_.setBounds(
      {CivilDay::FromYmd(2024, 2, 10), CivilDay::FromYmd(2024, 2, 20)});
  open();
  const std::vector<roo::byte> before = raster_;
  const CivilDay selected = panel().session().draft;
  for (int i = 0; i < 42; ++i) {
    const CivilDay day = internal::GridDay(panel().session().month,
                                           panel().session().firstWeekday(), i);
    if (day.month() == 2 && picker_.bounds().contains(day)) continue;
    const Rect cell = body().cellBounds(i);
    device_.reset();
    body().onSingleTapUp(cell.xMin() + cell.width() / 2,
                         cell.yMin() + cell.height() / 2);
    app_.refresh();
    EXPECT_EQ(selected, panel().session().draft);
    EXPECT_EQ(&body(), owner_.focus().focused());
    EXPECT_EQ(before, raster_);
    EXPECT_FALSE(body().isClicking());
    EXPECT_FALSE(body().isPressed());
    EXPECT_EQ(0, device_.writesAt(200, 200));
  }
}

// Verifies a month with no enabled dates has no initial purple cursor marker.
TEST_F(DatePickerRender, UnavailableMonthHasNoCursorMarker) {
  picker_.setBounds(
      {CivilDay::FromYmd(2024, 3, 1), CivilDay::FromYmd(2024, 3, 31)});
  open();
  EXPECT_FALSE(panel().session().draft.isValid());
  const Color primary = body().theme().material3Theme().color.primary;
  roo_display::Argb4444 mode;
  const Color marker = mode.toArgbColor(mode.fromArgbColor(primary));
  int primary_pixels = 0;
  for (int y = 0; y < body().height(); ++y) {
    for (int x = 0; x < body().width(); ++x) {
      if (localPixel(body(), x, y) == marker) ++primary_pixels;
    }
  }
  EXPECT_EQ(0, primary_pixels);
  expectSingleWrites();
}

// Verifies day clicks share the core timeline, animate only changed cells,
// settle each pixel once, and end at the same image as explicit invalidation.
TEST_F(DatePickerRender, DayRippleAndSelectionTransitionStayWithinCells) {
  open();
  const Rect target = body().cellBounds(18);  // February 15, 2024.
  device_.reset();
  body().onSingleTapUp(target.xMin() + target.width() / 2,
                       target.yMin() + target.height() / 2);
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 15), panel().session().draft);
  ASSERT_NE(nullptr, body().getClickAnimation());
  app_.refresh();
  expectSingleWrites();
  const std::vector<roo::byte> start = raster_;
  system_time_lag_ns(80000000);
  device_.reset();
  app_.refresh();
  expectSingleWrites();
  EXPECT_NE(start, raster_);
  XDim dx;
  YDim dy;
  body().getAbsoluteOffset(dx, dy);
  EXPECT_EQ(0, device_.writesAt(dx + 2, dy + 2));
  EXPECT_EQ(0, device_.writesAt(dx + 2, dy + body().cellBounds(0).yMin() + 2));
  finishFeedback();
  EXPECT_FALSE(body().isClicking());
  EXPECT_EQ(nullptr, body().getClickAnimation());
  const std::vector<roo::byte> settled = raster_;
  device_.reset();
  panel().invalidateInterior();
  app_.refresh();
  expectSingleWrites();
  EXPECT_EQ(settled, raster_);
}

// Verifies distant selection transitions preserve every intervening cell.
TEST_F(DatePickerRender, DistantSelectionPreservesInterveningDates) {
  open();
  const Rect target = body().cellBounds(31);  // February 28, 2024.
  const Rect between = body().cellBounds(24);
  XDim dx;
  YDim dy;
  body().getAbsoluteOffset(dx, dy);
  body().onSingleTapUp(target.xMin() + 4, target.yMin() + 4);
  app_.refresh();
  system_time_lag_ns(80000000);
  device_.reset();
  app_.refresh();
  expectSingleWrites();
  for (int y = between.yMin(); y <= between.yMax(); ++y) {
    for (int x = between.xMin(); x <= between.xMax(); ++x)
      EXPECT_EQ(0, device_.writesAt(dx + x, dy + y));
  }
  finishFeedback();
}

// Verifies a held press does not select early and leaving for another cell
// cancels selection; quick repeated taps and keyboard activation remain usable.
TEST_F(DatePickerRender, PressCancellationRetappingAndKeyboardFeedback) {
  open();
  const Rect first = body().cellBounds(18);
  const Rect second = body().cellBounds(19);
  const CivilDay original = panel().session().draft;
  body().onShowPress(first.xMin() + 4, first.yMin() + 4);
  app_.refresh();
  EXPECT_TRUE(body().isPressed());
  EXPECT_EQ(original, panel().session().draft);
  body().onSingleTapUp(second.xMin() + 4, second.yMin() + 4);
  EXPECT_EQ(original, panel().session().draft);
  EXPECT_FALSE(body().isPressed());
  EXPECT_FALSE(body().isClicking());
  body().onSingleTapUp(first.xMin() + 4, first.yMin() + 4);
  body().onSingleTapUp(second.xMin() + 4, second.yMin() + 4);
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 16), panel().session().draft);
  body().onKeyEvent({KeyPhase::kDown, KeyCode::kLeft, 0, 0});
  body().onKeyEvent({KeyPhase::kDown, KeyCode::kEnter, 0, 0});
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 15), panel().session().draft);
  EXPECT_TRUE(body().isClicking());
  finishFeedback();
}

// Verifies disabled dates cannot start held feedback and navigation retires
// day feedback without leaving selected-day artifacts in the next month.
TEST_F(DatePickerRender, DisabledPressAndMonthNavigationCannotLeaveRipple) {
  picker_.setBounds(
      {CivilDay::FromYmd(2024, 2, 10), CivilDay::FromYmd(2024, 2, 20)});
  open();
  const Rect disabled = body().cellBounds(4);
  body().onShowPress(disabled.xMin() + 4, disabled.yMin() + 4);
  EXPECT_FALSE(body().isPressed());
  EXPECT_FALSE(body().isClicking());
  const Rect enabled = body().cellBounds(18);
  body().onSingleTapUp(enabled.xMin() + 4, enabled.yMin() + 4);
  const Rect next = header().controlBounds(3);
  header().onSingleTapUp(next.xMin() + 4, next.yMin() + 4);
  EXPECT_FALSE(body().isClicking());
  EXPECT_EQ(CivilDay::FromYmd(2024, 3, 1), panel().session().month);
  EXPECT_TRUE(header().isClicking());
  app_.refresh();
  finishFeedback();
  const std::vector<roo::byte> settled = raster_;
  panel().invalidateInterior();
  app_.refresh();
  EXPECT_EQ(settled, raster_);
}

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

// Verifies all supported month names and four-digit years fit compact labels.
TEST_F(DatePickerRender, HeaderLabelsFitCompactWidth) {
  open();
  header().measure(WidthSpec::Exactly(Scaled(208)),
                   HeightSpec::Exactly(Scaled(104)));
  header().layout(Rect(0, 0, Scaled(208) - 1, Scaled(104) - 1));
  const TextStyle& style = header().navigationTextStyle();
  for (int i = 0; i < 12; ++i) {
    const roo::string_view month = header().monthLabel(i);
    EXPECT_EQ(panel().session().locale().month_standalone_abbreviated[i],
              month);
    roo_display::StringViewLabel label(month, style.font(), Color(),
                                       style.fontOptions());
    EXPECT_LE(label.anchorExtents().width(), header().controlBounds(1).width())
        << std::string(month.data(), month.size());
  }
  roo_display::StringViewLabel year("9999", style.font(), Color(),
                                    style.fontOptions());
  EXPECT_LE(year.anchorExtents().width(), header().controlBounds(2).width());
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
