#include <array>
#include <functional>
#include <iterator>
#include <vector>

#include "gtest/gtest.h"
#include "roo_windows/containers/blit_cache_container.h"
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/panel.h"
#include "roo_windows_render_test_support.h"

using namespace roo_display;
using namespace roo_windows;
using namespace roo_windows::test_support;

namespace roo_windows {
namespace {

class TestHorizontalPageHost : public HorizontalPageHost {
 public:
  explicit TestHorizontalPageHost(ApplicationContext& context)
      : HorizontalPageHost(context) {}

  using HorizontalPageHost::onDrag;
  using HorizontalPageHost::onDragStart;
  using HorizontalPageHost::onFling;

  int settledChangeCount() const { return settled_change_count_; }

 protected:
  void onSettledIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    (void)new_index;
    ++settled_change_count_;
  }

 private:
  int settled_change_count_ = 0;
};

class WidgetDestination : public Destination {
 public:
  explicit WidgetDestination(Widget& contents) : contents_(contents) {}
  Widget& getContents() override { return contents_; }

 private:
  Widget& contents_;
};

class DeletingHorizontalPageHost : public HorizontalPageHost {
 public:
  DeletingHorizontalPageHost(ApplicationContext& context,
                             std::function<void()>& delete_callback)
      : HorizontalPageHost(context), delete_callback_(delete_callback) {}

 protected:
  void onSettledIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    (void)new_index;
    delete_callback_();
  }

 private:
  std::function<void()>& delete_callback_;
};

Rect SlotBoundsForPage(const Widget& page) {
  const Container* wrapper = page.parent();
  return wrapper == nullptr ? Rect(0, 0, -1, -1) : wrapper->parent_bounds();
}

// Models the display traffic relevant to a slow address-window device. Blits
// are framebuffer-local operations, so they are counted separately and do not
// contribute to the number of pixels sent to the display.
class CountingOffscreenDevice : public OffscreenDevice<Argb4444> {
 public:
  CountingOffscreenDevice(int16_t width, int16_t height, roo::byte* data,
                          const Argb4444& color_mode)
      : OffscreenDevice<Argb4444>(width, height, data, color_mode) {}

  void resetCounters() {
    blit_calls_ = 0;
    output_pixels_ = 0;
    address_windows_ = 0;
    output_writes_.fill(0);
    last_blit_source_ = Box(0, 0, -1, -1);
    last_blit_destination_ = Box(0, 0, -1, -1);
  }

  uint32_t blitCalls() const { return blit_calls_; }
  uint32_t outputPixels() const { return output_pixels_; }
  uint32_t addressWindows() const { return address_windows_; }
  const Box& lastBlitSource() const { return last_blit_source_; }
  const Box& lastBlitDestination() const { return last_blit_destination_; }
  uint16_t outputWritesAt(int16_t x, int16_t y) const {
    return output_writes_[y * 120 + x];
  }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    ++address_windows_;
    write_x0_ = write_x_ = x0;
    write_x1_ = x1;
    write_y_ = y0;
    OffscreenDevice<Argb4444>::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* color, uint32_t pixel_count) override {
    output_pixels_ += pixel_count;
    recordWrites(pixel_count);
    OffscreenDevice<Argb4444>::write(color, pixel_count);
  }

  void fill(Color color, uint32_t pixel_count) override {
    output_pixels_ += pixel_count;
    recordWrites(pixel_count);
    OffscreenDevice<Argb4444>::fill(color, pixel_count);
  }

  void writePixels(BlendingMode mode, Color* color, int16_t* x, int16_t* y,
                   uint16_t pixel_count) override {
    for (uint16_t i = 0; i < pixel_count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(&color[i], 1);
    }
  }

  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t pixel_count) override {
    for (uint16_t i = 0; i < pixel_count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* color, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color[i], rectArea(x0[i], y0[i], x1[i], y1[i]));
    }
  }

  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, rectArea(x0[i], y0[i], x1[i], y1[i]));
    }
  }

  void blitCopy(int16_t src_x0, int16_t src_y0, int16_t src_x1, int16_t src_y1,
                int16_t dst_x0, int16_t dst_y0) override {
    ++blit_calls_;
    last_blit_source_ = Box(src_x0, src_y0, src_x1, src_y1);
    last_blit_destination_ =
        Box(dst_x0, dst_y0, dst_x0 + src_x1 - src_x0, dst_y0 + src_y1 - src_y0);
    OffscreenDevice<Argb4444>::blitCopy(src_x0, src_y0, src_x1, src_y1, dst_x0,
                                        dst_y0);
  }

 private:
  static uint32_t rectArea(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    return static_cast<uint32_t>(x1 - x0 + 1) *
           static_cast<uint32_t>(y1 - y0 + 1);
  }

  void recordWrites(uint32_t count) {
    while (count-- != 0) {
      if (write_x_ < 120 && write_y_ < 60) {
        ++output_writes_[write_y_ * 120 + write_x_];
      }
      if (++write_x_ > write_x1_) {
        write_x_ = write_x0_;
        ++write_y_;
      }
    }
  }

  uint32_t blit_calls_ = 0;
  uint32_t output_pixels_ = 0;
  uint32_t address_windows_ = 0;
  std::array<uint16_t, 120 * 60> output_writes_{};
  Box last_blit_source_{0, 0, -1, -1};
  Box last_blit_destination_{0, 0, -1, -1};
  uint16_t write_x0_ = 0;
  uint16_t write_x1_ = 0;
  uint16_t write_x_ = 0;
  uint16_t write_y_ = 0;
};

class HorizontalPageHostRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 120;
  static constexpr int16_t kHeight = 60;

  HorizontalPageHostRenderTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color result[1];
    offscreen_.raster().readColors(px, py, 1, result);
    return result[0];
  }

  void refresh() { app_.refresh(); }

  ApplicationContext& context() { return app_.context(); }

  roo::byte raster_[kWidth * kHeight * 2];
  CountingOffscreenDevice offscreen_;
  Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
};

// Verifies a plain cache's height change preserves existing child pixels and
// repaints the exposed surface, without copying pixels from a stale cache.
TEST_F(HorizontalPageHostRenderTest,
       BlitCacheHeightResizePreservesChildPixels) {
  auto cache = std::make_unique<BlitCacheContainer>(context());
  BlitCacheContainer* cache_ptr = cache.get();
  cache->setChild(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                   Dimensions(120, 40)));
  app_.add(std::move(cache), Box(0, 0, 119, 39));
  refresh();

  for (int height : {50, 30, 45}) {
    SCOPED_TRACE(height);
    offscreen_.resetCounters();
    cache_ptr->moveTo(Rect(0, 0, 119, height - 1));
    refresh();
    EXPECT_EQ(0u, offscreen_.blitCalls());
    // Growth beyond the child must not redraw it. Shrinking intersects the
    // child; this simple test widget then repaints its whole visible surface.
    if (height == 50) {
      EXPECT_LT(offscreen_.outputPixels(), 120u * 40u);
    }
    EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 10));
    const std::vector<roo::byte> before(std::begin(raster_), std::end(raster_));
    app_.root().invalidateInterior();
    refresh();
    EXPECT_EQ(before,
              std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
  }
}

class RoundedTestPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  bool clipsChildrenToRoundedBounds() const override { return true; }
  BorderStyle getBorderStyle() const override { return BorderStyle(16, 0); }
};

class PaintInvalidatingBox : public ColorBoxWidget {
 public:
  using ColorBoxWidget::ColorBoxWidget;

  void invalidateDuringNextPaint(Rect damage) {
    paint_damage_ = damage;
    invalidate_during_paint_ = true;
  }

  void paint(PaintContext& ctx) const override {
    ColorBoxWidget::paint(ctx);
    if (!invalidate_during_paint_) return;
    invalidate_during_paint_ = false;
    const_cast<PaintInvalidatingBox*>(this)->invalidateInterior(paint_damage_);
  }

 private:
  mutable bool invalidate_during_paint_ = false;
  Rect paint_damage_{0, 0, -1, -1};
};

void ExpectCopiedDestinationWasNotRepainted(
    const CountingOffscreenDevice& output) {
  const Box& copied = output.lastBlitDestination();
  ASSERT_FALSE(copied.empty());
  for (int y = copied.yMin(); y <= copied.yMax(); ++y) {
    for (int x = copied.xMin(); x <= copied.xMax(); ++x) {
      EXPECT_EQ(0, output.outputWritesAt(x, y))
          << "copied destination repainted at " << x << ", " << y;
    }
  }
}

