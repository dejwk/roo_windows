#include "roo_windows.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/navigation_panel.h"
#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/static_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/press_overlay.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/dialogs/radio_list_dialog.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/checkbox.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/icon_with_caption.h"
#include "roo_windows/widgets/image.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/radio_button.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/toggle_buttons.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {

namespace {

Color QuantizeToArgb4444(Color color) {
  Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

class ColorBoxWidget : public BasicSurfaceWidget {
 public:
  ColorBoxWidget(const Environment& env, Color color, Dimensions dims)
      : BasicSurfaceWidget(env), color_(color), dims_(dims) {}

  Color background() const override { return color_; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Color color_;
  Dimensions dims_;
};

class TouchSpyWidget : public BasicWidget {
 public:
  explicit TouchSpyWidget(const Environment& env, Dimensions dims)
      : BasicWidget(env), dims_(dims), touch_down_count_(0) {}

  Widget* dispatchTouchDownEvent(XDim x, YDim y) override {
    if (!bounds().contains(x, y)) return nullptr;
    ++touch_down_count_;
    return this;
  }

  int touch_down_count() const { return touch_down_count_; }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  Dimensions dims_;
  int touch_down_count_;
};

class RooWindowsRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 64;
  static constexpr int16_t kHeight = 48;

  RooWindowsRenderTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

}  // namespace

TEST(Windows, BasicCompilation) {
  roo::byte raster[320 * 240 * 2];
  OffscreenDevice<Argb4444> offscreen(320, 240, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
}

TEST(PressOverlay, WideTopStripCrossingCenterIsNotUniformTransparent) {
  PressOverlay overlay(0, 0, 10, color::Red);

  Color uniform;
  EXPECT_FALSE(overlay.readUniformColorRect(-11, -9, 11, -9, &uniform));

  Color colors[23];
  EXPECT_FALSE(overlay.readColorRect(-11, -9, 11, -9, colors));
  EXPECT_NE(colors[11], color::Transparent);
}

TEST_F(RooWindowsRenderTest, LaterAddedChildPaintsOnTop) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(20, 20));
  auto front =
      std::make_unique<ColorBoxWidget>(env_, color::Blue, Dimensions(20, 20));

  app_.add(WidgetRef(std::move(back)), Box(4, 4, 30, 30));
  app_.add(WidgetRef(std::move(front)), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(8, 8));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

TEST_F(RooWindowsRenderTest, HideAndShowRestoresUnderlyingContent) {
  auto back =
      std::make_unique<ColorBoxWidget>(env_, color::Red, Dimensions(20, 20));
  auto front =
      std::make_unique<ColorBoxWidget>(env_, color::Blue, Dimensions(20, 20));
  ColorBoxWidget* front_ptr = front.get();

  app_.add(WidgetRef(std::move(back)), Box(4, 4, 30, 30));
  app_.add(WidgetRef(std::move(front)), Box(16, 16, 40, 40));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));

  front_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(20, 20));

  front_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(20, 20));
}

TEST_F(RooWindowsRenderTest, TouchDispatchPrefersTopmostVisibleChild) {
  auto back = std::make_unique<TouchSpyWidget>(env_, Dimensions(20, 20));
  auto front = std::make_unique<TouchSpyWidget>(env_, Dimensions(20, 20));
  TouchSpyWidget* back_ptr = back.get();
  TouchSpyWidget* front_ptr = front.get();

  app_.add(WidgetRef(std::move(back)), Box(0, 0, 24, 24));
  app_.add(WidgetRef(std::move(front)), Box(10, 10, 34, 34));

  Widget* target = app_.root().dispatchTouchDownEvent(15, 15);
  ASSERT_EQ(front_ptr, target);
  EXPECT_EQ(1, front_ptr->touch_down_count());
  EXPECT_EQ(0, back_ptr->touch_down_count());

  front_ptr->setVisibility(Visibility::kInvisible);
  target = app_.root().dispatchTouchDownEvent(15, 15);
  ASSERT_EQ(back_ptr, target);
  EXPECT_EQ(1, front_ptr->touch_down_count());
  EXPECT_EQ(1, back_ptr->touch_down_count());
}

TEST_F(RooWindowsRenderTest, RefreshCanResumeAfterDeadlineExceeded) {
  auto box =
      std::make_unique<ColorBoxWidget>(env_, color::Green, Dimensions(20, 20));
  app_.add(WidgetRef(std::move(box)), Box(8, 8, 36, 36));

  EXPECT_FALSE(refresh(roo_time::Uptime::Start()));
  EXPECT_NE(QuantizeToArgb4444(color::Green), pixelAt(16, 16));

  EXPECT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Green), pixelAt(16, 16));
}

}  // namespace roo_windows