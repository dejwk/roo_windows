#include <algorithm>
#include <cstring>
#include <memory>
#include <type_traits>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows::material3 {
namespace {

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

class ThemeSurface : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }
  ColorToken containerRole() const override { return ColorToken::kPrimary; }
  Color background() const override {
    return theme().material3Theme().color.primary;
  }
  bool isClickable() const override { return true; }
  void paint(PaintContext& ctx) const override { ctx.clear(); }
};

class DetachedRoot : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  void invalidateDescending() override {
    ++invalidations;
    Panel::invalidateDescending();
  }
  int invalidations = 0;
};

void SetInteraction(Widget& widget, int state) {
  widget.setHover(state == 0);
  widget.setFocused(state == 1);
  widget.setPressed(state == 2);
  widget.setEnabled(state != 3);
}

static_assert(std::is_copy_constructible<Theme>::value);
static_assert(std::is_move_constructible<Theme>::value);

// Verifies a palette update preserves stable borrowed addresses and custom
// component/density/interaction settings while framework colors resolve live.
TEST(Material3ApplicationThemeTest, PreservesIdentityAndCustomPolicy) {
  Material3Theme material;
  material.components.card.filledContainer = ColorToken::kTertiary;
  material.density = Density::kMinus2;
  Theme owner{.material3 = material};
  owner.framework.state.pressed = 128;
  owner.framework.state.disabledContentOpacity = 17;
  const Theme* theme = &owner;
  const Material3Theme* m3 = &owner.material3Theme();
  ColorScheme colors = DefaultColorScheme();
  colors.primary = Color(0xFF345678);
  colors.onPrimary = Color(0xFF876543);
  owner.material3Theme().color = colors;
  EXPECT_EQ(theme, &owner);
  EXPECT_EQ(m3, &owner.material3Theme());
  EXPECT_EQ(m3, &theme->material3Theme());
  EXPECT_EQ(ColorToken::kTertiary, m3->components.card.filledContainer);
  EXPECT_EQ(Density::kMinus2, m3->density);
  EXPECT_EQ(128, theme->framework.state.pressed);
  EXPECT_EQ(17, theme->disabledContentOpacity());
  EXPECT_EQ(Color(0xFF6750A4),
            theme->frameworkColor(FrameworkColorRole::kEmphasis));
  EXPECT_EQ(20, theme->framework.state.hover);
}

class EmptyKeys : public KeySource {
 public:
  int drain(KeyEvent*, int) override { return 0; }

 private:
  bool hasPendingEvents() const override { return false; }
};

// Verifies a palette update wakes a dormant running application; callers do
// not need an explicit refresh or layout request to display the new colors.
TEST(Material3ApplicationThemeTest, SchedulesRepaintWithoutLayout) {
  roo::byte raster[16 * 16 * 2]{};
  WriteCountingDevice device(16, 16, raster);
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Theme owner;
  Environment env(scheduler, owner);
  EmptyKeys keys;
  Application app(&env, display, keys, false);
  app.add(std::make_unique<ThemeSurface>(app.context()),
          roo_display::Box(0, 0, 7, 7));
  system_time_delay_micros(20000 - roo_time::Uptime::Now().inMicros() % 1000);
  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  ASSERT_FALSE(app.root().isDirty());
  ASSERT_EQ(roo_time::Uptime::Max(), scheduler.getNearestExecutionTime());
  device.reset();
  ColorScheme colors = DefaultColorScheme();
  colors.primary = Color(0xFF225566);
  owner.material3Theme().color = colors;
  app.invalidateTheme();
  ASSERT_FALSE(app.root().isLayoutRequested());
  ASSERT_NE(roo_time::Uptime::Max(), scheduler.getNearestExecutionTime());
  // Allow the existing frame throttle to elapse, using virtual test time.
  system_time_delay_micros(100000);
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 8);
  EXPECT_FALSE(app.root().isDirty());
  EXPECT_EQ(1, device.writesAt(0, 0));
  EXPECT_EQ(roo_time::Uptime::Max(), scheduler.getNearestExecutionTime());
  owner.material3Theme().density = Density::kMinus2;
  app.invalidateTheme(true);
  EXPECT_TRUE(app.root().isLayoutRequested());
}

