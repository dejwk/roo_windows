#include <array>
#include <cstdint>
#include <cstdio>
#include <memory>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/shape/smooth.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/blit_plan.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/surface_widget.h"

namespace roo_windows::internal {

struct DamageRegionRenderTestPeer {
  static bool exclusionsPrepared(const ClipperOutput& out) {
    return out.exclusions_valid_;
  }

  static size_t preparedCount(const ClipperState& state) {
    return state.bounded_exclusions_.size();
  }

  static size_t preparedCapacity(const ClipperState& state) {
    return state.bounded_exclusions_.capacity();
  }
};

}  // namespace roo_windows::internal

namespace roo_windows {
namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Box;
using roo_display::Color;

constexpr int kWidth = 32;
constexpr int kHeight = 24;

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

class CountingWidget : public SurfaceWidget {
 public:
  explicit CountingWidget(ApplicationContext& context, uint8_t elevation = 0)
      : SurfaceWidget(context), elevation_(elevation) {}

  uint8_t getElevation() const override { return elevation_; }

  Color background() const override { return Color(0xFF123456); }

  Dimensions getSuggestedMinimumDimensions() const override { return {10, 10}; }

  void paint(PaintContext& ctx) const override {
    ++paint_calls;
    ctx.clear();
  }

  mutable int paint_calls = 0;

 private:
  uint8_t elevation_;
};

class ChildPaintingPanel : public Panel {
 public:
  using Panel::Panel;

  void addChild(std::unique_ptr<Widget> child, const Rect& bounds) {
    add(std::move(child), bounds);
  }

  void paintChildrenForTest(PaintContext& ctx) { paintChildren(ctx); }

 protected:
  bool respectsChildrenBoundaries() const override { return false; }
};

void DrawMethod(internal::ClipperOutput& out, int method) {
  const Color paint(0xFF123456);
  if (method == 0 || method == 1) {
    out.setAddress(0, 0, kWidth - 1, kHeight - 1, BlendingMode::kSource);
    std::array<Color, kWidth * kHeight> colors;
    colors.fill(paint);
    if (method == 0) {
      out.write(colors.data(), colors.size());
    } else {
      out.fill(paint, colors.size());
    }
  } else if (method == 2 || method == 3) {
    for (int16_t y = 0; y < kHeight; ++y) {
      std::array<int16_t, kWidth> xs;
      std::array<int16_t, kWidth> ys;
      std::array<Color, kWidth> colors;
      for (int16_t x = 0; x < kWidth; ++x) {
        xs[x] = x;
        ys[x] = y;
        colors[x] = paint;
      }
      if (method == 2) {
        out.writePixels(BlendingMode::kSource, colors.data(), xs.data(),
                        ys.data(), kWidth);
      } else {
        out.fillPixels(BlendingMode::kSource, paint, xs.data(), ys.data(),
                       kWidth);
      }
    }
  } else if (method == 4 || method == 5) {
    int16_t x0 = 0;
    int16_t y0 = 0;
    int16_t x1 = kWidth - 1;
    int16_t y1 = kHeight - 1;
    Color color = paint;
    if (method == 4) {
      out.writeRects(BlendingMode::kSource, &color, &x0, &y0, &x1, &y1, 1);
    } else {
      out.fillRects(BlendingMode::kSource, paint, &x0, &y0, &x1, &y1, 1);
    }
  } else {
    std::array<roo::byte, kWidth * kHeight * 4> source{};
    out.drawDirectRect(source.data(), kWidth * 4, 0, 0, kWidth - 1, kHeight - 1,
                       0, 0);
  }
}

// Verifies every output family writes each damaged pixel once and no gap pixel.
TEST(DamageRegionRenderTest, PreparedGapsFilterAllOutputMethods) {
  const Box full(0, 0, kWidth - 1, kHeight - 1);
  internal::DamageRegion damage;
  damage.add(Box(2, 2, 3, 3));
  damage.add(Box(28, 20, 29, 21));
  for (int method = 0; method < 7; ++method) {
    std::array<roo::byte, kWidth * kHeight * 4> pixels{};
    RecordingDevice device(pixels.data());
    internal::ClipperState state;
    internal::ClipperOutput out(state, device, &damage);
    out.setBounds(full);
    DrawMethod(out, method);
    EXPECT_EQ(internal::DamageRegionRenderTestPeer::preparedCount(state), 7u);
    const size_t capacity =
        internal::DamageRegionRenderTestPeer::preparedCapacity(state);
    EXPECT_GE(capacity, 7u);
    if (method == 0) {
      std::printf("P8_HOST_GAP_CAPACITY gaps=7 capacity=%zu bytes=%zu\n",
                  capacity, capacity * sizeof(Box));
    }
    for (int y = 0; y < kHeight; ++y) {
      for (int x = 0; x < kWidth; ++x) {
        const bool damaged = damage.covers(Box(x, y, x, y));
        EXPECT_EQ(device.writes[y * kWidth + x], damaged ? 1 : 0);
      }
    }
  }
}

// Verifies direct rejection skips preparation and a later clip expansion
// rebuilds the exact output inputs.
TEST(DamageRegionRenderTest, DirectClipRejectionAndExpansion) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  internal::DamageRegion damage;
  damage.add(Box(2, 2, 3, 3));
  damage.add(Box(28, 20, 29, 21));
  internal::ClipperState state;
  internal::ClipperOutput out(state, device, &damage);

