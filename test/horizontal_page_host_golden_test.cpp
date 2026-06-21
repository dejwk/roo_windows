#include "gtest/gtest.h"
#include "roo_windows/containers/horizontal_page_host.h"
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

  using HorizontalPageHost::onDown;
  using HorizontalPageHost::onScroll;
};

using HorizontalPageHostRenderTest =
    test_support::RooWindowsRenderTestSized<120, 60>;

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

  host_ptr->onDown(0, 0);
  host_ptr->onScroll(0, 0, -30, 0);

  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(color::Red), pixelAt(10, 20));
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), pixelAt(110, 20));
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

  host_ptr->onDown(0, 0);
  host_ptr->onScroll(0, 0, -30, 0);
  ASSERT_TRUE(app.refresh());

  Color colors1[2];
  offscreen.raster().readColors(px0, py0, 2, colors1);
  EXPECT_EQ(QuantizeToArgb4444(color::Red), colors1[0]);
  EXPECT_EQ(QuantizeToArgb4444(color::Blue), colors1[1]);
}

}  // namespace
}  // namespace roo_windows
