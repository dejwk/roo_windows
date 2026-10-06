// Host pthread high-water measurement; not an ESP32 stack-size guarantee.
#include <pthread.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gtest/gtest.h"
#include "roo_testing/system/timer.h"
#include "roo_windows.h"
#include "roo_windows/containers/accelerated_scrollable_panel.h"

namespace roo_windows {
namespace {

class StackContent : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return {240, 640};
  }

  void paint(PaintContext& ctx) const override {
    ctx.fillRect(Rect(20, 0, 180, 5), roo_display::color::White);
    ctx.addExclusion(Rect(20, 0, 180, 5));
    system_time_lag_ns(2000);
    ctx.clearDeferrableBackground();
  }
};

struct PaintSample {
  Application* app;
  uintptr_t origin = 0;
};

// Captures the stack origin outside the entire synchronous refresh call.
void* PaintOnStack(void* arg) {
  PaintSample& sample = *static_cast<PaintSample*>(arg);
  volatile char origin = 0;
  sample.origin = reinterpret_cast<uintptr_t>(&origin);
  if (sample.app != nullptr) sample.app->refresh();
  return nullptr;
}

// Measures deepest changed stack byte below the refresh caller's local origin.
size_t MeasurePaintStack(Application* app) {
  constexpr size_t kStackSize = 1024 * 1024;
  void* memory = nullptr;
  if (posix_memalign(&memory, 4096, kStackSize) != 0) std::abort();
  std::memset(memory, 0xA5, kStackSize);
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0 ||
      pthread_attr_setstack(&attr, memory, kStackSize) != 0) {
    std::abort();
  }
  PaintSample sample{app};
  pthread_t thread;
  if (pthread_create(&thread, &attr, PaintOnStack, &sample) != 0) std::abort();
  if (pthread_join(thread, nullptr) != 0) std::abort();
  pthread_attr_destroy(&attr);
  const auto* bytes = static_cast<const unsigned char*>(memory);
  size_t first = 0;
  while (first < kStackSize && bytes[first] == 0xA5) ++first;
  const uintptr_t lowest = reinterpret_cast<uintptr_t>(bytes + first);
  const size_t used = sample.origin > lowest ? sample.origin - lowest : 0;
  std::free(memory);
  return used;
}

// Compares warmed complete and accelerated refreshes on the same host driver,
// measuring the whole synchronous refresh rather than a helper's local frame.
TEST(AcceleratedRedrawStack, FullPaintHighWater) {
  size_t measured[2];
  for (int mode = 0; mode < 2; ++mode) {
    std::array<roo::byte, 240 * 320 * 4> pixels{};
    roo_display::OffscreenDevice<roo_display::Argb8888> device(
        240, 320, pixels.data(), roo_display::Argb8888());
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    auto content = std::make_unique<StackContent>(app.context());
    std::unique_ptr<SimpleScrollablePanel> panel;
    if (mode == 0) {
      panel = std::make_unique<SimpleScrollablePanel>(app.context(),
                                                      std::move(content));
    } else {
      panel = std::make_unique<AcceleratedScrollablePanel>(app.context(),
                                                           std::move(content));
    }
    SimpleScrollablePanel* ptr = panel.get();
    app.add(std::move(panel), display.extents());
    app.window().setAdvisoryPaintBudget(roo_time::Micros(1));
    app.refresh();
    for (int i = 0; i < 20; ++i) {
      ptr->scrollBy(0, -2);
      app.refresh();
    }
    ptr->scrollBy(0, -2);
    measured[mode] = MeasurePaintStack(&app);
    EXPECT_GT(measured[mode], 0u);
    std::printf("host_stack mode=%d bytes=%zu\n", mode, measured[mode]);
  }
  std::printf("host_stack_incremental_bytes=%lld\n",
              static_cast<long long>(measured[1]) -
                  static_cast<long long>(measured[0]));
}

}  // namespace
}  // namespace roo_windows
