#include <array>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <new>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/rounded_clip.h"

namespace {
bool tracking = false;
size_t allocations = 0;
size_t allocated_bytes = 0;
void* Allocate(size_t size) {
  if (tracking) {
    ++allocations;
    allocated_bytes += size;
  }
  return std::malloc(size == 0 ? 1 : size);
}
}  // namespace

void* operator new(size_t size) {
  void* p = Allocate(size);
  if (p == nullptr) std::abort();
  return p;
}
void* operator new[](size_t size) { return ::operator new(size); }
void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}
void* operator new[](size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }
void operator delete[](void* p, size_t) noexcept { std::free(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { std::free(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept {
  std::free(p);
}

namespace roo_windows {
namespace {

// Measures identical scene construction and invalidation with clipping off/on.
// Timings exclude display bus transfer; use -c opt for useful host comparisons.
TEST(RoundedClipResources, CharacterizeRefresh) {
  class PanelProbe : public Panel {
   public:
    PanelProbe(ApplicationContext& context, bool rounded)
        : Panel(context), rounded_(rounded) {}
    using Panel::add;
    bool clipsChildrenToRoundedBounds() const override { return rounded_; }
    Color background() const override { return Color(0xFFF4EAD0); }
    BorderStyle getBorderStyle() const override { return BorderStyle(16, 0); }

   private:
    bool rounded_;
  };
  class Row : public SurfaceWidget {
   public:
    using SurfaceWidget::SurfaceWidget;
    Dimensions getSuggestedMinimumDimensions() const override {
      return Dimensions(192, 26);
    }
    Color background() const override { return Color(0xFF3167B7); }
    BorderStyle getBorderStyle() const override { return BorderStyle(8, 0); }
    void paint(PaintContext& ctx) const override { ctx.clear(); }
  };
  for (bool rounded : {false, true}) {
    std::array<roo::byte, 240 * 160 * 4> pixels{};
    roo_display::OffscreenDevice<roo_display::Argb8888> device(
        240, 160, pixels.data(), roo_display::Argb8888());
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    ASSERT_TRUE(app.refresh());
    auto panel = std::make_unique<PanelProbe>(app.context(), rounded);
    PanelProbe* owner = panel.get();
    for (int i = 0; i < 5; ++i) {
      panel->add(std::make_unique<Row>(app.context()),
                 Rect(0, i * 30 - 9, 191, i * 30 + 16));
    }
    app.add(std::move(panel), roo_display::Box(24, 16, 215, 143));
    allocations = allocated_bytes = 0;
    tracking = true;
    const bool first = app.refresh();
    tracking = false;
    ASSERT_TRUE(first);
    std::printf("rounded=%d first_refresh_new_calls=%zu requested_bytes=%zu\n",
                rounded, allocations, allocated_bytes);
    // Warm renderer vectors and deque capacities before the measured interval.
    for (int i = 0; i < 10; ++i) {
      owner->invalidateInterior();
      ASSERT_TRUE(app.refresh());
    }
    allocations = allocated_bytes = 0;
    timespec cpu_start{};
    timespec cpu_end{};
    ASSERT_EQ(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_start), 0);
    const auto start = std::chrono::steady_clock::now();
    tracking = true;
    bool complete = true;
    for (int i = 0; i < 200; ++i) {
      owner->invalidateInterior();
      complete = app.refresh() && complete;
    }
    tracking = false;
    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                             std::chrono::steady_clock::now() - start)
                             .count();
    ASSERT_EQ(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_end), 0);
    const double cpu_us = (cpu_end.tv_sec - cpu_start.tv_sec) * 1e6 +
                          (cpu_end.tv_nsec - cpu_start.tv_nsec) / 1e3;
    ASSERT_TRUE(complete);
    std::printf(
        "rounded=%d wall_us_per_frame=%.2f cpu_us_per_frame=%.2f "
        "new_calls_per_frame=%.2f "
        "requested_bytes_per_frame=%.2f\n",
        rounded, elapsed / 200.0, cpu_us / 200.0, allocations / 200.0,
        allocated_bytes / 200.0);
  }
  std::printf(
      "host_sizeof: clip=%zu output=%zu overlay=%zu decoration=%zu arena=%zu\n",
      sizeof(internal::RoundedClip), sizeof(internal::RoundedClipOutput),
      sizeof(internal::RoundedOverlay), sizeof(internal::RoundedDecoration),
      sizeof(internal::RoundedPaintState));
}

// Verifies the sparse boundary storage allocates nothing when reused for the
// same geometry. Complete-scene allocation costs are measured separately above.
TEST(RoundedClipResources, WarmedBoundaryDoesNotAllocate) {
  internal::RoundedClip clip;
  const roo_display::Box bounds(0, 0, 191, 127);
  clip.reset(&clip, bounds, BorderStyle(16, 0));
  allocations = 0;
  tracking = true;
  for (int i = 0; i < 100; ++i) {
    clip.reset(&clip, bounds, BorderStyle(16, 0));
    clip.accumulate(11, 0, Color(0x804080C0));
  }
  tracking = false;
  EXPECT_EQ(allocations, 0u);
}

}  // namespace
}  // namespace roo_windows
