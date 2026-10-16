#include <algorithm>
#include <array>
#include <cstdio>
#include <ctime>
#include <memory>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {
namespace {

constexpr int kSide = 200;
constexpr int kWidgetCount = 100;
constexpr int kBatches = 5;
constexpr int kFramesPerBatch = 100;

class PaintLeaf : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(5, 5);
  }

  Color background() const override { return defaultColor(); }

  void paint(PaintContext& ctx) const override {
    ++paint_count;
    ctx.clear();
  }

  mutable int paint_count = 0;
};

class PaintPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;

  Color background() const override { return Color(0xFFF0F0F0); }
};

/// Converts two thread-CPU timestamps to elapsed microseconds.
double CpuMicroseconds(const timespec& start, const timespec& end) {
  return (end.tv_sec - start.tv_sec) * 1e6 +
         (end.tv_nsec - start.tv_nsec) / 1e3;
}

/// Measures full refresh with a fixed 100-leaf paint workload under @p depth
/// generic layout ancestors. Timings are host CPU, excluding display transfer.
double MeasureDepth(int depth) {
  std::array<roo::byte, kSide * kSide * 4> pixels{};
  roo_display::OffscreenDevice<roo_display::Argb8888> device(
      kSide, kSide, pixels.data(), roo_display::Argb8888());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application application(&environment, display);
  application.refresh();

  auto deepest = std::make_unique<PaintPanel>(application.context());
  std::array<PaintLeaf*, kWidgetCount> leaves{};
  for (int i = 0; i < kWidgetCount; ++i) {
    auto leaf = std::make_unique<PaintLeaf>(application.context());
    leaves[i] = leaf.get();
    const int x = (i % 10) * 10;
    const int y = (i / 10) * 10;
    deepest->add(std::move(leaf), Rect(x, y, x + 4, y + 4));
  }

  std::unique_ptr<PaintPanel> root = std::move(deepest);
  for (int level = 1; level < depth; ++level) {
    auto parent = std::make_unique<PaintPanel>(application.context());
    parent->add(std::move(root), Rect(0, 0, kSide - 1, kSide - 1));
    root = std::move(parent);
  }
  PaintPanel* root_widget = root.get();
  application.add(std::move(root),
                  roo_display::Box(0, 0, kSide - 1, kSide - 1));
  application.refresh();

  for (int i = 0; i < 8; ++i) {
    root_widget->invalidateInterior();
    application.refresh();
  }

  std::array<double, kBatches> times{};
  for (int batch = 0; batch < kBatches; ++batch) {
    timespec start{};
    timespec end{};
    EXPECT_EQ(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start), 0);
    for (int frame = 0; frame < kFramesPerBatch; ++frame) {
      root_widget->invalidateInterior();
      application.refresh();
    }
    EXPECT_EQ(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end), 0);
    times[batch] = CpuMicroseconds(start, end) / kFramesPerBatch;
  }

  for (const PaintLeaf* leaf : leaves) {
    EXPECT_EQ(leaf->paint_count, 1 + 8 + kBatches * kFramesPerBatch);
  }
  std::sort(times.begin(), times.end());
  return times[kBatches / 2];
}

// Verifies the baseline fixture paints all 100 leaves at each depth and
// reports comparable host medians for later no-regression checks.
TEST(OptionalDesignSystemsResources, CharacterizeDepth) {
  for (int depth : {1, 8, 32}) {
    const double median = MeasureDepth(depth);
    EXPECT_GT(median, 0.0);
    std::printf("depth=%d leaves=100 median_host_cpu_us=%.2f\n", depth, median);
  }
  std::printf(
      "host_sizes widget=%zu surface=%zu container=%zu theme=%zu "
      "click=%zu overlay=%zu\n",
      sizeof(Widget), sizeof(SurfaceWidget), sizeof(Container), sizeof(Theme),
      sizeof(ClickAnimation), sizeof(OverlaySpec));
}

}  // namespace
}  // namespace roo_windows