  EXPECT_TRUE(out.prepareClip(Box(10, 10, 19, 15)).empty());
  EXPECT_FALSE(internal::DamageRegionRenderTestPeer::exclusionsPrepared(out));
  EXPECT_EQ(internal::DamageRegionRenderTestPeer::preparedCount(state), 0u);

  EXPECT_EQ(out.prepareClip(Box(0, 0, 7, 7)), Box(2, 2, 3, 3));
  EXPECT_EQ(internal::DamageRegionRenderTestPeer::preparedCount(state), 0u);
  EXPECT_EQ(out.prepareClip(Box(0, 0, 31, 23)), Box(2, 2, 29, 21));
  EXPECT_EQ(internal::DamageRegionRenderTestPeer::preparedCount(state), 3u);
  const auto overlay =
      roo_display::SmoothFilledRoundRect(2, 2, 3, 3, 0, Color(0x80204060));
  out.addOverlay(&overlay, Box(0, 0, 31, 23));
  EXPECT_TRUE(internal::DamageRegionRenderTestPeer::exclusionsPrepared(out));
  out.fillRect(BlendingMode::kSource, Box(0, 0, 31, 23), Color(0xFF123456));
  EXPECT_EQ(internal::DamageRegionRenderTestPeer::preparedCount(state), 3u);
  out.addExclusion(Box(2, 2, 3, 3));
  EXPECT_EQ(out.prepareClip(Box(0, 0, 31, 23)), Box(28, 20, 29, 21));
  EXPECT_EQ(out.exclusions().size(), 1u);
}

// The same masks, semantic exclusions and translucent foreground overlays
// must produce the same damaged pixels as an unrestricted paint.
TEST(DamageRegionRenderTest, DamageComposesWithMasksAndForegroundEffects) {
  const Box full(0, 0, kWidth - 1, kHeight - 1);
  internal::DamageRegion damage;
  damage.add(Box(1, 1, 5, 5));
  damage.add(Box(25, 17, 30, 22));
  std::array<roo::byte, kWidth * kHeight * 4> normal_pixels{};
  std::array<roo::byte, kWidth * kHeight * 4> sparse_pixels{};
  RecordingDevice normal(normal_pixels.data());
  RecordingDevice sparse(sparse_pixels.data());

  auto paint = [&](RecordingDevice& device,
                   const internal::DamageRegion* active) {
    internal::ClipperState state;
    internal::ClipperOutput out(state, device, active);
    const auto overlay = roo_display::SmoothFilledRoundRect(
        0, 0, kWidth - 1, kHeight - 1, 0, Color(0x80204060));
    out.addOverlay(&overlay, full);
    auto& mask = out.prepareRoundedClip(&state, full, BorderStyle(5, 0));
    out.activateRoundedClip(mask);
    out.addExclusion(Box(0, 0, 7, 7));
    out.deactivateRoundedClip();
    ASSERT_TRUE(out.exclusions().empty());
    ASSERT_EQ(out.maskedExclusions().size(), 1u);
    out.setBounds(full);
    out.fillRect(BlendingMode::kSource, full, Color(0xFF123456));
    EXPECT_EQ(out.exclusions().size(), 0u);
    EXPECT_EQ(out.maskedExclusions().size(), 1u);
  };
  paint(normal, nullptr);
  paint(sparse, &damage);

  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const int index = y * kWidth + x;
      if (damage.covers(Box(x, y, x, y))) {
        EXPECT_EQ(sparse.writes[index], normal.writes[index]);
        EXPECT_EQ(sparse_pixels[index * 4 + 0], normal_pixels[index * 4 + 0]);
        EXPECT_EQ(sparse_pixels[index * 4 + 1], normal_pixels[index * 4 + 1]);
        EXPECT_EQ(sparse_pixels[index * 4 + 2], normal_pixels[index * 4 + 2]);
        EXPECT_EQ(sparse_pixels[index * 4 + 3], normal_pixels[index * 4 + 3]);
      } else {
        EXPECT_EQ(sparse.writes[index], 0);
      }
    }
  }
}