// Verifies a cache under an active rounded owner copies only its fully opaque
// interior. The copied destination is reserved before child painting, and a
// following reconstruction with no movement neither changes pixels nor repeats
// the consumed translation.
TEST_F(HorizontalPageHostRenderTest, RoundedOwnerCopiesSettledInteriorOnce) {
  auto owner = std::make_unique<RoundedTestPanel>(context());
  RoundedTestPanel* owner_ptr = owner.get();
  auto cache = std::make_unique<BlitCacheContainer>(context());
  BlitCacheContainer* moving = cache.get();
  auto contents = std::make_unique<ColorBoxWidget>(context(), Color(0xFF3769A5),
                                                   Dimensions(kWidth, 84));
  contents->setMargins(MarginSize::kNone);
  cache->setChild(std::move(contents));
  owner->add(std::move(cache), Rect(0, 0, kWidth - 1, 83));
  app_.add(std::move(owner), Box(0, 0, kWidth - 1, kHeight - 1));
  refresh();

  moving->moveTo(Rect(0, -8, kWidth - 1, 75));
  offscreen_.resetCounters();
  refresh();

  ASSERT_EQ(1u, offscreen_.blitCalls());
  ExpectCopiedDestinationWasNotRepainted(offscreen_);
  const std::vector<roo::byte> copied_frame(std::begin(raster_),
                                            std::end(raster_));

  offscreen_.resetCounters();
  owner_ptr->invalidateInterior();
  refresh();
  EXPECT_EQ(0u, offscreen_.blitCalls());
  EXPECT_EQ(copied_frame,
            std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));

  // The synchronous reconstruction established a fresh certificate before
  // traversing the cache, so a later movement can copy again.
  moving->moveTo(Rect(0, -16, kWidth - 1, 67));
  offscreen_.resetCounters();
  refresh();
  EXPECT_EQ(1u, offscreen_.blitCalls());
  ExpectCopiedDestinationWasNotRepainted(offscreen_);

  // A large move with no proven overlap consumes the translation and falls
  // back to ordinary painting. A later reconstruction must match exactly.
  moving->moveTo(Rect(0, 40, kWidth - 1, 123));
  offscreen_.resetCounters();
  refresh();
  EXPECT_EQ(0u, offscreen_.blitCalls());
  const std::vector<roo::byte> large_move_frame(std::begin(raster_),
                                                std::end(raster_));
  owner_ptr->invalidateInterior();
  refresh();
  EXPECT_EQ(large_move_frame,
            std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
}

