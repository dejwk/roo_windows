#include "examples/material3/theme/coordinated_palette/palette_preview.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/task.h"

namespace roo_windows::coordinated_palette_example {
namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Color;

constexpr int kWidth = 320;
constexpr int kHeight = 240;

class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* pixels)
      : OffscreenDevice(kWidth, kHeight, pixels, Argb8888()) {}

  void reset() { writes.fill(0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    record(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(Color color, uint32_t count) override {
    record(count);
    OffscreenDevice::fill(color, count);
  }

  void writePixels(BlendingMode mode, Color* colors, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(colors + i, 1);
    }
  }

  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* colors, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(colors[i], (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  void blitCopy(int16_t src_x0, int16_t src_y0, int16_t src_x1, int16_t src_y1,
                int16_t dst_x0, int16_t dst_y0) override {
    for (int y = dst_y0; y <= dst_y0 + src_y1 - src_y0; ++y) {
      for (int x = dst_x0; x <= dst_x0 + src_x1 - src_x0; ++x) {
        ++writes[y * kWidth + x];
      }
    }
    OffscreenDevice::blitCopy(src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0);
  }

  std::array<uint8_t, kWidth * kHeight> writes{};

 private:
  void record(uint32_t count) {
    while (count-- > 0) {
      if (x_ >= 0 && x_ < kWidth && y_ >= 0 && y_ < kHeight) {
        ++writes[y_ * kWidth + x_];
      }
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  int left_ = 0;
  int right_ = 0;
  int x_ = 0;
  int y_ = 0;
};

class PalettePreviewTest : public testing::Test {
 protected:
  PalettePreviewTest()
      : device_(pixels_),
        display_(device_),
        environment_(scheduler_, theme_),
        app_(&environment_, display_) {
    // Initialize the display before measuring the preview's first paint.
    app_.refresh();
    auto screen = std::make_unique<PalettePreview>(app_, theme_);
    screen_ = screen.get();
    storage_ = std::move(screen);
    owner_ = &app_.addTaskFullScreen(*screen_);
  }

  ~PalettePreviewTest() override {
    owner_->navigation().clear();
    storage_.reset();
  }

  // Checks actual display writes, including sparse output and ancestor fills.
  void singleWriteFrame(const char* phase = "full") {
    device_.reset();
    app_.refresh();
    const auto worst =
        std::max_element(device_.writes.begin(), device_.writes.end());
    const int offset = static_cast<int>(worst - device_.writes.begin());
    EXPECT_EQ(1, *worst) << phase << " at " << offset % kWidth << ","
                         << offset / kWidth;
  }

  // A forced full repaint must settle the same pixels as the incremental one.
  void expectFullRepaintMatches() {
    const std::vector<roo::byte> incremental(pixels_,
                                             pixels_ + sizeof(pixels_));
    app_.invalidateTheme();
    singleWriteFrame();
    EXPECT_TRUE(std::equal(incremental.begin(), incremental.end(), pixels_));
  }

  void chooseNextPreset() {
    ASSERT_EQ(material3::MenuShowResult::kShown,
              screen_->presetSelector().showMenu());
    app_.refresh();
    Widget* focused = owner_->focus().focused();
    ASSERT_NE(nullptr, focused);
    ASSERT_TRUE(focused->onKeyEvent(
        KeyEvent{KeyPhase::kDown, KeyCode::kDown, 0, PhysicalKey::kNone, 0}));
    focused = owner_->focus().focused();
    ASSERT_NE(nullptr, focused);
    app_.refresh();
    // A long catalog must reveal the focused row, not merely commit an
    // offscreen item when the test invokes its callback directly.
    Rect full;
    Rect visible;
    focused->getAbsoluteBounds(full, visible);
    ASSERT_GT(visible.height(), 0);
    ASSERT_GT(visible.width(), 0);
    EXPECT_GE(visible.yMin(), 0);
    EXPECT_LT(visible.yMax(), kHeight);
    if (screen_->presetSelector().selectedIndex() + 2 ==
        screen_->presetSelector().itemCount()) {
      capture("last_theme_menu");
    }
    focused->onSingleTapUp(1, 1);
    app_.refresh();
    app_.refresh();
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           4);
    app_.refresh();
  }

  bool contains(Color color) {
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        Color pixel;
        device_.raster().readColors(&x, &y, 1, &pixel);
        if (pixel == color) return true;
      }
    }
    return false;
  }

  // Retain reviewable captures alongside the test log, without source goldens.
  void capture(const char* name) {
    const char* directory = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR");
    if (directory == nullptr) return;
    std::ofstream out(std::string(directory) + "/" + name + ".ppm",
                      std::ios::binary);
    out << "P6\n" << kWidth << " " << kHeight << "\n255\n";
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        Color pixel;
        device_.raster().readColors(&x, &y, 1, &pixel);
        const char rgb[] = {static_cast<char>(pixel.r()),
                            static_cast<char>(pixel.g()),
                            static_cast<char>(pixel.b())};
        out.write(rgb, 3);
      }
    }
  }

  roo::byte pixels_[kWidth * kHeight * 4]{};
  RecordingDevice device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Theme theme_;
  Environment environment_;
  Application app_;
  std::unique_ptr<PalettePreview> storage_;
  PalettePreview* screen_ = nullptr;
  Task* owner_ = nullptr;
};