// Verifies absent damage preserves unrestricted output while an empty region
// suppresses it, including direct output without prepareClip().
TEST(DamageRegionRenderTest, NullAndEmptyDamageHaveDistinctMeaning) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  internal::ClipperState state;
  internal::DamageRegion empty;
  {
    internal::ClipperOutput out(state, device);
    out.setBounds(Box(0, 0, 31, 23));
    out.fillRect(BlendingMode::kSource, Box(0, 0, 31, 23), Color(0xFF123456));
    EXPECT_EQ(device.writes[10 * kWidth + 10], 1);
  }
  device.reset();
  {
    internal::ClipperOutput out(state, device, &empty);
    EXPECT_TRUE(out.prepareClip(Box(0, 0, 31, 23)).empty());
    out.setBounds(Box(0, 0, 31, 23));
    out.fillRect(BlendingMode::kSource, Box(0, 0, 31, 23), Color(0xFF123456));
    for (uint8_t count : device.writes) EXPECT_EQ(count, 0);
  }
}

// Verifies a widget inside the damage envelope's gap skips its paint hook,
// while intersecting damage still reaches the same hook.
TEST(DamageRegionRenderTest, WidgetSkipsContentBeforeGapPreparation) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  auto parent = std::make_unique<ChildPaintingPanel>(app.context());
  ChildPaintingPanel* parent_ptr = parent.get();
  auto widget = std::make_unique<CountingWidget>(app.context());
  CountingWidget* target = widget.get();
  parent->addChild(std::move(widget), Rect(10, 10, 19, 19));
  app.add(std::move(parent), Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();
  target->paint_calls = 0;
  target->invalidateInterior();

  roo_display::Surface surface(device, 0, 0, Box(0, 0, kWidth - 1, kHeight - 1),
                               false, display.getBackgroundColor(),
                               roo_display::FillMode::kVisible,
                               BlendingMode::kSourceOver);
  internal::DamageRegion gap_damage;
  gap_damage.add(Box(0, 0, 2, 2));
  gap_damage.add(Box(28, 20, 31, 23));
  internal::ClipperState state;
  {
    Clipper clipper(state, device, roo_time::Uptime::Max(), &gap_damage);
    Canvas canvas(&surface);
    canvas.set_out(clipper.out());
    PaintContext ctx(canvas, clipper);
    parent_ptr->paintChildrenForTest(ctx);
    EXPECT_EQ(target->paint_calls, 0);
    EXPECT_EQ(internal::DamageRegionRenderTestPeer::preparedCount(state), 0u);
  }

  target->invalidateInterior();
  internal::DamageRegion nearby_damage;
  nearby_damage.add(Box(12, 12, 13, 13));
  {
    Clipper clipper(state, device, roo_time::Uptime::Max(), &nearby_damage);
    Canvas canvas(&surface);
    canvas.set_out(clipper.out());
    PaintContext ctx(canvas, clipper);
    parent_ptr->paintChildrenForTest(ctx);
    EXPECT_EQ(target->paint_calls, 1);
  }
}

