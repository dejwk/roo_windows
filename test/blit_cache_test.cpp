#include "roo_windows/core/blit_cache.h"

#include <array>
#include <limits>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/background_deferral.h"
#include "roo_windows/core/paint_context.h"

namespace roo_windows {
namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;

constexpr int16_t kWidth = 64;
constexpr int16_t kHeight = 48;
const Rect kBounds(0, 0, 39, 23);
const Rect kStable(1, 0, 38, 23);

// Counts copied pixels as writes too: every painted pixel must be settled once.
class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* pixels)
      : OffscreenDevice(kWidth, kHeight, pixels, Argb8888()) {}

  void reset() {
    writes.fill(0);
    copies = 0;
  }

  const Capabilities& getCapabilities() const override {
    static const Capabilities kBlit(true, true);
    static const Capabilities kNoBlit(true, false);
    return supports_copy ? kBlit : kNoBlit;
  }

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

  void writePixels(BlendingMode mode, Color* colors, int16_t* xs, int16_t* ys,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(xs[i], ys[i], xs[i], ys[i], mode);
      write(colors + i, 1);
    }
  }

  void fillPixels(BlendingMode mode, Color color, int16_t* xs, int16_t* ys,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(xs[i], ys[i], xs[i], ys[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* colors, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, colors[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  void blitCopy(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t dx,
                int16_t dy) override {
    ++copies;
    for (int y = dy; y <= dy + y1 - y0; ++y) {
      for (int x = dx; x <= dx + x1 - x0; ++x) {
        ++writes[y * kWidth + x];
      }
    }
    OffscreenDevice::blitCopy(x0, y0, x1, y1, dx, dy);
  }

  bool supports_copy = true;
  int copies = 0;
  std::array<uint8_t, kWidth * kHeight> writes{};

 private:
  void record(uint32_t count) {
    while (count-- != 0) {
      ++writes[y_ * kWidth + x_];
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

// Each refresh gets a fresh clipper; the cache alone persists across frames.
class PaintPass {
 public:
  explicit PaintPass(RecordingDevice& device,
                     roo_time::Uptime deadline = roo_time::Uptime::Max())
      : surface_(device, 0, 0, Box(0, 0, kWidth - 1, kHeight - 1), false,
                 roo_display::color::Black, roo_display::FillMode::kVisible,
                 BlendingMode::kSource),
        clipper(state_, device, deadline),
        ctx(Canvas(&surface_), clipper) {
    ctx.canvas().set_out(clipper.out());
    ctx.canvas().shift(5, 7);
  }

 private:
  roo_display::Surface surface_;
  internal::ClipperState state_;

 public:
  Clipper clipper;
  PaintContext ctx;
};

Color Pattern(int x, int y) {
  return Color(0xFF000000u | ((x & 255) << 16) | ((y & 255) << 8) |
               ((x + y) & 255));
}

void DrawPattern(PaintContext& ctx, const BlitCache::PaintScope& frame,
                 PixelDelta origin = {}) {
  for (const Rect& region : frame.repaintRegions()) {
    for (int y = region.yMin(); y <= region.yMax(); ++y) {
      for (int x = region.xMin(); x <= region.xMax(); ++x) {
        ctx.fillRect(x, y, x, y, Pattern(x + origin.dx, y + origin.dy));
      }
    }
  }
}

class BlitCacheTest : public testing::Test {
 protected:
  Rect paint(PixelDelta movement = {}, PixelDelta origin = {},
             BlitOptions options = {kStable}) {
    device.reset();
    PaintPass pass(device);
    auto frame = cache.beginPaint(pass.ctx, kBounds, movement, options);
    DrawPattern(pass.ctx, frame, origin);
    return frame.copiedBounds();
  }

  void expectPixelsOnce(PixelDelta origin = {}) {
    for (int16_t y = 0; y < kHeight; ++y) {
      for (int16_t x = 0; x < kWidth; ++x) {
        const bool inside = kBounds.contains(x - 5, y - 7);
        ASSERT_EQ(inside ? 1 : 0, device.writes[y * kWidth + x])
            << "at " << x << "," << y;
        if (inside) {
          Color value;
          device.raster().readColors(&x, &y, 1, &value);
          ASSERT_EQ(Pattern(x - 5 + origin.dx, y - 7 + origin.dy), value);
        }
      }
    }
  }

  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device{pixels.data()};
  BlitCache cache;
};

// Verifies initial, dirty, translated and invalidated repaint settle each pixel
// exactly once, and repeated copies preserve the same result as full painting.
TEST_F(BlitCacheTest, CopiesStableColumnsInBothDirections) {
  EXPECT_TRUE(paint().empty());
  expectPixelsOnce();
  EXPECT_TRUE(paint().empty());
  expectPixelsOnce();
  EXPECT_EQ(Rect(1, 0, 33, 23), paint({-5, 0}, {5, 0}));
  EXPECT_EQ(1, device.copies);
  expectPixelsOnce({5, 0});
  EXPECT_EQ(Rect(4, 0, 38, 23), paint({3, 0}, {2, 0}));
  expectPixelsOnce({2, 0});
  cache.invalidate();
  EXPECT_TRUE(paint({-2, 0}, {4, 0}).empty());
  expectPixelsOnce({4, 0});
}

// Verifies explicit stationary reuse preserves only certified pixels, without
// a device copy, and explicit invalidation restores normal single-write paint.
TEST_F(BlitCacheTest, PreservesStationaryPixelsOnlyWhenRequested) {
  paint();
  EXPECT_TRUE(paint().empty());
  const auto before = pixels;
  EXPECT_EQ(kStable, paint({}, {}, {kStable, BlitCoverage::kFullHeight, true}));
  EXPECT_EQ(0, device.copies);
  EXPECT_EQ(before, pixels);
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      bool painted =
          kBounds.contains(x - 5, y - 7) && !kStable.contains(x - 5, y - 7);
      EXPECT_EQ(painted ? 1 : 0, device.writes[y * kWidth + x]);
    }
  }
  cache.invalidate();
  EXPECT_TRUE(
      paint({}, {}, {kStable, BlitCoverage::kFullHeight, true}).empty());
  expectPixelsOnce();
}

// Verifies stationary reuse excludes foreground and cannot resurrect pixels
// revoked by source damage, even when the current viewport grows.
TEST_F(BlitCacheTest, StationaryReuseHonorsDamageAndForeground) {
  paint({}, {}, {kBounds});
  cache.invalidateSource(Rect(5, 7, 14, 30));
  PaintPass pass(device);
  pass.clipper.addExclusion(Box(35, 7, 44, 30));
  auto frame =
      cache.beginPaint(pass.ctx, Rect(0, 0, 49, 23), {},
                       {Rect(0, 0, 49, 23), BlitCoverage::kFullHeight, true});
  EXPECT_EQ(Rect(10, 0, 29, 23), frame.copiedBounds());
  EXPECT_EQ(0, device.copies);
  DrawPattern(pass.ctx, frame);
}

// Verifies current reusable bounds restrict destinations independently of the
// preceding source, with four disjoint repaint regions around an interior copy.
TEST_F(BlitCacheTest, ReportsCopiedBoundsAndOptionalRepaintComplement) {
  paint({}, {}, {kBounds});
  device.reset();
  PaintPass pass(device);
  const Rect interior(4, 4, 35, 19);
  auto frame = cache.beginPaint(pass.ctx, kBounds, {1, 1}, {interior});
  EXPECT_EQ(interior, frame.copiedBounds());
  auto regions = frame.repaintRegions();
  EXPECT_EQ(4, regions.end() - regions.begin());
  DrawPattern(pass.ctx, frame, {-1, -1});
  expectPixelsOnce({-1, -1});
}

// Verifies full-height coverage is measured before reusable bounds, while a
// geometric viewport clip really does narrow the visible height requirement.
TEST_F(BlitCacheTest, FullHeightRejectsPartialRowsButAcceptsClippedViewport) {
  paint({}, {}, {kBounds});
  const Rect middle(1, 4, 38, 19);
  EXPECT_EQ(middle, paint({-1, 0}, {1, 0}, {middle}));
  EXPECT_TRUE(
      paint({-1, 0}, {2, 0}, {middle, BlitCoverage::kFullHeight}).empty());
  expectPixelsOnce({2, 0});
  PaintPass pass(device);
  PaintContext clipped = pass.ctx.clipped(Rect(0, 4, 39, 19));
  auto frame = cache.beginPaint(clipped, kBounds, {-1, 0},
                                {kStable, BlitCoverage::kFullHeight});
  EXPECT_EQ(Rect(1, 4, 37, 19), frame.copiedBounds());
  DrawPattern(clipped, frame, {3, 0});
}

// Verifies unsupported devices and nonrepresentable or nonoverlapping movement
// all fall back to full painting without adding an exclusion for a rejected
// copy.
TEST_F(BlitCacheTest, FallsBackWithoutUsableCopy) {
  paint();
  device.supports_copy = false;
  EXPECT_TRUE(paint({1, 0}, {-1, 0}).empty());
  expectPixelsOnce({-1, 0});
  device.supports_copy = true;
  EXPECT_TRUE(paint().empty());
  for (PixelDelta movement : {PixelDelta{40, 0},
                              {0, -24},
                              {std::numeric_limits<int32_t>::max(), 0},
                              {0, std::numeric_limits<int32_t>::min()}}) {
    EXPECT_TRUE(paint(movement).empty());
    expectPixelsOnce();
  }
}

// Verifies partial/full invalidation during painting survives scope
// destruction, and far-offscreen damage cannot wrap into the retained device
// rectangle.
TEST_F(BlitCacheTest, PreservesInvalidationsRaisedDuringPaint) {
  {
    PaintPass pass(device);
    auto frame = cache.beginPaint(pass.ctx, kBounds);
    DrawPattern(pass.ctx, frame);
    cache.invalidateSource(Rect(5, 7, 44, 18));
  }
  EXPECT_EQ(Rect(1, 12, 38, 23), paint({1, 0}, {-1, 0}));
  cache.invalidateSource(Rect(5, 65536 + 7, 44, 65536 + 30));
  EXPECT_FALSE(paint({-1, 0}).empty());
  {
    PaintPass pass(device);
    auto frame = cache.beginPaint(pass.ctx, kBounds);
    DrawPattern(pass.ctx, frame);
    cache.invalidate();
  }
  EXPECT_TRUE(paint({1, 0}, {-1, 0}).empty());
  expectPixelsOnce({-1, 0});
}

// Verifies deferral that begins inside the cached paint prevents that
// incomplete frame from becoming the source of a subsequent copy.
TEST_F(BlitCacheTest, DeferredBackgroundRevokesCertificateOnScopeExit) {
  {
    PaintPass pass(device, roo_time::Uptime::Now() - roo_time::Millis(1));
    auto frame = cache.beginPaint(pass.ctx, kBounds);
    const Box viewport = kBounds.translate(5, 7).asBox();
    internal::BackgroundDeferralScope deferred(pass.clipper, viewport, viewport,
                                               0);
    pass.ctx.clearDeferrableBackground();
    EXPECT_TRUE(pass.clipper.backgroundDeferred());
  }
  EXPECT_TRUE(paint({1, 0}, {-1, 0}).empty());
  expectPixelsOnce({-1, 0});
}

// Verifies narrower paint bounds preserve the planner's source-viewport safety
// restriction, and empty paint bounds yield no repaint rectangles.
TEST_F(BlitCacheTest, NarrowAndEmptyPaintBoundsRestrictRepaint) {
  paint({}, {}, {kBounds});
  {
    PaintPass pass(device);
    const Rect small(5, 5, 25, 20);
    auto frame = cache.beginPaint(pass.ctx, small, {1, 0});
    EXPECT_EQ(Rect(6, 5, 25, 20), frame.copiedBounds());
    auto regions = frame.repaintRegions();
    ASSERT_EQ(1, regions.end() - regions.begin());
    EXPECT_EQ(Rect(5, 5, 5, 20), *regions.begin());
  }
  {
    PaintPass pass(device);
    auto frame = cache.beginPaint(pass.ctx, Rect(0, 0, -1, -1), {1, 0});
    EXPECT_TRUE(frame.copiedBounds().empty());
    auto regions = frame.repaintRegions();
    EXPECT_EQ(regions.begin(), regions.end());
  }
}

static_assert(sizeof(BlitCache) == sizeof(Box),
              "Retain one rectangle, not paint pointers or a pixel buffer");

}  // namespace
}  // namespace roo_windows
