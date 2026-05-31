#pragma once

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

namespace roo_windows::test_support {

inline roo_display::Color QuantizeToArgb4444(roo_display::Color color) {
  roo_display::Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

class ColorBoxWidget : public BasicSurfaceWidget {
 public:
  ColorBoxWidget(ApplicationContext& context, roo_display::Color color,
                 Dimensions dims)
      : BasicSurfaceWidget(context), color_(color), dims_(dims) {}

  roo_display::Color background() const override { return color_; }

  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  roo_display::Color color_;
  Dimensions dims_;
};

class ElevatedColorBoxWidget : public ColorBoxWidget {
 public:
  ElevatedColorBoxWidget(ApplicationContext& context, roo_display::Color color,
                         Dimensions dims, uint8_t elevation)
      : ColorBoxWidget(context, color, dims), elevation_(elevation) {}

  uint8_t getElevation() const override { return elevation_; }

 private:
  uint8_t elevation_;
};

class PointOverlayBoxWidget : public ColorBoxWidget {
 public:
  PointOverlayBoxWidget(ApplicationContext& context, roo_display::Color color,
                        Dimensions dims)
      : ColorBoxWidget(context, color, dims) {}

  OverlayType getOverlayType() const override { return OVERLAY_POINT; }

  bool isClickable() const override { return true; }
};

class MutableShapeColorBoxWidget : public BasicSurfaceWidget {
 public:
  MutableShapeColorBoxWidget(ApplicationContext& context,
                             roo_display::Color color, Dimensions dims)
      : BasicSurfaceWidget(context),
        color_(color),
        dims_(dims),
        rounded_(false) {}

  roo_display::Color background() const override { return color_; }

  BorderStyle getBorderStyle() const override {
    return rounded_ ? BorderStyle(10, 0) : BorderStyle(0, 0);
  }

  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  void setRounded(bool rounded) {
    if (rounded == rounded_) return;
    rounded_ = rounded;
    invalidateInterior();
  }

 private:
  roo_display::Color color_;
  Dimensions dims_;
  bool rounded_;
};

class TouchSpyWidget : public BasicWidget {
 public:
  explicit TouchSpyWidget(ApplicationContext& context, Dimensions dims)
      : BasicWidget(context), dims_(dims), touch_down_count_(0) {}

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

class InkBoundsWidget : public BasicWidget {
 public:
  InkBoundsWidget(ApplicationContext& context, Dimensions dims,
                  Insets ink_insets)
      : BasicWidget(context), dims_(dims), ink_insets_(ink_insets) {}

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  Insets getInkInsets() const override { return ink_insets_; }

  Rect exclusionBounds() const { return getDirectPaintExclusionBounds(); }

 private:
  Dimensions dims_;
  Insets ink_insets_;
};

class RooWindowsRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 64;
  static constexpr int16_t kHeight = 48;

  RooWindowsRenderTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  roo_display::Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    roo_display::Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

  ApplicationContext& context() { return app_.context(); }
  const ApplicationContext& context() const { return app_.context(); }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

template <int16_t W, int16_t H>
class RooWindowsRenderTestSized : public testing::Test {
 protected:
  static constexpr int16_t kWidth = W;
  static constexpr int16_t kHeight = H;

  RooWindowsRenderTestSized()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  roo_display::Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    roo_display::Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  bool refresh(roo_time::Uptime deadline = roo_time::Uptime::Max()) {
    return app_.refresh(deadline);
  }

  ApplicationContext& context() { return app_.context(); }
  const ApplicationContext& context() const { return app_.context(); }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

}  // namespace roo_windows::test_support