// Verifies every catalog entry is reachable through the actual drop-down menu,
// and changing mode keeps the selection while coordinating M3, M2 and roots.
TEST_F(PalettePreviewTest, SelectsEveryThemeAndBothModes) {
  app_.refresh();
  ASSERT_EQ(material3::schemes::kPresetCount,
            screen_->presetSelector().itemCount());
  EXPECT_EQ(roo::string_view("Indigo San Marino"),
            screen_->presetSelector().selectedText());
  const size_t first = screen_->presetSelector().selectedIndex();
  theme_.framework.state.pressed = 17;
  for (size_t step = 0; step < material3::schemes::kPresetCount; ++step) {
    if (step > 0) chooseNextPreset();
    const size_t i = (first + step) % material3::schemes::kPresetCount;
    ASSERT_EQ(i, screen_->presetSelector().selectedIndex());
    const material3::schemes::SchemePreset& preset =
        material3::schemes::kPresets[i];
    EXPECT_EQ(roo::string_view(preset.name),
              screen_->presetSelector().selectedText());
    EXPECT_EQ(preset.colors().light.primary, theme_.framework.emphasis);
    screen_->darkModeSwitch().onClicked();
    EXPECT_TRUE(screen_->darkModeSwitch().isOn());
    EXPECT_EQ(i, screen_->presetSelector().selectedIndex());
    EXPECT_EQ(preset.colors().dark.primary, theme_.material3.color.primary);
    EXPECT_EQ(preset.colors().dark.primary, theme_.material2.color.primary);
    EXPECT_EQ(preset.colors().dark.background, theme_.framework.canvas);
    EXPECT_EQ(17, theme_.framework.state.pressed);
    screen_->darkModeSwitch().onClicked();
    EXPECT_FALSE(screen_->darkModeSwitch().isOn());
    EXPECT_EQ(preset.colors().light.primary, theme_.framework.emphasis);
  }
}

// Verifies secondary/tertiary surfaces render immediately and all three repaint
// paths settle each pixel once, with incremental/full equivalence after edits.
TEST_F(PalettePreviewTest, RepaintsColoredPreviewWithoutOverdraw) {
  singleWriteFrame("initial");
  EXPECT_TRUE(contains(theme_.material3.color.secondaryContainer));
  EXPECT_TRUE(contains(theme_.material3.color.tertiaryContainer));
  EXPECT_TRUE(contains(theme_.material3.color.onSecondaryContainer));
  EXPECT_TRUE(contains(theme_.material3.color.onTertiaryContainer));
  capture("light_top");
  expectFullRepaintMatches();

  // Programmatic selection dirties only the selector; it does not invoke the
  // installation callback. The complete repaint must preserve the same image.
  ASSERT_TRUE(screen_->presetSelector().setSelectedIndex(1));
  singleWriteFrame("dirty selector");
  expectFullRepaintMatches();

  screen_->preview().scrollToBottom();
  singleWriteFrame("scroll");
  capture("light_controls");
  expectFullRepaintMatches();
  EXPECT_GT(screen_->preview().contents()->height(),
            screen_->preview().height());
  EXPECT_GE(screen_->presetSelector().bounds().yMin(), 0);

  screen_->darkModeSwitch().onClicked();
  system_time_delay_micros(500000);
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  singleWriteFrame("dark");
  capture("dark_controls");
  expectFullRepaintMatches();
}

}  // namespace
}  // namespace roo_windows::coordinated_palette_example
