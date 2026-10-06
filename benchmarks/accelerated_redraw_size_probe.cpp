// Link with the optional scroller or its ordinary base to compare host code
// size.
#include <array>
#include <cstdio>
#include <cstdlib>

#include "roo_display/core/offscreen.h"
#include "roo_windows.h"
#include "roo_windows/containers/scrollable_panel.h"
#ifndef ROO_REDRAW_BASELINE
#include "roo_windows/containers/accelerated_scrollable_panel.h"
#endif

namespace roo_windows {
namespace {

class ProbeContent : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return {240, 640};
  }

  void paint(PaintContext& ctx) const override {
    ctx.fillRect(Rect(20, 0, 180, 5), roo_display::color::White);
    ctx.addExclusion(Rect(20, 0, 180, 5));
#ifdef ROO_REDRAW_BASELINE
    ctx.clear();
#else
    ctx.clearDeferrableBackground();
#endif
  }
};

}  // namespace
}  // namespace roo_windows

void setup() {
  using namespace roo_windows;
  static std::array<roo::byte, 240 * 320 * 4> pixels{};
  static roo_display::OffscreenDevice<roo_display::Argb8888> device(
      240, 320, pixels.data(), roo_display::Argb8888());
  static roo_display::Display display(device);
  static roo_scheduler::SchedulingService scheduler;
  static Environment env(scheduler);
  static Application app(&env, display);
#if defined(ROO_REDRAW_OPTIONAL) && !defined(ROO_REDRAW_BASELINE)
  static AcceleratedScrollablePanel panel(
      app.context(), std::make_unique<ProbeContent>(app.context()));
#else
  static SimpleScrollablePanel panel(
      app.context(), std::make_unique<ProbeContent>(app.context()));
#endif
  app.add(panel, display.extents());
#ifndef ROO_REDRAW_BASELINE
  app.window().setAdvisoryPaintBudget(roo_time::Millis(16));
#endif
  app.refresh();
  panel.scrollBy(0, -8);
  app.refresh();
  std::printf(
      "Widget=%zu Container=%zu SimpleScrollablePanel=%zu Canvas=%zu "
      "PaintContext=%zu DisplayWindow=%zu Clipper=%zu Overlay=%zu\n",
      sizeof(Widget), sizeof(Container), sizeof(SimpleScrollablePanel),
      sizeof(Canvas), sizeof(PaintContext), sizeof(DisplayWindow),
      sizeof(Clipper), sizeof(roo_windows::internal::ClippedOverlay));
  std::exit(0);
}

void loop() {}