// Verifies the existing startup prefill plus first paint, then single-write
// dirty and palette-invalidated frames. Updates reach two displays and a
// detached tree without layout. Hover,
// focus, press and disabled output must match a fresh application using the new
// palette.
TEST(Material3ApplicationThemeTest, RepaintsAllRootsWithLiveInteractionColors) {
  constexpr int kWidth = 16;
  constexpr int kHeight = 16;
  for (int state = 0; state < 4; ++state) {
    SCOPED_TRACE(state);
    roo::byte raster[kWidth * kHeight * 2]{};
    roo::byte second_raster[kWidth * kHeight * 2]{};
    roo::byte reference_raster[kWidth * kHeight * 2]{};
    WriteCountingDevice device(kWidth, kHeight, raster);
    roo_display::OffscreenDevice<roo_display::Argb4444> second_device(
        kWidth, kHeight, second_raster, roo_display::Argb4444());
    roo_display::OffscreenDevice<roo_display::Argb4444> reference_device(
        kWidth, kHeight, reference_raster, roo_display::Argb4444());
    roo_display::Display display(device);
    roo_display::Display second_display(second_device);
    roo_display::Display reference_display(reference_device);
    roo_scheduler::SchedulingService scheduler;
    Theme owner;
    Environment env(scheduler, owner);
    Application app(&env, display);
    Application second_app(&env, second_display);
    ThemeSurface surface(app.context());
    ThemeSurface second_surface(second_app.context());
    DetachedRoot detached(app.context());
    ThemeSurface detached_child(app.context());
    detached.add(detached_child);
    app.add(surface, roo_display::Box(0, 0, 7, 7));
    second_app.add(second_surface, roo_display::Box(0, 0, 7, 7));
    SetInteraction(surface, state);
    SetInteraction(second_surface, state);
    device.reset();
    app.refresh();
    second_app.refresh();
    // MainWindow performs a one-time canvas prefill before its first paint.
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) EXPECT_EQ(2, device.writesAt(x, y));
    }
    device.reset();
    surface.invalidateInterior();
    app.refresh();
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        EXPECT_EQ(x < 8 && y < 8 ? 1 : 0, device.writesAt(x, y));
      }
    }
    ASSERT_FALSE(app.root().isLayoutRequested());
    ASSERT_FALSE(surface.isLayoutRequested());
    const int detached_invalidations = detached.invalidations;
    ColorScheme colors = DefaultColorScheme();
    colors.primary = Color(0xFF225566);
    colors.onPrimary = Color(0xFFEE9911);
    colors.background = Color(0xFF112233);
    device.reset();
    owner.material3Theme().color = colors;
    app.invalidateTheme();
    second_app.invalidateTheme();
    detached.invalidateDescending();
    detached.invalidateInterior();
    EXPECT_GT(detached.invalidations, detached_invalidations);
    EXPECT_TRUE(detached_child.isInvalidated());
    EXPECT_TRUE(surface.isInvalidated());
    EXPECT_TRUE(second_surface.isInvalidated());
    EXPECT_FALSE(app.root().isLayoutRequested());
    EXPECT_FALSE(surface.isLayoutRequested());
    EXPECT_EQ(colors.onPrimary, detached_child.defaultColor());
    app.refresh();
    second_app.refresh();
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) EXPECT_EQ(1, device.writesAt(x, y));
    }
    EXPECT_EQ(0, std::memcmp(raster, second_raster, sizeof(raster)));
    Theme reference_theme{.material3 = {.color = colors}};
    Environment reference_env(scheduler, reference_theme);
    Application reference_app(&reference_env, reference_display);
    ThemeSurface reference_surface(reference_app.context());
    reference_app.add(reference_surface, roo_display::Box(0, 0, 7, 7));
    SetInteraction(reference_surface, state);
    reference_app.refresh();
    EXPECT_EQ(0, std::memcmp(raster, reference_raster, sizeof(raster)));
  }
}

}  // namespace
}  // namespace roo_windows::material3