// Raw copy bypasses filters, so a legal source outside active damage is copied
// only into damage. The regular paint supplies remaining damage without
// repainting the copied rectangle.
TEST(DamageRegionRenderTest, RawCopyAndFilteredRemainderMatchPixelOracle) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  std::array<Color, kWidth * kHeight> original;
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const Color color(0xFF000000u | (x << 16) | (y << 8) | (x ^ y));
      original[y * kWidth + x] = color;
      device.setAddress(x, y, x, y, BlendingMode::kSource);
      device.fill(color, 1);
    }
  }
  device.reset();
  const Box full(0, 0, kWidth - 1, kHeight - 1);
  internal::DamageRegion damage;
  damage.add(Box(16, 0, 31, 23));
  internal::ClipperState state;
  Clipper clipper(state, device, roo_time::Uptime::Max(), &damage);
  const internal::BlitPlan plan = clipper.planBlitCopy(full, full, 4, 0);
  ASSERT_FALSE(plan.empty());
  EXPECT_LT(plan.source.xMin(), damage.bounds().xMin());
  EXPECT_TRUE(damage.covers(plan.destination));
  clipper.addExclusion(plan.destination);
  clipper.rawOut().blitCopy(plan.source.xMin(), plan.source.yMin(),
                            plan.source.xMax(), plan.source.yMax(),
                            plan.destination.xMin(), plan.destination.yMin());
  clipper.setBounds(full);
  const Color fresh(0xFF2488CC);
  clipper.out()->fillRect(BlendingMode::kSource, full, fresh);
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const int index = y * kWidth + x;
      const bool damaged = damage.covers(Box(x, y, x, y));
      EXPECT_EQ(device.writes[index], damaged ? 1 : 0);
      const Color expected = plan.destination.contains(x, y)
                                 ? original[y * kWidth + x - 4]
                             : damaged ? fresh
                                       : original[index];
      int16_t px = x;
      int16_t py = y;
      Color actual;
      device.raster().readColors(&px, &py, 1, &actual);
      EXPECT_EQ(actual, expected);
    }
  }
}

// A shadow that intersects damage outside content must still be registered
// and its eventual output must remain inside the active damage union.
TEST(DamageRegionRenderTest, DecorationOverflowReachesDamage) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  auto parent = std::make_unique<ChildPaintingPanel>(app.context());
  ChildPaintingPanel* parent_ptr = parent.get();
  auto widget = std::make_unique<CountingWidget>(app.context(), 8);
  CountingWidget* target = widget.get();
  parent->addChild(std::move(widget), Rect(10, 10, 19, 19));
  app.add(std::move(parent), Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();
  ASSERT_GT(target->getDecorationBounds().yMax(), target->bounds().yMax());
  device.reset();
  target->invalidateInterior();

  roo_display::Surface surface(device, 0, 0, Box(0, 0, kWidth - 1, kHeight - 1),
                               false, display.getBackgroundColor(),
                               roo_display::FillMode::kVisible,
                               BlendingMode::kSourceOver);
  internal::DamageRegion damage;
  damage.add(Box(12, 20, 17, 20));
  internal::ClipperState state;
  Clipper clipper(state, device, roo_time::Uptime::Max(), &damage);
  Canvas canvas(&surface);
  canvas.set_out(clipper.out());
  PaintContext ctx(canvas, clipper);
  parent_ptr->paintChildrenForTest(ctx);
  EXPECT_FALSE(
      static_cast<internal::ClipperOutput*>(clipper.out())->overlays().empty());
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      EXPECT_EQ(device.writes[y * kWidth + x],
                damage.covers(Box(x, y, x, y)) ? 1 : 0);
    }
  }
  ctx.setClipBox(Box(0, 0, kWidth - 1, kHeight - 1));
  ctx.clear();
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const bool damaged = damage.covers(Box(x, y, x, y));
      EXPECT_EQ(device.writes[y * kWidth + x], damaged ? 2 : 0);
    }
  }
}

class DeferredOverlay : public Widget {
 public:
  explicit DeferredOverlay(ApplicationContext& context, int overflow = 0)
      : Widget(context), overflow_(overflow) {}

  Insets getInkInsets() const override {
    return Insets(-overflow_, -overflow_, -overflow_, -overflow_);
  }
  Dimensions getSuggestedMinimumDimensions() const override { return {12, 8}; }
  void setColor(Color color) {
    color_ = color;
    invalidateInterior();
  }

  void invalidateInterior() override {
    invalidateInterior(getInteriorBounds());
  }

