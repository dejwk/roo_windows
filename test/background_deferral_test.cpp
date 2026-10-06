#include "roo_windows/core/background_deferral.h"

#include <array>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/smooth.h"
#include "roo_testing/system/timer.h"
#include "roo_windows.h"
#include "roo_windows/containers/blit_cache_container.h"

namespace roo_windows {
namespace {
using namespace roo_display;
constexpr int kWidth = 96;
constexpr int kHeight = 72;

class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* data)
      : OffscreenDevice(kWidth, kHeight, data, Argb8888()) {}

  void reset() { writes.fill(0); }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(Color value, uint32_t count) override {
    countPixels(count);
    OffscreenDevice::fill(value, count);
  }

  void writePixels(BlendingMode mode, Color* values, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(BlendingMode mode, Color value, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(value, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* values, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(BlendingMode mode, Color value, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(value, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  void blitCopy(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t dx,
                int16_t dy) override {
    ++blits;
    OffscreenDevice::blitCopy(x0, y0, x1, y1, dx, dy);
  }

  int blits = 0;
  std::array<uint16_t, kWidth * kHeight> writes{};

 private:
  void countPixels(uint32_t count) {
    while (count-- != 0) {
      EXPECT_GE(x_, 0);
      EXPECT_LT(x_, kWidth);
      EXPECT_GE(y_, 0);
      EXPECT_LT(y_, kHeight);
      if (x_ >= 0 && x_ < kWidth && y_ >= 0 && y_ < kHeight) {
        ++writes[y_ * kWidth + x_];
      }
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }
  int16_t left_ = 0;
  int16_t right_ = 0;
  int16_t x_ = 0;
  int16_t y_ = 0;
};

class BackgroundDeferralTest : public testing::Test {
 protected:
  BackgroundDeferralTest()
      : device_(pixels_),
        display_(device_),
        env_(scheduler_),
        app_(&env_, display_) {}

  Color pixelAt(int16_t x, int16_t y) const {
    Color result;
    device_.raster().readColors(&x, &y, 1, &result);
    return result;
  }

  roo::byte pixels_[kWidth * kHeight * 4]{};
  RecordingDevice device_;
  Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
};

class OptionalSurface : public SurfaceWidget {
 public:
  explicit OptionalSurface(ApplicationContext& ctx) : SurfaceWidget(ctx) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return {96, 160};
  }

  void paint(PaintContext& ctx) const override {
    if (own_scope) {
      system_time_delay_micros(2);
      Box viewport = ctx.canvas().clip_box();
      internal::BackgroundDeferralScope scope(ctx.clipperForFramework(),
                                              viewport, viewport, 0);
      ctx.clearDeferrableBackground();
    } else {
      ctx.clearDeferrableBackground();
    }
  }

  bool own_scope = false;
};

class ExposedCache : public BlitCacheContainer {
 public:
  using BlitCacheContainer::BlitCacheContainer;
  using BlitCacheContainer::paintWidgetContents;
};

// Verifies a cache reached inside an optional scope cannot copy its old source,
// and an enclosing cache cannot publish a source after a child defers.
TEST_F(BackgroundDeferralTest, CacheGuardsBothTraversalDirections) {
  auto surface = std::make_unique<OptionalSurface>(app_.context());
  OptionalSurface* child = surface.get();
  auto cache = std::make_unique<ExposedCache>(app_.context());
  ExposedCache* ptr = cache.get();
  ptr->setChild(std::move(surface));
  app_.add(std::move(cache), display_.extents());
  app_.refresh();
  ptr->moveTo(Rect(0, -8, 95, 63));
  {
    Surface output(device_, 0, 0, display_.extents(), false,
                   ptr->effectiveBackground(), FillMode::kVisible,
                   BlendingMode::kSource);
    Canvas canvas(&output);
    internal::ClipperState state;
    Clipper clipper(state, device_, roo_time::Uptime::Now());
    canvas.set_out(clipper.out());
    PaintContext ctx(canvas, clipper);
    internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                            display_.extents(), 0);
    ptr->paintWidgetContents(ctx);
    EXPECT_EQ(0, device_.blits);
  }
  ptr->invalidateInterior();
  app_.refresh();
  child->own_scope = true;
  ptr->invalidateInterior();
  app_.window().setAdvisoryPaintBudget(roo_time::Micros(1));
  // Expire the budget inside the child's paint, independently of host speed.
  app_.refresh();
  ptr->moveTo(Rect(0, -16, 95, 55));
  child->own_scope = false;
  app_.refresh();
  EXPECT_EQ(0, device_.blits);
}

// Verifies omitted background survives lower ancestor writes, the mandatory
// band writes once, and the sticky flag outlives the lexical scope.
TEST_F(BackgroundDeferralTest, ExpiredBandsPreservePixelsAndCoalesce) {
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  canvas.clear();
  device_.reset();
  internal::ClipperState state;
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  {
    internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                            display_.extents(), 1);
    ctx.clearDeferrableBackground();
    EXPECT_TRUE(scope.deferred());
    EXPECT_EQ(2u, clipper.exclusions().size());
    EXPECT_EQ(1, device_.writes[20 * kWidth]);
    EXPECT_EQ(0, device_.writes[40 * kWidth]);
  }
  EXPECT_TRUE(clipper.backgroundDeferred());
  ctx.addExclusion(Rect(0, 16, 95, 31));
  ctx.setBgcolor(color::Red);
  ctx.clear();
  for (uint16_t count : device_.writes) EXPECT_LE(count, 1);
  EXPECT_EQ(color::Blue, pixelAt(10, 40));
}

// Verifies disabled helper output is complete, and eligibility requires an
// explicitly optional opaque surface even when its color differs.
TEST_F(BackgroundDeferralTest, DisabledFillsCompleteAndContrastingFillsDefer) {
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  ctx.clearDeferrableBackground();
  EXPECT_FALSE(clipper.backgroundDeferred());
  for (uint16_t count : device_.writes) EXPECT_EQ(count, 1);
  device_.reset();
  ctx.setBgcolor(color::Red);
  internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                          display_.extents(), 0);
  ctx.clearDeferrableBackground();
  EXPECT_TRUE(scope.deferred());
  EXPECT_EQ(1, device_.writes[5 * kWidth]);
  EXPECT_EQ(0, device_.writes[40 * kWidth]);
}

// Verifies pending overlay bounds reject entry admission but do not veto an
// admitted fill; a later ordinary draw still composes the retained overlay.
TEST_F(BackgroundDeferralTest,
       PendingOverlayCanLagButOrdinaryDrawingComposesIt) {
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  ctx.addOverlayShape(SmoothFilledCircle(FpPoint{40, 40}, 5, color::Red));
  ctx.addOverlayShape(SmoothFilledCircle(FpPoint{40, 5}, 3, color::Red));
  EXPECT_FALSE(clipper.backgroundUnobscured(display_.extents()));
  internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                          display_.extents(), 0);
  ctx.clearDeferrableBackground();
  EXPECT_EQ(0, device_.writes[40 * kWidth]);
  EXPECT_EQ(0, device_.writes[60 * kWidth]);
  // The mandatory band composes another overlay from the same retained stack.
  EXPECT_EQ(color::Red, pixelAt(40, 5));
}

// Builds both ordinary decoration and captured rounded foreground using the
// production compositor. No descriptor is removed when background output skips.
void PaintDecoratedRows(PaintContext& ctx) {
  Clipper& clipper = ctx.clipperForFramework();
  int owner;
  internal::RoundedClip& mask = clipper.prepareRoundedClip(
      &owner, Box(12, 20, 70, 55), BorderStyle(10, 1));
  {
    PaintContext row = ctx.clipped(Rect(12, 20, 70, 55));
    row.setBgcolor(color::Red);
    internal::RoundedClipScope rounded(row, mask);
    // A foreground stripe crosses fractional edge pixels, which must receive
    // their current backing colors before the completed decoration is retained.
    row.fillRect(Rect(12, 20, 70, 24), color::White);
    row.addExclusion(Rect(12, 20, 70, 24));
    row.clearDeferrableBackground();
    row.addExclusion(Rect(12, 20, 70, 55));
  }
  clipper.addRoundedDecoration(&owner, ctx.canvas().clip_box(),
                               Box(12, 20, 70, 55), 2, color::Red,
                               BorderStyle(10, 1), color::Green);
  PaintDecoration decoration;
  decoration.bounds = Rect(75, 20, 90, 55);
  decoration.background = color::Green;
  decoration.corner_radii = {5, 5, 5, 5};
  decoration.elevation = 2;
  decoration.outline_width = 1;
  decoration.outline_color = color::White;
  ctx.addDecoration(decoration);
  ctx.clearDeferrableBackground();
}

// Verifies deferred ordinary and nested rounded composition matches a complete
// reference at every emitted pixel, with untouched old output in skipped gaps.
TEST_F(BackgroundDeferralTest,
       PendingDecorationsRetainCurrentCompositionInputs) {
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  std::array<Color, kWidth * kHeight> reference;
  internal::ClipperState state;
  {
    Canvas canvas(&surface);
    Clipper clipper(state, device_);
    canvas.set_out(clipper.out());
    PaintContext ctx(canvas, clipper);
    PaintDecoratedRows(ctx);
  }
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) reference[y * kWidth + x] = pixelAt(x, y);
  }
  Canvas old(&surface);
  old.set_bgcolor(color::Magenta);
  old.clear();
  device_.reset();
  Canvas canvas(&surface);
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                          display_.extents(), 1);
  PaintDecoratedRows(ctx);
  EXPECT_TRUE(scope.deferred());
  EXPECT_EQ(0, device_.writes[54 * kWidth + 13]);
  EXPECT_EQ(color::Magenta, pixelAt(13, 54));
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      int index = y * kWidth + x;
      EXPECT_LE(device_.writes[index], 1);
      if (device_.writes[index] != 0) {
        EXPECT_EQ(reference[index], pixelAt(x, y));
      }
      if (y >= 16 && y < 32) {
        EXPECT_EQ(reference[index], pixelAt(x, y));
      }
    }
  }
}

