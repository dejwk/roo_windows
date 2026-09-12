#include <functional>

#include "gtest/gtest.h"
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/core/destination.h"
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
  return wrapper == nullptr ? Rect(0, 0, -1, -1)
                            : wrapper->parent_bounds();
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
  }

  uint32_t blitCalls() const { return blit_calls_; }
  uint32_t outputPixels() const { return output_pixels_; }
  uint32_t addressWindows() const { return address_windows_; }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    ++address_windows_;
    OffscreenDevice<Argb4444>::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* color, uint32_t pixel_count) override {
    output_pixels_ += pixel_count;
    OffscreenDevice<Argb4444>::write(color, pixel_count);
  }

  void fill(Color color, uint32_t pixel_count) override {
    output_pixels_ += pixel_count;
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

  void blitCopy(int16_t src_x0, int16_t src_y0, int16_t src_x1,
                int16_t src_y1, int16_t dst_x0, int16_t dst_y0) override {
    ++blit_calls_;
    OffscreenDevice<Argb4444>::blitCopy(src_x0, src_y0, src_x1, src_y1,
                                       dst_x0, dst_y0);
  }

 private:
  static uint32_t rectArea(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    return static_cast<uint32_t>(x1 - x0 + 1) *
           static_cast<uint32_t>(y1 - y0 + 1);
  }

  uint32_t blit_calls_ = 0;
  uint32_t output_pixels_ = 0;
  uint32_t address_windows_ = 0;
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

  bool refresh() { return app_.refresh(); }

  ApplicationContext& context() { return app_.context(); }

  roo::byte raster_[kWidth * kHeight * 2];
  CountingOffscreenDevice offscreen_;
  Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
};

// Verifies horizontal drag reveals the adjacent page strip with correct colors
// when the active-slot wrappers run on a blit-capable output.
TEST_F(HorizontalPageHostRenderTest, RevealedStripRepaintsWithBlitSupport) {
  auto host = std::make_unique<TestHorizontalPageHost>(context());
  TestHorizontalPageHost* host_ptr = host.get();

  host_ptr->addPage(std::make_unique<ColorBoxWidget>(context(), color::Red,
                                                     Dimensions(120, 60)));
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(context(), color::Blue,
                                                     Dimensions(120, 60)));

  app_.add(std::move(host), Box(0, 0, 119, 59));

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(110, 20));

  host_ptr->onDragStart(0, 0);
  host_ptr->onDrag(0, 0, -30, 0);

  offscreen_.resetCounters();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(1u, offscreen_.blitCalls());
  EXPECT_EQ(3600u, offscreen_.outputPixels());
  EXPECT_EQ(2u, offscreen_.addressWindows());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(110, 20));

  host_ptr->onDrag(0, 0, 130, 0);

  ASSERT_TRUE(refresh());
  EXPECT_NE(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(40, 20));
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
  ASSERT_TRUE(refresh());

  ASSERT_TRUE(host_ptr->setCurrentIndex(1));
  ASSERT_TRUE(refresh());
  delay(65);
  ASSERT_TRUE(refresh());
  const Rect interrupted = SlotBoundsForPage(*first_ptr);
  ASSERT_LT(interrupted.xMin(), 0);
  ASSERT_GT(interrupted.xMin(), -kWidth);

  host_ptr->onDragStart(0, 0);
  delay(220);
  ASSERT_TRUE(refresh());
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
  ASSERT_TRUE(refresh());

  ASSERT_TRUE(host_ptr->setCurrentIndex(1));
  ASSERT_TRUE(refresh());
  delay(65);
  ASSERT_TRUE(refresh());
  host_ptr->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  const Rect paused = SlotBoundsForPage(*first_ptr);

  delay(220);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(paused, SlotBoundsForPage(*first_ptr));
  EXPECT_EQ(0, host_ptr->currentIndex());

  host_ptr->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  delay(140);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(1, host_ptr->currentIndex());
  EXPECT_EQ(1, host_ptr->settledChangeCount());
}

// Verifies covering navigation cancels settling and returning silently applies
// the selected target without reporting an offscreen semantic completion.
TEST_F(HorizontalPageHostRenderTest,
       NavigationDetachReconcilesTargetWithoutCallback) {
  TestHorizontalPageHost host(context());
  host.addPage(std::make_unique<ColorBoxWidget>(
      context(), color::Red, Dimensions(kWidth, kHeight)));
  host.addPage(std::make_unique<ColorBoxWidget>(
      context(), color::Blue, Dimensions(kWidth, kHeight)));
  ColorBoxWidget covering(context(), color::Green,
                          Dimensions(kWidth, kHeight));
  WidgetDestination covering_destination(covering);
  Task& task = app_.addTaskFullScreen(host);
  ASSERT_TRUE(refresh());

  ASSERT_TRUE(host.setCurrentIndex(1));
  ASSERT_TRUE(refresh());
  delay(65);
  ASSERT_TRUE(refresh());
  task.navigation().push(covering_destination);
  ASSERT_TRUE(refresh());
  delay(220);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(0, host.currentIndex());

  task.navigation().pop();
  ASSERT_TRUE(refresh());
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
  host = std::make_unique<DeletingHorizontalPageHost>(context(),
                                                       delete_callback);
  host->addPage(std::make_unique<ColorBoxWidget>(
      context(), color::Red, Dimensions(kWidth, kHeight)));
  host->addPage(std::make_unique<ColorBoxWidget>(
      context(), color::Blue, Dimensions(kWidth, kHeight)));
  task = &app_.addTaskFullScreen(*host);
  ASSERT_TRUE(refresh());

  ASSERT_TRUE(host->setCurrentIndex(1));
  ASSERT_TRUE(refresh());
  delay(220);
  ASSERT_TRUE(refresh());
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
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);

  auto host = std::make_unique<TestHorizontalPageHost>(app.context());
  TestHorizontalPageHost* host_ptr = host.get();
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(app.context(), color::Red,
                                                     Dimensions(120, 60)));
  host_ptr->addPage(std::make_unique<ColorBoxWidget>(app.context(), color::Blue,
                                                     Dimensions(120, 60)));

  app.add(std::move(host), Box(0, 0, 119, 59));
  ASSERT_TRUE(app.refresh());

  int16_t px0[] = {10, 110};
  int16_t py0[] = {20, 20};
  Color colors0[2];
  offscreen.raster().readColors(px0, py0, 2, colors0);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors0[0]);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors0[1]);

  host_ptr->onDragStart(0, 0);
  host_ptr->onDrag(0, 0, -30, 0);
  ASSERT_TRUE(app.refresh());

  Color colors1[2];
  offscreen.raster().readColors(px0, py0, 2, colors1);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors1[0]);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), colors1[1]);
}

}  // namespace
}  // namespace roo_windows