  void invalidateInterior(const Rect& rect) override {
    Widget::invalidateInterior(rect);
    notifyParentInvalidatedRegion(rect.translate(offsetLeft(), offsetTop()));
  }

 protected:
  Rect getDirectPaintExclusionBounds() const override { return {0, 0, -1, -1}; }
  void emitPersistentDecoration(PaintContext& ctx) const override {
    const Rect ink = getContentBounds();
    ctx.addOverlayShape(roo_display::SmoothFilledRoundRect(
                            ink.xMin() - 0.5f, ink.yMin() - 0.5f,
                            ink.xMax() + 0.5f, ink.yMax() + 0.5f, 0, color_),
                        ink);
  }

 private:
  Color color_{128, 255, 0, 0};
  int overflow_;
};

// Verifies pre-paint invalidation reconstructs lower contributors and
// overlapping translucent foregrounds in the same frame, once per pixel,
// without scheduling a follow-up refresh or disturbing a disjoint static area.
TEST(DamageRegionRenderTest,
     ForegroundInvalidationReconstructsBackdropInOnePass) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  app.refresh();
  auto panel = std::make_unique<ChildPaintingPanel>(app.context());
  auto lower = std::make_unique<CountingWidget>(app.context());
  CountingWidget* lower_ptr = lower.get();
  panel->addChild(std::move(lower), Rect(0, 0, kWidth - 1, kHeight - 1));
  auto first = std::make_unique<DeferredOverlay>(app.context());
  DeferredOverlay* first_ptr = first.get();
  panel->addChild(std::move(first), Rect(2, 2, 13, 9));
  auto second = std::make_unique<DeferredOverlay>(app.context());
  DeferredOverlay* second_ptr = second.get();
  panel->addChild(std::move(second), Rect(10, 6, 21, 13));
  app.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();
  lower_ptr->paint_calls = 0;
  first_ptr->setColor(Color(160, 0, 255, 0));
  second_ptr->setColor(Color(180, 0, 0, 255));
  EXPECT_TRUE(first_ptr->isInvalidated());
  EXPECT_TRUE(second_ptr->isInvalidated());
  EXPECT_TRUE(lower_ptr->isInvalidated());
  device.reset();
  app.refresh();
  EXPECT_EQ(lower_ptr->paint_calls, 1);
  for (uint8_t count : device.writes) EXPECT_LE(count, 1);
  EXPECT_EQ(device.writes[22 * kWidth + 30], 0);
  const auto incremental = pixels;
  device.reset();
  app.refresh();
  for (uint8_t count : device.writes) EXPECT_EQ(count, 0);
  app.root().invalidateInterior();
  device.reset();
  app.refresh();
  EXPECT_EQ(incremental, pixels);
  for (uint8_t count : device.writes) EXPECT_LE(count, 1);
}

class BackdropPanel : public ChildPaintingPanel {
 public:
  BackdropPanel(ApplicationContext& context, Color color, bool rounded)
      : ChildPaintingPanel(context), color_(color), rounded_(rounded) {}

  Color background() const override { return color_; }

  bool fullyCoversBoundsWithOpaqueColors() const override {
    return color_.isOpaque() && !rounded_;
  }

  bool clipsChildrenToRoundedBounds() const override { return rounded_; }

  BorderStyle getBorderStyle() const override {
    return BorderStyle(rounded_ ? 6 : 0, 0);
  }

 private:
  Color color_;
  bool rounded_;
};

// Models the keyboard's private damage clip as well as a changing pattern.
class PatternedBackdrop : public Widget {
 public:
  explicit PatternedBackdrop(ApplicationContext& context) : Widget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return {kWidth, kHeight};
  }

  void setAlternate() {
    alternate_ = !alternate_;
    setDirty();
  }

  Color colorAt(int x, int y) const {
    return ((x / 4 + y / 4) % 2 == 0) != alternate_ ? Color(0xFFE0F0C0)
                                                    : Color(0xFF204060);
  }

  int paint_calls = 0;

 protected:
  void invalidateDescending() override {
    Widget::invalidateDescending();
    pending_ = bounds();
  }

  void invalidateDescending(const Rect& rect) override {
    Widget::invalidateDescending(rect);
    const Rect clipped = Rect::Intersect(rect, bounds());
    if (clipped.empty()) return;
    pending_ = pending_.empty() ? clipped : Rect::Extent(pending_, clipped);
  }

  void paintWidgetContents(PaintContext& ctx) override {
    if (!isDirty()) return;
    ++paint_calls;
    PaintContext local = ctx.clipped(pending_.empty() ? bounds() : pending_);
    Widget::paintWidgetContents(local);
    pending_ = Rect(0, 0, -1, -1);
  }

  void paint(PaintContext& ctx) const override {
    const auto pattern = roo_display::MakeRasterizable(
        bounds().asBox(),
        [this](int16_t x, int16_t y) { return colorAt(x, y); });
    ctx.drawObject(pattern);
  }

 private:
  Rect pending_{0, 0, -1, -1};
  bool alternate_ = false;
};