// Verifies a certificate is published before child traversal, so damage raised
// during that traversal and another invalidation before the next refresh both
// remove stale source pixels from the subsequent copy.
TEST_F(HorizontalPageHostRenderTest,
       PaintTimeAndSuccessiveDamageShrinkRoundedSource) {
  auto owner = std::make_unique<RoundedTestPanel>(context());
  RoundedTestPanel* owner_ptr = owner.get();
  auto cache = std::make_unique<BlitCacheContainer>(context());
  BlitCacheContainer* moving = cache.get();
  auto contents = std::make_unique<PaintInvalidatingBox>(
      context(), Color(0xFF7146A8), Dimensions(kWidth, 84));
  PaintInvalidatingBox* contents_ptr = contents.get();
  contents->setMargins(MarginSize::kNone);
  cache->setChild(std::move(contents));
  owner->add(std::move(cache), Rect(0, 0, kWidth - 1, 83));
  app_.add(std::move(owner), Box(0, 0, kWidth - 1, kHeight - 1));
  refresh();

  const Rect paint_damage(0, 20, kWidth - 1, 31);
  const Rect later_damage(0, 40, kWidth - 1, 45);
  contents_ptr->invalidateDuringNextPaint(paint_damage);
  owner_ptr->invalidateInterior();
  refresh();
  contents_ptr->invalidateInterior(later_damage);

  moving->moveTo(Rect(0, -8, kWidth - 1, 75));
  offscreen_.resetCounters();
  refresh();

  ASSERT_EQ(1u, offscreen_.blitCalls());
  EXPECT_FALSE(offscreen_.lastBlitSource().intersects(paint_damage.asBox()));
  EXPECT_FALSE(offscreen_.lastBlitSource().intersects(later_damage.asBox()));
  ExpectCopiedDestinationWasNotRepainted(offscreen_);
  const std::vector<roo::byte> copied_frame(std::begin(raster_),
                                            std::end(raster_));
  owner_ptr->invalidateInterior();
  refresh();
  EXPECT_EQ(copied_frame,
            std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
}

// Verifies a rounded descendant moving with the cached composition retains the
// source certificate. A complete repaint must produce identical pixels.
TEST_F(HorizontalPageHostRenderTest, RoundedDescendantRetainsBlitReuse) {
  auto cache = std::make_unique<BlitCacheContainer>(context());
  BlitCacheContainer* moving = cache.get();
  auto rounded = std::make_unique<RoundedTestPanel>(context());
  rounded->add(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                Dimensions(100, 48)),
               Rect(0, 0, 99, 47));
  cache->setChild(std::move(rounded));
  app_.add(std::move(cache), Box(0, 0, 99, 47));
  refresh();
  for (int x : {4, 8, 2}) {
    moving->moveTo(Rect(x, 0, x + 99, 47));
    offscreen_.resetCounters();
    refresh();
    EXPECT_EQ(offscreen_.blitCalls(), 1u);
    const std::vector<roo::byte> before(std::begin(raster_), std::end(raster_));
    app_.root().invalidateInterior();
    refresh();
    EXPECT_EQ(before,
              std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
    // Ensure this cache participates in a complete paint before the next move;
    // the test then isolates movement reuse from root damage selection.
    moving->invalidateInterior();
    refresh();
  }
}

// Verifies retained masks from a foreground sibling remove its conservative
// bounds while still allowing an uncovered cache rectangle to be copied.
TEST_F(HorizontalPageHostRenderTest, RoundedForegroundAllowsUncoveredBlit) {
  auto cache = std::make_unique<BlitCacheContainer>(context());
  BlitCacheContainer* moving = cache.get();
  cache->setChild(std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                   Dimensions(110, 60)));
  app_.add(std::move(cache), Box(0, 0, 109, 59));
  refresh();
  auto rounded = std::make_unique<RoundedTestPanel>(context());
  rounded->add(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                Dimensions(32, 32)),
               Rect(0, 0, 31, 31));
  app_.add(std::move(rounded), Box(24, 8, 55, 39));
  refresh();
  moving->invalidateInterior();
  refresh();
  moving->moveTo(Rect(4, 0, 113, 59));
  offscreen_.resetCounters();
  refresh();
  EXPECT_EQ(offscreen_.blitCalls(), 1u);
  const std::vector<roo::byte> before(std::begin(raster_), std::end(raster_));
  app_.root().invalidateInterior();
  refresh();
  EXPECT_EQ(before,
            std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
}

// Verifies raw copies cannot bypass an ancestor's content effect, even when
// this child opts out of the rounded geometry and could otherwise cache pixels.
TEST_F(HorizontalPageHostRenderTest, OwnerEffectsDisableUnclippedBlitReuse) {
  auto owner = std::make_unique<RoundedTestPanel>(context());
  RoundedTestPanel* panel = owner.get();
  auto cache = std::make_unique<BlitCacheContainer>(context());
  BlitCacheContainer* moving = cache.get();
  cache->setParentClipMode(ParentClipMode::kUnclipped);
  cache->setChild(std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                   Dimensions(80, 40)));
  owner->add(std::move(cache), Rect(0, 0, 79, 39));
  app_.add(std::move(owner), Box(0, 0, 119, 59));
  refresh();
  moving->moveTo(Rect(4, 0, 83, 39));
  offscreen_.resetCounters();
  refresh();
  EXPECT_GT(offscreen_.blitCalls(), 0u);
  panel->setPressed(true);
  refresh();
  for (int x : {8, 2}) {
    moving->moveTo(Rect(x, 0, x + 79, 39));
    offscreen_.resetCounters();
    refresh();
    EXPECT_EQ(offscreen_.blitCalls(), 0u);
    const std::vector<roo::byte> before(std::begin(raster_), std::end(raster_));
    app_.root().invalidateInterior();
    refresh();
    EXPECT_EQ(before,
              std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
  }
}