// Verifies active inherited content effects still require complete output.
TEST_F(BackgroundDeferralTest, ActiveContentEffectsVetoOptionalFills) {
  auto widget = std::make_unique<OptionalSurface>(app_.context());
  OptionalSurface* ptr = widget.get();
  app_.add(std::move(widget), display_.extents());
  ptr->setEnabled(false);
  app_.refresh();
  device_.reset();
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  clipper.pushOverlaySpec(*ptr, canvas);
  ASSERT_TRUE(clipper.hasContentEffects());
  internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                          display_.extents(), 0);
  ctx.clearDeferrableBackground();
  EXPECT_FALSE(scope.deferred());
  for (uint16_t count : device_.writes) EXPECT_EQ(count, 1);
  clipper.popOverlaySpec();
}

// Verifies nested rounded masks admit only their opaque interior; fractional
// edge work completes through the existing mask route.
TEST_F(BackgroundDeferralTest, NestedRoundedInteriorsAreConservative) {
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  int outer_owner;
  int inner_owner;
  internal::RoundedClip& outer = clipper.prepareRoundedClip(
      &outer_owner, display_.extents(), BorderStyle(20, 1));
  internal::RoundedClipScope outer_scope(ctx, outer);
  internal::RoundedClip& inner = clipper.prepareRoundedClip(
      &inner_owner, Box(4, 4, 91, 67), BorderStyle(12, 1));
  internal::RoundedClipScope inner_scope(ctx, inner);
  Box interior = clipper.opaqueInterior(display_.extents());
  EXPECT_TRUE(outer.containsOpaque(interior));
  EXPECT_TRUE(inner.containsOpaque(interior));
  internal::BackgroundDeferralScope scope(clipper, display_.extents(), interior,
                                          0);
  ctx.clearDeferrableBackground();
  EXPECT_TRUE(scope.deferred());
  EXPECT_EQ(0, device_.writes[40 * kWidth + 40]);
}

// Verifies suspension fully paints through a translated context, then restores
// permission for subsequent clipped content in the same traversal.
TEST_F(BackgroundDeferralTest, SuspensionRestoresPermission) {
  Surface surface(device_, 3, 5, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, device_, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                          display_.extents(), 0);
  {
    internal::BackgroundDeferralSuspension suspension(clipper);
    ctx.clipped(Rect(0, 15, 20, 25)).clearDeferrableBackground();
    EXPECT_FALSE(scope.deferred());
    EXPECT_EQ(1, device_.writes[20 * kWidth + 3]);
  }
  ctx.clipped(Rect(0, 35, 20, 45)).clearDeferrableBackground();
  EXPECT_TRUE(scope.deferred());
  EXPECT_EQ(0, device_.writes[40 * kWidth + 3]);
}

}  // namespace
}  // namespace roo_windows
