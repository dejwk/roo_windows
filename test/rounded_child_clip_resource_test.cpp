#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <new>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/rounded_clip.h"
#include "roo_windows/material3/menu/menu_surface.h"

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

enum class GroupMode {
  kGuaranteedClipped,
  kGroupedAllClipped,
  kGroupedAllUnclipped,
  kGroupedMixed,
};

const char* GroupModeName(GroupMode mode) {
  switch (mode) {
    case GroupMode::kGuaranteedClipped:
      return "guaranteed_clipped";
    case GroupMode::kGroupedAllClipped:
      return "grouped_all_clipped";
    case GroupMode::kGroupedAllUnclipped:
      return "grouped_all_unclipped";
    case GroupMode::kGroupedMixed:
      return "grouped_mixed";
  }
  std::abort();
}

class ResourceRow : public SurfaceWidget {
 public:
  using SurfaceWidget::SurfaceWidget;

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(192, 4);
  }

  Color background() const override { return Color(0xFF3167B7); }

  void paint(PaintContext& ctx) const override {
    ++paint_calls;
    ctx.clear();
  }

  mutable size_t paint_calls = 0;
};

class ResourcePanel : public Panel {
 public:
  ResourcePanel(ApplicationContext& context, bool grouped)
      : Panel(context), grouped_(grouped) {}
  using Panel::add;

  bool clipsChildrenToRoundedBounds() const override { return true; }

  bool mayHaveUnclippedChildren() const override {
    ++capability_queries;
    return grouped_;
  }

  Color background() const override { return Color(0xFFF4EAD0); }

  BorderStyle getBorderStyle() const override { return BorderStyle(16, 0); }

  void resetCounters() const {
    capability_queries = 0;
    child_visits = 0;
  }

  mutable size_t capability_queries = 0;
  mutable size_t child_visits = 0;

 protected:
  void paintChildren(PaintContext& ctx) override {
    counting_paint_visits_ = true;
    Panel::paintChildren(ctx);
    counting_paint_visits_ = false;
  }

  const Widget& getChild(int idx) const override {
    if (counting_paint_visits_) ++child_visits;
    return Panel::getChild(idx);
  }

  Widget& getChild(int idx) override {
    if (counting_paint_visits_) ++child_visits;
    return Panel::getChild(idx);
  }

 private:
  bool grouped_;
  mutable bool counting_paint_visits_ = false;
};

struct ResourceMetrics {
  size_t allocations;
  size_t allocated_bytes;
  size_t capability_queries;
  size_t child_visits;
  size_t child_paints;
  double median_cpu_us;
};