// Verifies horizontal drag reveals the adjacent page strip with correct colors
// when the active-slot wrappers run on a blit-capable output.
TEST_F(HorizontalPageHostRenderTest, RevealedStripRepaintsWithBlitSupport) {
  auto host = std::make_unique<TestHorizontalPageHost>(context());
  TestHorizontalPageHost* host_ptr = host.get();

  // This traffic benchmark uses full-bleed pages so each exposed strip is
  // one rectangle. Margined page painting is checked separately below.
  auto red = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                              Dimensions(120, 60));
  auto blue = std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                               Dimensions(120, 60));
  red->setMargins(MarginSize::kNone);
  blue->setMargins(MarginSize::kNone);
  host_ptr->addPage(std::move(red));
  host_ptr->addPage(std::move(blue));

  app_.add(std::move(host), Box(0, 0, 119, 59));

  refresh();
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(110, 20));

  host_ptr->onDragStart(0, 0);
  host_ptr->onDrag(0, 0, -30, 0);

  offscreen_.resetCounters();
  refresh();
  EXPECT_EQ(1u, offscreen_.blitCalls());
  EXPECT_EQ(3600u, offscreen_.outputPixels());
  EXPECT_EQ(2u, offscreen_.addressWindows());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(110, 20));

  host_ptr->onDrag(0, 0, 130, 0);

  refresh();
  EXPECT_NE(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(40, 20));
}

// Verifies page margins expose the host background and a blitted partial
// repaint produces the same pixels as repainting the entire screen.
TEST_F(HorizontalPageHostRenderTest, MarginedPagesPreserveRepaintCorrectness) {
  auto host = std::make_unique<TestHorizontalPageHost>(context());
  TestHorizontalPageHost* host_ptr = host.get();
  host->addPage(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                 Dimensions(120, 60)));
  host->addPage(std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                 Dimensions(120, 60)));
  app_.add(std::move(host), Box(0, 0, 119, 59));
  refresh();
  EXPECT_NE(QuantizeToArgb4444(color::Red), pixelAt(1, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  host_ptr->onDragStart(0, 0);
  host_ptr->onDrag(0, 0, -30, 0);
  refresh();
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(110, 20));
  EXPECT_NE(QuantizeToArgb4444(color::Blue), pixelAt(91, 20));
  const std::vector<roo::byte> before(std::begin(raster_), std::end(raster_));
  app_.root().invalidateInterior();
  refresh();
  EXPECT_EQ(before,
            std::vector<roo::byte>(std::begin(raster_), std::end(raster_)));
}

// Verifies a drag takes over at the last applied registry sample and no stale
// settle update moves the page afterward.
TEST_F(HorizontalPageHostRenderTest, DragInterruptsSettleAtAppliedPosition) {
  auto host = std::make_unique<TestHorizontalPageHost>(context());
  TestHorizontalPageHost* host_ptr = host.get();
  auto first = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                Dimensions(kWidth, kHeight));
  ColorBoxWidget* first_ptr = first.get();
  host_ptr->addPage(std::move(first));
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(
      context(), color::Blue, Dimensions(kWidth, kHeight)));
  app_.add(std::move(host), Box(0, 0, kWidth - 1, kHeight - 1));
  refresh();

  ASSERT_TRUE(host_ptr->setCurrentIndex(1));
  refresh();
  delay(65);
  refresh();
  const Rect interrupted = SlotBoundsForPage(*first_ptr);
  ASSERT_LT(interrupted.xMin(), 0);
  ASSERT_GT(interrupted.xMin(), -kWidth);

  host_ptr->onDragStart(0, 0);
  delay(220);
  refresh();
  EXPECT_EQ(interrupted, SlotBoundsForPage(*first_ptr));
  EXPECT_EQ(0, host_ptr->currentIndex());
}

// Verifies hidden time is excluded from settling and the retained channel
// resumes from its frozen page position.
TEST_F(HorizontalPageHostRenderTest, SettlePausesWhileHidden) {
  auto host = std::make_unique<TestHorizontalPageHost>(context());
  TestHorizontalPageHost* host_ptr = host.get();
  auto first = std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                Dimensions(kWidth, kHeight));
  ColorBoxWidget* first_ptr = first.get();
  host_ptr->addPage(std::move(first));
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(
      context(), color::Blue, Dimensions(kWidth, kHeight)));
  app_.add(std::move(host), Box(0, 0, kWidth - 1, kHeight - 1));
  refresh();

  ASSERT_TRUE(host_ptr->setCurrentIndex(1));
  refresh();
  delay(65);
  refresh();
  host_ptr->setVisibility(Visibility::kInvisible);
  refresh();
  const Rect paused = SlotBoundsForPage(*first_ptr);

  delay(220);
  refresh();
  EXPECT_EQ(paused, SlotBoundsForPage(*first_ptr));
  EXPECT_EQ(0, host_ptr->currentIndex());

  host_ptr->setVisibility(Visibility::kVisible);
  refresh();
  delay(140);
  refresh();
  EXPECT_EQ(1, host_ptr->currentIndex());
  EXPECT_EQ(1, host_ptr->settledChangeCount());
}

