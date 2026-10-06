#include "roo_windows/core/background_deferral.h"

#include <array>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/smooth.h"
#include "roo_testing/system/timer.h"
#include "roo_windows.h"
#include "roo_windows/containers/accelerated_scrollable_panel.h"
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
    if (delay_per_fill_us != 0) system_time_delay_micros(delay_per_fill_us);
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
  int delay_per_fill_us = 0;
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

// Verifies a display operation crossing the advisory deadline completes; only
// subsequent eligible bands skip, without checks inside physical writes.
TEST_F(BackgroundDeferralTest, DeadlineExpiresBetweenIndivisibleBandWrites) {
  Surface surface(device_, 0, 0, display_.extents(), false, color::Blue,
                  FillMode::kVisible, BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, device_,
                  roo_time::Uptime::Now() + roo_time::Micros(1500));
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  internal::BackgroundDeferralScope scope(clipper, display_.extents(),
                                          display_.extents(), 0);
  device_.delay_per_fill_us = 1000;
  ctx.clearDeferrableBackground();
  EXPECT_TRUE(scope.deferred());
  EXPECT_EQ(1, device_.writes[5 * kWidth]);
  EXPECT_EQ(1, device_.writes[20 * kWidth]);
  EXPECT_EQ(0, device_.writes[40 * kWidth]);
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

class LogStripe : public Widget {
 public:
  using Widget::Widget;

  Dimensions getSuggestedMinimumDimensions() const override { return {72, 9}; }

  void paint(PaintContext& ctx) const override {
    ctx.fillRect(bounds(), color::White);
  }
};

class LogRow : public Panel {
 public:
  LogRow(ApplicationContext& ctx, Color color, bool rounded)
      : Panel(ctx), color_(color), rounded_(rounded) {
    add(std::make_unique<LogStripe>(ctx), Rect(0, 0, 71, 8));
  }

  Color background() const override { return color_; }

  bool clipsChildrenToRoundedBounds() const override { return rounded_; }

  BorderStyle getBorderStyle() const override { return BorderStyle(10, 1); }

  uint8_t getElevation() const override { return 2; }

 private:
  Color color_;
  bool rounded_;
};

class LogRows : public Panel {
 public:
  LogRows(ApplicationContext& ctx, bool rounded) : Panel(ctx) {
    for (int i = 0; i < 8; ++i) {
      add(std::make_unique<LogRow>(ctx, i % 2 == 0 ? color::Red : color::Green,
                                   rounded),
          Rect(12, 10 + i * 48, 83, 45 + i * 48));
    }
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return {96, 400};
  }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    Panel::onMeasure(width, height);
    return {width.resolveSize(96), height.resolveSize(400)};
  }

  Color background() const override { return color::Blue; }

  void paintWidgetContents(PaintContext& ctx) override {
    // Models a slow mandatory draw consuming the allowance before optional
    // fills.
    system_time_delay_micros(1000);
    ++paints;
    Panel::paintWidgetContents(ctx);
    if (during_paint != nullptr) {
      std::function<void()> callback = std::move(during_paint);
      callback();
    }
  }

  void addWidget(WidgetRef widget, const Rect& bounds) {
    add(std::move(widget), bounds);
  }

  int paints = 0;
  std::function<void()> during_paint;
};

class ObservableAcceleratedPanel : public AcceleratedScrollablePanel {
 public:
  using AcceleratedScrollablePanel::AcceleratedScrollablePanel;
  using AcceleratedScrollablePanel::cleanupPending;
  using AcceleratedScrollablePanel::nextBand;
  using AcceleratedScrollablePanel::paintWidgetContents;
};

class RoundedLogFrame : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;

  bool clipsChildrenToRoundedBounds() const override { return true; }

  BorderStyle getBorderStyle() const override { return BorderStyle(20, 1); }
};

struct LogWorld {
  LogWorld(bool accelerated, bool rounded, bool rounded_parent = false)
      : device(pixels.data()),
        display(device),
        env(scheduler),
        app(&env, display) {
    auto rows = std::make_unique<LogRows>(app.context(), rounded);
    content = rows.get();
    if (accelerated) {
      auto widget = std::make_unique<ObservableAcceleratedPanel>(
          app.context(), std::move(rows));
      optional = widget.get();
      panel = widget.get();
      install(std::move(widget), rounded_parent);
    } else {
      auto widget = std::make_unique<SimpleScrollablePanel>(app.context(),
                                                            std::move(rows));
      panel = widget.get();
      install(std::move(widget), rounded_parent);
    }
    app.window().setAdvisoryPaintBudget(roo_time::Micros(1));
    app.refresh();
  }

  void install(WidgetRef widget, bool rounded_parent) {
    if (rounded_parent) {
      auto frame = std::make_unique<RoundedLogFrame>(app.context());
      frame->add(std::move(widget), Rect(display.extents()));
      app.add(std::move(frame), display.extents());
    } else {
      app.add(std::move(widget), display.extents());
    }
  }

  Color pixelAt(int16_t x, int16_t y) const {
    Color result;
    device.raster().readColors(&x, &y, 1, &result);
    return result;
  }