ResourceMetrics MeasureScenario(int child_count, GroupMode mode) {
  constexpr int kBatchCount = 5;
  constexpr int kFramesPerBatch = 400;
  constexpr int kMeasuredFrames = kBatchCount * kFramesPerBatch;

  std::array<roo::byte, 240 * 160 * 4> pixels{};
  roo_display::OffscreenDevice<roo_display::Argb8888> device(
      240, 160, pixels.data(), roo_display::Argb8888());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  app.refresh();

  const bool grouped = mode != GroupMode::kGuaranteedClipped;
  auto panel = std::make_unique<ResourcePanel>(app.context(), grouped);
  ResourcePanel* owner = panel.get();
  std::array<ResourceRow*, 32> rows{};
  for (int i = 0; i < child_count; ++i) {
    auto row = std::make_unique<ResourceRow>(app.context());
    rows[i] = row.get();
    if (mode == GroupMode::kGroupedAllUnclipped ||
        (mode == GroupMode::kGroupedMixed && i % 2 != 0)) {
      row->setParentClipMode(ParentClipMode::kUnclipped);
    }
    panel->add(std::move(row), Rect(0, i * 4, 191, i * 4 + 3));
  }
  app.add(std::move(panel), roo_display::Box(24, 16, 215, 143));

  allocations = 0;
  allocated_bytes = 0;
  tracking = true;
  app.refresh();
  tracking = false;
  std::printf(
      "children=%d mode=%s first_refresh_new_calls=%zu "
      "requested_bytes=%zu\n",
      child_count, GroupModeName(mode), allocations, allocated_bytes);

  // Warm renderer vectors and deque capacities before the measured interval.
  for (int i = 0; i < 10; ++i) {
    owner->invalidateInterior();
    app.refresh();
  }
  owner->resetCounters();
  for (int i = 0; i < child_count; ++i) rows[i]->paint_calls = 0;

  allocations = 0;
  allocated_bytes = 0;
  std::array<double, kBatchCount> cpu_us{};
  tracking = true;
  for (int batch = 0; batch < kBatchCount; ++batch) {
    timespec cpu_start{};
    timespec cpu_end{};
    EXPECT_EQ(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_start), 0);
    for (int frame = 0; frame < kFramesPerBatch; ++frame) {
      owner->invalidateInterior();
      app.refresh();
    }
    EXPECT_EQ(clock_gettime(CLOCK_THREAD_CPUTIME_ID, &cpu_end), 0);
    cpu_us[batch] = ((cpu_end.tv_sec - cpu_start.tv_sec) * 1e6 +
                     (cpu_end.tv_nsec - cpu_start.tv_nsec) / 1e3) /
                    kFramesPerBatch;
  }
  tracking = false;

  size_t child_paints = 0;
  for (int i = 0; i < child_count; ++i) {
    child_paints += rows[i]->paint_calls;
  }
  std::sort(cpu_us.begin(), cpu_us.end());
  const size_t expected_visits =
      kMeasuredFrames * child_count * (grouped ? 2 : 1);
  EXPECT_EQ(owner->capability_queries, static_cast<size_t>(kMeasuredFrames));
  EXPECT_EQ(owner->child_visits, expected_visits);
  EXPECT_EQ(child_paints, static_cast<size_t>(kMeasuredFrames * child_count));

  ResourceMetrics metrics{
      allocations,         allocated_bytes, owner->capability_queries,
      owner->child_visits, child_paints,    cpu_us[kBatchCount / 2]};
  std::printf(
      "children=%d mode=%s median_cpu_us_per_frame=%.2f "
      "new_calls_per_frame=%.2f requested_bytes_per_frame=%.2f "
      "capability_queries=%zu child_visits=%zu child_paints=%zu\n",
      child_count, GroupModeName(mode), metrics.median_cpu_us,
      metrics.allocations / static_cast<double>(kMeasuredFrames),
      metrics.allocated_bytes / static_cast<double>(kMeasuredFrames),
      metrics.capability_queries, metrics.child_visits, metrics.child_paints);
  return metrics;
}

// Verifies grouped traversal stays within its query/visit/paint bounds and
// adds no allocations after warm-up for 0, 8, and 32 direct children.
// Timings exclude display bus transfer; use -c opt for useful comparisons.
TEST(RoundedClipResources, CharacterizeGroupedTraversal) {
  for (int child_count : {0, 8, 32}) {
    const ResourceMetrics baseline =
        MeasureScenario(child_count, GroupMode::kGuaranteedClipped);
    for (GroupMode mode :
         {GroupMode::kGroupedAllClipped, GroupMode::kGroupedAllUnclipped,
          GroupMode::kGroupedMixed}) {
      const ResourceMetrics grouped = MeasureScenario(child_count, mode);
      EXPECT_EQ(grouped.allocations, baseline.allocations);
      EXPECT_EQ(grouped.allocated_bytes, baseline.allocated_bytes);
    }
  }
  std::printf(
      "host_sizeof: clip=%zu output=%zu overlay=%zu decoration=%zu arena=%zu\n",
      sizeof(internal::RoundedClip), sizeof(internal::RoundedClipOutput),
      sizeof(internal::RoundedOverlay), sizeof(internal::RoundedDecoration),
      sizeof(internal::RoundedPaintState));
}

// Verifies the real fixed-child rounded component retains the single-scan
// capability declaration measured by the synthetic baseline.
TEST(RoundedClipResources, MenuPanelUsesGuaranteedClippedPath) {
  std::array<roo::byte, 4> pixels{};
  roo_display::OffscreenDevice<roo_display::Argb8888> device(
      1, 1, pixels.data(), roo_display::Argb8888());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  material3::internal::MenuPanel panel(app.context());
  EXPECT_FALSE(panel.mayHaveUnclippedChildren());
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