// Verifies covering navigation cancels settling and returning silently applies
// the selected target without reporting an offscreen semantic completion.
TEST_F(HorizontalPageHostRenderTest,
       NavigationDetachReconcilesTargetWithoutCallback) {
  TestHorizontalPageHost host(context());
  host.addPage(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                Dimensions(kWidth, kHeight)));
  host.addPage(std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                Dimensions(kWidth, kHeight)));
  ColorBoxWidget covering(context(), color::Green, Dimensions(kWidth, kHeight));
  WidgetDestination covering_destination(covering);
  Task& task = app_.addTaskFullScreen(host);
  refresh();

  ASSERT_TRUE(host.setCurrentIndex(1));
  refresh();
  delay(65);
  refresh();
  task.navigation().push(covering_destination);
  refresh();
  delay(220);
  refresh();
  EXPECT_EQ(0, host.currentIndex());

  task.navigation().pop();
  refresh();
  EXPECT_EQ(1, host.currentIndex());
  EXPECT_EQ(0, host.settledChangeCount());
  task.navigation().clear();
}

// Verifies the settled-index notification can detach and delete the host. The
// completion hook must not touch host state after invoking user code.
TEST_F(HorizontalPageHostRenderTest, CompletionCallbackCanDeleteHost) {
  std::unique_ptr<DeletingHorizontalPageHost> host;
  Task* task = nullptr;
  bool deleted = false;
  std::function<void()> delete_callback = [&] {
    task->navigation().clear();
    host.reset();
    deleted = true;
  };
  host =
      std::make_unique<DeletingHorizontalPageHost>(context(), delete_callback);
  host->addPage(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                 Dimensions(kWidth, kHeight)));
  host->addPage(std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                 Dimensions(kWidth, kHeight)));
  task = &app_.addTaskFullScreen(*host);
  refresh();

  ASSERT_TRUE(host->setCurrentIndex(1));
  refresh();
  delay(220);
  refresh();
  EXPECT_TRUE(deleted);
  EXPECT_EQ(nullptr, host.get());
}

class NoBlitOffscreenDevice : public OffscreenDevice<Argb4444> {
 public:
  NoBlitOffscreenDevice(int16_t width, int16_t height, roo::byte* data,
                        const Argb4444& color_mode)
      : OffscreenDevice<Argb4444>(width, height, data, color_mode) {}

  const Capabilities& getCapabilities() const override {
    static const Capabilities kCaps(/*supports_blending=*/true,
                                    /*supports_blit_copy=*/false);
    return kCaps;
  }
};

// Verifies the same revealed-strip repaint remains correct when blit-copy is
// unavailable and wrappers must fall back to normal repaint.
TEST(HorizontalPageHostRender, RevealedStripRepaintsWithoutBlitSupport) {
  roo::byte raster[120 * 60 * 2];
  NoBlitOffscreenDevice offscreen(120, 60, raster, Argb4444());
  Display display(offscreen);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto host = std::make_unique<TestHorizontalPageHost>(app.context());
  TestHorizontalPageHost* host_ptr = host.get();
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(app.context(), color::Red,
                                                     Dimensions(120, 60)));
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(app.context(), color::Blue,
                                                     Dimensions(120, 60)));

  app.add(std::move(host), Box(0, 0, 119, 59));
  app.refresh();

  int16_t px0[] = {10, 110};
  int16_t py0[] = {20, 20};
  Color colors0[2];
  offscreen.raster().readColors(px0, py0, 2, colors0);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors0[0]);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors0[1]);

  host_ptr->onDragStart(0, 0);
  host_ptr->onDrag(0, 0, -30, 0);
  app.refresh();

  Color colors1[2];
  offscreen.raster().readColors(px0, py0, 2, colors1);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors1[0]);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), colors1[1]);
}

}  // namespace
}  // namespace roo_windows