  void runNext() {
    roo_time::Uptime next = scheduler.getNearestExecutionTime();
    ASSERT_NE(roo_time::Uptime::Max(), next);
    if (next > roo_time::Uptime::Now()) {
      system_time_delay_micros((next - roo_time::Uptime::Now()).inMicros());
    }
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  }

  void runNextPaint() {
    int previous = content->paints;
    // An already queued input/application dispatch can precede the eligible
    // paint deadline. Advance scheduled work until precisely one refresh runs.
    for (int attempt = 0; attempt < 4 && content->paints == previous;
         ++attempt) {
      runNext();
    }
    EXPECT_EQ(previous + 1, content->paints);
  }

  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device;
  Display display;
  roo_scheduler::SchedulingService scheduler;
  Environment env;
  Application app;
  SimpleScrollablePanel* panel;
  ObservableAcceleratedPanel* optional = nullptr;
  LogRows* content;
};

// Verifies every selected band agrees with a fresh complete renderer through
// sustained motion and reversal; no pixel's mismatching age reaches one cycle.
// Stopping at each cursor position settles from scheduled work without input.
TEST(AcceleratedScrolling, MovingBandsAndScheduledSettlementMatchReference) {
  for (int variant = 0; variant < 3; ++variant) {
    bool rounded = variant != 0;
    LogWorld current(true, rounded, variant == 2);
    LogWorld reference(false, rounded, variant == 2);
    current.app.start();
    current.runNext();
    std::array<int, kWidth * kHeight> ages{};
    for (int frame = 0; frame < 30; ++frame) {
      int delta = frame < 15 ? -2 : 2;
      int band = current.optional->nextBand();
      current.panel->scrollBy(0, delta);
      reference.panel->scrollBy(0, delta);
      reference.app.refresh();
      current.device.reset();
      current.app.refresh();
      ASSERT_TRUE(current.optional->cleanupPending());
      EXPECT_TRUE(current.app.root().isDirty());
      for (int y = 0; y < kHeight; ++y) {
        for (int x = 0; x < kWidth; ++x) {
          int index = y * kWidth + x;
          Color actual = current.pixelAt(x, y);
          Color expected = reference.pixelAt(x, y);
          EXPECT_LE(current.device.writes[index], 1);
          if (current.device.writes[index] != 0 || y / 16 == band) {
            ASSERT_EQ(expected, actual) << x << "," << y << " frame " << frame;
          }
          ages[index] = actual == expected ? 0 : ages[index] + 1;
          ASSERT_LT(ages[index], (kHeight + 15) / 16);
        }
      }
    }
    for (int stop = 1; stop <= (kHeight + 15) / 16; ++stop) {
      current.optional->requestCompleteRedraw();
      current.app.refresh();
      for (int frame = 0; frame < stop; ++frame) {
        current.panel->scrollBy(0, -2);
        reference.panel->scrollBy(0, -2);
        current.app.refresh();
        reference.app.refresh();
      }
      current.device.reset();
      current.runNextPaint();
      EXPECT_FALSE(current.optional->cleanupPending());
      EXPECT_FALSE(current.app.root().isDirty());
      EXPECT_EQ(roo_time::Uptime::Max(),
                current.scheduler.getNearestExecutionTime());
      EXPECT_EQ(reference.pixels, current.pixels);
      for (uint16_t writes : current.device.writes) EXPECT_LE(writes, 1);
    }
  }
}

// Verifies content damage from a callback and fresh damage raised inside a
// complete cleanup survive the consumed old obligation and remain scheduled.
TEST(AcceleratedScrolling,
     ForeignDamageAndPaintTimeMutationRequireCompleteRepair) {
  LogWorld current(true, true);
  LogWorld reference(false, true);
  current.app.start();
  current.runNext();
  current.panel->scrollBy(0, -8);
  reference.panel->scrollBy(0, -8);
  current.app.refresh();
  reference.app.refresh();
  ASSERT_TRUE(current.optional->cleanupPending());
  current.content->during_paint = [&]() {
    current.content->invalidateInterior();
  };
  current.runNextPaint();
  EXPECT_TRUE(current.app.root().isDirty());
  EXPECT_NE(roo_time::Uptime::Max(),
            current.scheduler.getNearestExecutionTime());
  current.runNextPaint();
  EXPECT_FALSE(current.app.root().isDirty());
  EXPECT_EQ(reference.pixels, current.pixels);
  current.panel->setOnScrollPositionChanged(
      [&](ScrollPosition, ScrollPosition) {
        current.content->invalidateInterior();
      });
  current.panel->scrollBy(0, -8);
  reference.panel->scrollBy(0, -8);
  current.app.refresh();
  reference.app.refresh();
  EXPECT_FALSE(current.optional->cleanupPending());
  EXPECT_EQ(reference.pixels, current.pixels);
}