// Independent source-over arithmetic for these fully covered rectangular
// layers.
Color CompositeOver(Color backdrop, Color foreground) {
  const auto channel = [foreground](int back, int front) {
    return (back * (255 - foreground.a()) + front * foreground.a() + 127) / 255;
  };
  return Color(255, channel(backdrop.r(), foreground.r()),
               channel(backdrop.g(), foreground.g()),
               channel(backdrop.b(), foreground.b()));
}

// Verifies invalidation reaches lower widgets' private dirty clips before
// painting, including disjoint ordinary damage and overlapping alpha sources.
TEST(DamageRegionRenderTest, DeferredInvalidationMergesPrivateDamage) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  app.refresh();
  auto panel = std::make_unique<ChildPaintingPanel>(app.context());
  auto lower = std::make_unique<PatternedBackdrop>(app.context());
  PatternedBackdrop* lower_ptr = lower.get();
  panel->addChild(std::move(lower), Rect(0, 0, kWidth - 1, kHeight - 1));
  auto first = std::make_unique<DeferredOverlay>(app.context());
  DeferredOverlay* first_ptr = first.get();
  const Rect first_bounds(2, 2, 13, 9);
  panel->addChild(std::move(first), first_bounds);
  auto second = std::make_unique<DeferredOverlay>(app.context());
  DeferredOverlay* second_ptr = second.get();
  const Rect second_bounds(10, 6, 21, 13);
  panel->addChild(std::move(second), second_bounds);
  app.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();
  lower_ptr->paint_calls = 0;
  const Rect ordinary(26, 18, 29, 21);
  device.fillRect(26, 18, 29, 21, roo_display::color::Magenta);
  lower_ptr->invalidateInterior(ordinary);
  const Color first_color(160, 0, 255, 0);
  const Color second_color(180, 0, 0, 255);
  first_ptr->setColor(first_color);
  second_ptr->setColor(second_color);
  device.reset();
  app.refresh();
  EXPECT_EQ(lower_ptr->paint_calls, 1);
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color expected = lower_ptr->colorAt(x, y);
      if (first_bounds.contains(x, y)) {
        expected = CompositeOver(expected, first_color);
      }
      if (second_bounds.contains(x, y)) {
        expected = CompositeOver(expected, second_color);
      }
      Color actual;
      device.raster().readColors(&x, &y, 1, &actual);
      // The compositor can round the combined overlays before the final blend.
      EXPECT_NEAR(actual.r(), expected.r(), 1) << x << "," << y;
      EXPECT_NEAR(actual.g(), expected.g(), 1) << x << "," << y;
      EXPECT_NEAR(actual.b(), expected.b(), 1) << x << "," << y;
      EXPECT_LE(device.writes[y * kWidth + x], 1);
      if (first_bounds.contains(x, y) || second_bounds.contains(x, y) ||
          ordinary.contains(x, y)) {
        EXPECT_EQ(device.writes[y * kWidth + x], 1);
      }
    }
  }
  device.reset();
  app.refresh();
  for (uint8_t count : device.writes) EXPECT_EQ(count, 0);
}

