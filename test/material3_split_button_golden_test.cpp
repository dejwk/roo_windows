#include <algorithm>
#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "material3_density_acceptance_theme.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/split_button.h"

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

class GoldenPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

// Captures a complete split surface with the borrowed menu in its requested
// state, including the ancestor backdrop visible through the separation.
void CheckSplit(const char* name, SplitButtonVariant variant, bool open,
                bool dark = false) {
  constexpr int kWidth = 320;
  constexpr int kHeight = 240;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      kWidth, kHeight, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Material3Theme material = test_support::MakeAcceptanceTheme(dark);
  Theme theme{MakeFrameworkTheme(material), &material};
  Environment environment(scheduler, theme);
  Application app(&environment, display);
  GoldenPanel panel(app.context());
  app.addTaskFullScreen(panel);
  StandardMenuItem item(StandardMenuItemInit{"Run once"});
  MenuEntry row(app.context());
  row.setMenuItem(item);
  MenuGroup group(app.context());
  group.add(row);
  Menu menu(app.context());
  menu.addGroup(group);
  SplitButton button(app.context(), menu, "Run", variant);
  Dimensions natural =
      button.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  panel.add(WidgetRef(button),
            Rect(32, 32, 31 + natural.width(), 31 + natural.height()));
  app.refresh();
  if (open) {
    ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
    app.refresh();
  }
  EXPECT_TRUE(::roo_windows::test::CompareOrUpdateGolden(
      ::roo_windows::test::CaptureRgb(device.raster(), 0, 0, kWidth, kHeight),
      std::string("test/goldens/material3_split_button/") + name +
          (ROO_WINDOWS_ZOOM == 100 ? ""
                                   : "_" + std::to_string(ROO_WINDOWS_ZOOM)) +
          ".ppm",
      name));
  button.dismissMenu();
  panel.removeLast();
  menu.clearGroups();
  group.clear();
}

// Verifies independent filled segments and a visible gap against a reference.
TEST(SplitButtonGolden, ClosedFilled) {
  CheckSplit("closed_filled", SplitButtonVariant::kFilled, false);
}

// Verifies open trailing geometry and anchored menu placement against a
// reference, including the upward chevron and unchanged primary segment.
TEST(SplitButtonGolden, OpenOutlined) {
  CheckSplit("open_outlined", SplitButtonVariant::kOutlined, true);
}

TEST(SplitButtonGolden, ClosedFilledDark) {
  CheckSplit("closed_filled_dark", SplitButtonVariant::kFilled, false, true);
}

TEST(SplitButtonGolden, OpenOutlinedDark) {
  CheckSplit("open_outlined_dark", SplitButtonVariant::kOutlined, true, true);
}

// Verifies foreground and decoration paths write each split-control pixel at
// most once during initial rendering, including its transparent gap.
TEST(SplitButtonGolden, ForegroundFirstAvoidsIntermediatePixelWrites) {
  constexpr int kWidth = 240;
  constexpr int kHeight = 120;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  WriteCountingDevice device(kWidth, kHeight, raster.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  GoldenPanel panel(app.context());
  app.addTaskFullScreen(panel);
  Menu menu(app.context());
  SplitButton button(app.context(), menu, "Run");
  panel.add(WidgetRef(button), Rect(32, 32, 181, 71));
  app.refresh();
  device.reset();
  button.invalidateInterior();
  app.refresh();
  int excess = 0;
  for (int y = 32; y <= 71; ++y) {
    for (int x = 32; x <= 181; ++x) {
      if (device.writesAt(x, y) > 1) ++excess;
    }
  }
  EXPECT_EQ(0, excess);
  panel.removeLast();
}

// Verifies opening and closing the borrowed menu leave display corners outside
// the split control and menu footprint unwritten.
TEST(SplitButtonGolden, MenuPresentationKeepsDisplayDamageLocal) {
  constexpr int kWidth = 240;
  constexpr int kHeight = 160;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  WriteCountingDevice device(kWidth, kHeight, raster.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  GoldenPanel panel(app.context());
  app.addTaskFullScreen(panel);
  StandardMenuItem item(StandardMenuItemInit{"Run once"});
  MenuEntry row(app.context());
  row.setMenuItem(item);
  MenuGroup group(app.context());
  group.add(row);
  Menu menu(app.context());
  menu.addGroup(group);
  SplitButton button(app.context(), menu, "Run");
  panel.add(WidgetRef(button), Rect(32, 32, 181, 71));
  app.refresh();

  device.reset();
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  app.refresh();
  EXPECT_EQ(0, device.writesAt(0, 0));
  EXPECT_EQ(0, device.writesAt(kWidth - 1, 0));
  EXPECT_EQ(0, device.writesAt(0, kHeight - 1));
  EXPECT_EQ(0, device.writesAt(kWidth - 1, kHeight - 1));

  device.reset();
  button.dismissMenu();
  app.refresh();
  EXPECT_EQ(0, device.writesAt(0, 0));
  EXPECT_EQ(0, device.writesAt(kWidth - 1, 0));
  EXPECT_EQ(0, device.writesAt(0, kHeight - 1));
  EXPECT_EQ(0, device.writesAt(kWidth - 1, kHeight - 1));
  panel.removeLast();
  menu.clearGroups();
  group.clear();
}

// Verifies moving an elevated split button restores its old shadow and matches
// a full repaint of the new scene, including pixels outside both segments.
TEST(SplitButtonGolden, MovingElevatedControlRestoresOldShadow) {
  constexpr int kWidth = 240;
  constexpr int kHeight = 120;
  std::vector<roo::byte> raster(kWidth * kHeight * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      kWidth, kHeight, raster.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  GoldenPanel panel(app.context());
  app.addTaskFullScreen(panel);
  Menu menu(app.context());
  SplitButton button(app.context(), menu, "Run", SplitButtonVariant::kElevated);
  panel.add(WidgetRef(button), Rect(16, 16, 165, 55));
  app.refresh();
  button.measure(WidthSpec::Exactly(150), HeightSpec::Exactly(40));
  button.layout(Rect(40, 60, 189, 99));
  app.refresh();
  const std::vector<roo::byte> incremental = raster;
  app.root().invalidateDescending();
  app.refresh();
  EXPECT_EQ(incremental, raster);
  panel.removeLast();
}

}  // namespace
}  // namespace roo_windows::material3