// Verifies explicit complete redraw overrides moving-frame advice, while an
// unlimited budget never opts a scroller into approximate output.
TEST(AcceleratedScrolling, ForcedCompleteAndUnlimitedBudgetsRemainExact) {
  LogWorld current(true, true);
  LogWorld reference(false, true);
  current.panel->scrollBy(0, -8);
  reference.panel->scrollBy(0, -8);
  current.optional->requestCompleteRedraw();
  current.app.refresh();
  reference.app.refresh();
  EXPECT_FALSE(current.optional->cleanupPending());
  EXPECT_EQ(reference.pixels, current.pixels);
  current.app.window().setAdvisoryPaintBudget(roo_time::Duration());
  current.panel->scrollBy(0, -8);
  reference.panel->scrollBy(0, -8);
  current.app.refresh();
  reference.app.refresh();
  EXPECT_FALSE(current.optional->cleanupPending());
  EXPECT_EQ(reference.pixels, current.pixels);
}

// Verifies a narrowed cleanup cannot discard the remaining viewport obligation.
TEST(AcceleratedScrolling, PartialCleanupReissuesFullViewportDamage) {
  LogWorld current(true, true);
  LogWorld reference(false, true);
  current.app.start();
  current.runNext();
  current.panel->scrollBy(0, -8);
  reference.panel->scrollBy(0, -8);
  current.app.refresh();
  reference.app.refresh();
  ASSERT_TRUE(current.optional->cleanupPending());
  Surface surface(current.device, 0, 0, Box(0, 16, 95, 31), false,
                  current.optional->effectiveBackground(), FillMode::kVisible,
                  BlendingMode::kSource);
  Canvas canvas(&surface);
  internal::ClipperState state;
  Clipper clipper(state, current.device, roo_time::Uptime::Now());
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  current.optional->paintWidgetContents(ctx);
  EXPECT_TRUE(current.optional->cleanupPending());
  EXPECT_TRUE(current.app.root().isDirty());
  current.runNextPaint();
  EXPECT_EQ(reference.pixels, current.pixels);
  EXPECT_FALSE(current.optional->cleanupPending());
  EXPECT_EQ(roo_time::Uptime::Max(),
            current.scheduler.getNearestExecutionTime());
}

// Verifies nested opt-in scrollers suspend their own fills and leave one
// cleanup owner; final settlement reconstructs both scenes using current
// inputs.
TEST(AcceleratedScrolling, NestedOptInHasOneCleanupOwner) {
  LogWorld current(true, true);
  LogWorld reference(false, true);
  auto inner = std::make_unique<ObservableAcceleratedPanel>(
      current.app.context(),
      std::make_unique<LogRows>(current.app.context(), true));
  ObservableAcceleratedPanel* nested = inner.get();
  current.content->addWidget(std::move(inner), Rect(25, 5, 85, 60));
  reference.content->addWidget(
      std::make_unique<SimpleScrollablePanel>(
          reference.app.context(),
          std::make_unique<LogRows>(reference.app.context(), true)),
      Rect(25, 5, 85, 60));
  current.optional->requestCompleteRedraw();
  current.app.refresh();
  reference.app.refresh();
  current.panel->scrollBy(0, -8);
  reference.panel->scrollBy(0, -8);
  current.app.refresh();
  reference.app.refresh();
  EXPECT_TRUE(current.optional->cleanupPending());
  EXPECT_FALSE(nested->cleanupPending());
  current.app.refresh();
  EXPECT_EQ(reference.pixels, current.pixels);
  EXPECT_FALSE(current.optional->cleanupPending());
}

// Verifies a held-still drag needs no release event for cleanup, and the final
// sampled fling position settles without requiring another animation sample.
TEST(AcceleratedScrolling, HeldStillDragAndTerminalFlingSettleWithoutInput) {
  LogWorld current(true, true);
  LogWorld reference(false, true);
  current.app.start();
  current.runNext();
  current.panel->onDragStart(0, 0);
  reference.panel->onDragStart(0, 0);
  current.panel->onDrag(0, 0, 0, -8);
  reference.panel->onDrag(0, 0, 0, -8);
  current.app.refresh();
  reference.app.refresh();
  ASSERT_TRUE(current.optional->cleanupPending());
  current.runNextPaint();
  EXPECT_EQ(reference.pixels, current.pixels);
  EXPECT_EQ(roo_time::Uptime::Max(),
            current.scheduler.getNearestExecutionTime());
  current.panel->onFling(0, 0, 0, -1800);
  reference.panel->onFling(0, 0, 0, -1800);
  // Establish the registry anchors before advancing custom animation time.
  current.app.refresh();
  reference.app.refresh();
  system_time_delay_micros(5000000);
  current.app.refresh();
  reference.app.refresh();
  // The overshooting terminal fling sample starts a spring-back. Finish that
  // final track too before checking cleanup alone returns the app to idle.
  system_time_delay_micros(600000);
  current.app.refresh();
  reference.app.refresh();
  if (current.optional->cleanupPending()) current.runNextPaint();
  EXPECT_EQ(reference.panel->getScrollPosition().y,
            current.panel->getScrollPosition().y);
  EXPECT_EQ(reference.pixels, current.pixels);
  EXPECT_EQ(roo_time::Uptime::Max(),
            current.scheduler.getNearestExecutionTime());
}

}  // namespace
}  // namespace roo_windows