// Verifies clean foreground registration, unchanged-value invalidation, and
// removal of declared ink outside layout bounds over a changing patterned leaf.
TEST(DamageRegionRenderTest, DeferredOverflowAndSourceOnlyInvalidation) {
  std::array<roo::byte, kWidth * kHeight * 4> pixels{};
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  app.refresh();
  auto panel = std::make_unique<ChildPaintingPanel>(app.context());
  auto lower = std::make_unique<PatternedBackdrop>(app.context());
  PatternedBackdrop* lower_ptr = lower.get();
  panel->addChild(std::move(lower), Rect(0, 0, kWidth - 1, kHeight - 1));
  auto foreground = std::make_unique<DeferredOverlay>(app.context(), 2);
  DeferredOverlay* foreground_ptr = foreground.get();
  panel->addChild(std::move(foreground), Rect(6, 6, 17, 13));
  app.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  app.refresh();
  lower_ptr->setAlternate();
  EXPECT_FALSE(foreground_ptr->isDirty());
  device.reset();
  app.refresh();
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color expected = lower_ptr->colorAt(x, y);
      if (foreground_ptr->getParentContentBounds().contains(x, y)) {
        expected = CompositeOver(expected, Color(128, 255, 0, 0));
      }
      Color actual;
      device.raster().readColors(&x, &y, 1, &actual);
      EXPECT_EQ(actual, expected) << x << "," << y;
      EXPECT_EQ(device.writes[y * kWidth + x], 1);
    }
  }
  const auto expected_frame = pixels;
  device.fillRect(4, 4, 8, 8, roo_display::color::Magenta);
  Widget* source = foreground_ptr;
  source->invalidateInterior(Rect(-2, -2, 2, 2));
  device.reset();
  app.refresh();
  EXPECT_EQ(pixels, expected_frame);
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      EXPECT_EQ(device.writes[y * kWidth + x], Rect(4, 4, 8, 8).contains(x, y));
    }
  }
  foreground_ptr->setColor(roo_display::color::Transparent);
  device.reset();
  app.refresh();
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color actual;
      device.raster().readColors(&x, &y, 1, &actual);
      EXPECT_EQ(actual, lower_ptr->colorAt(x, y));
      EXPECT_LE(device.writes[y * kWidth + x], 1);
    }
  }
  device.reset();
  app.refresh();
  for (uint8_t count : device.writes) EXPECT_EQ(count, 0);
}

// Verifies backdrop invalidation crosses transparent ancestors, restores
// rounded corner coverage, and honors unclipped children and sibling stacking
// groups.
TEST(DamageRegionRenderTest, ForegroundInvalidationReconstructsNestedBackdrop) {
  for (bool rounded : {false, true}) {
    for (bool unclipped : {false, true}) {
      SCOPED_TRACE(rounded);
      SCOPED_TRACE(unclipped);
      std::array<roo::byte, kWidth * kHeight * 4> pixels{};
      RecordingDevice device(pixels.data());
      roo_display::Display display(device);
      roo_scheduler::SchedulingService scheduler;
      Environment environment(scheduler);
      Application app(&environment, display);
      app.refresh();
      auto root = std::make_unique<ChildPaintingPanel>(app.context());
      root->addChild(std::make_unique<CountingWidget>(app.context()),
                     Rect(0, 0, kWidth - 1, kHeight - 1));
      auto nested = std::make_unique<BackdropPanel>(
          app.context(), Color(100, 50, 180, 90), rounded);
      auto foreground = std::make_unique<DeferredOverlay>(app.context());
      DeferredOverlay* foreground_ptr = foreground.get();
      if (unclipped) {
        foreground->setParentClipMode(ParentClipMode::kUnclipped);
      }
      nested->addChild(std::move(foreground), Rect(-2, -2, 9, 5));
      // A later clipped sibling remains below the unclipped foreground.
      nested->addChild(std::make_unique<CountingWidget>(app.context()),
                       Rect(0, 0, 5, 5));
      root->addChild(std::move(nested), Rect(4, 3, 27, 20));
      app.add(std::move(root), Box(0, 0, kWidth - 1, kHeight - 1));
      app.refresh();
      foreground_ptr->setColor(Color(160, 0, 255, 0));
      device.reset();
      app.refresh();
      const auto incremental = pixels;
      for (uint8_t count : device.writes) EXPECT_LE(count, 1);
      EXPECT_EQ(device.writes[22 * kWidth + 30], 0);
      app.root().invalidateInterior();
      device.reset();
      app.refresh();
      EXPECT_EQ(incremental, pixels);
      for (uint8_t count : device.writes) EXPECT_LE(count, 1);
    }
  }
}

}  // namespace
}  // namespace roo_windows
