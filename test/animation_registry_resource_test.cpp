#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/animation_registry.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace {

struct AllocationRecord {
  void* pointer = nullptr;
  size_t bytes = 0;
};

AllocationRecord g_allocation_records[2048];
bool g_track_allocations = false;
size_t g_allocation_count = 0;
size_t g_active_bytes = 0;
size_t g_peak_bytes = 0;

void RecordAllocation(void* pointer, size_t bytes) {
  if (!g_track_allocations) return;
  for (AllocationRecord& record : g_allocation_records) {
    if (record.pointer != nullptr) continue;
    record = AllocationRecord{pointer, bytes};
    ++g_allocation_count;
    g_active_bytes += bytes;
    if (g_active_bytes > g_peak_bytes) g_peak_bytes = g_active_bytes;
    return;
  }
  std::abort();
}

void RecordDeallocation(void* pointer) {
  if (!g_track_allocations || pointer == nullptr) return;
  for (AllocationRecord& record : g_allocation_records) {
    if (record.pointer != pointer) continue;
    g_active_bytes -= record.bytes;
    record = AllocationRecord();
    return;
  }
}

void BeginAllocationTracking() {
  for (AllocationRecord& record : g_allocation_records) {
    record = AllocationRecord();
  }
  g_allocation_count = 0;
  g_active_bytes = 0;
  g_peak_bytes = 0;
  g_track_allocations = true;
}

void EndAllocationTracking() { g_track_allocations = false; }

}  // namespace

void* operator new(size_t bytes) {
  void* pointer = std::malloc(bytes == 0 ? 1 : bytes);
  if (pointer == nullptr) std::abort();
  RecordAllocation(pointer, bytes);
  return pointer;
}

void* operator new[](size_t bytes) { return ::operator new(bytes); }

void* operator new(size_t bytes, const std::nothrow_t&) noexcept {
  void* pointer = std::malloc(bytes == 0 ? 1 : bytes);
  if (pointer != nullptr) RecordAllocation(pointer, bytes);
  return pointer;
}

void* operator new[](size_t bytes, const std::nothrow_t& tag) noexcept {
  return ::operator new(bytes, tag);
}

void operator delete(void* pointer) noexcept {
  RecordDeallocation(pointer);
  std::free(pointer);
}

void operator delete[](void* pointer) noexcept { ::operator delete(pointer); }
void operator delete(void* pointer, size_t) noexcept {
  ::operator delete(pointer);
}
void operator delete[](void* pointer, size_t) noexcept {
  ::operator delete(pointer);
}
void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  ::operator delete(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  ::operator delete(pointer);
}

void* operator new(size_t bytes, std::align_val_t alignment) {
  void* pointer = nullptr;
  if (posix_memalign(&pointer, static_cast<size_t>(alignment),
                     bytes == 0 ? 1 : bytes) != 0) {
    std::abort();
  }
  RecordAllocation(pointer, bytes);
  return pointer;
}

void* operator new[](size_t bytes, std::align_val_t alignment) {
  return ::operator new(bytes, alignment);
}

void* operator new(size_t bytes, std::align_val_t alignment,
                   const std::nothrow_t&) noexcept {
  void* pointer = nullptr;
  if (posix_memalign(&pointer, static_cast<size_t>(alignment),
                     bytes == 0 ? 1 : bytes) != 0) {
    return nullptr;
  }
  RecordAllocation(pointer, bytes);
  return pointer;
}

void* operator new[](size_t bytes, std::align_val_t alignment,
                     const std::nothrow_t& tag) noexcept {
  return ::operator new(bytes, alignment, tag);
}

void operator delete(void* pointer, std::align_val_t) noexcept {
  ::operator delete(pointer);
}

void operator delete[](void* pointer, std::align_val_t) noexcept {
  ::operator delete(pointer);
}

void operator delete(void* pointer, size_t, std::align_val_t) noexcept {
  ::operator delete(pointer);
}

void operator delete[](void* pointer, size_t, std::align_val_t) noexcept {
  ::operator delete(pointer);
}
void operator delete(void* pointer, std::align_val_t alignment,
                     const std::nothrow_t&) noexcept {
  ::operator delete(pointer, alignment);
}
void operator delete[](void* pointer, std::align_val_t alignment,
                       const std::nothrow_t&) noexcept {
  ::operator delete(pointer, alignment);
}

namespace roo_windows {
namespace test {

struct AnimationRegistryTestAccess {
  static void dispatch(AnimationRegistry& registry, roo_time::Uptime now) {
    registry.beginFrame(now);
    while (registry.dispatchNext()) {
    }
    registry.endFrame();
  }

  static size_t dispatchCapacity(const AnimationRegistry& registry) {
    return registry.dispatch_.capacity();
  }

  static roo_time::Uptime nextDeadline(const AnimationRegistry& registry) {
    return registry.nextFrameDeadline();
  }

  static uint16_t trackCapacity(const AnimationRegistry& registry) {
    return registry.tracks_.capacity();
  }

  static constexpr size_t channelKeySize() {
    return sizeof(AnimationRegistry::ChannelKey);
  }

  static constexpr size_t trackSize() {
    return sizeof(AnimationRegistry::Track);
  }

  static constexpr size_t dispatchItemSize() {
    return sizeof(AnimationRegistry::DispatchItem);
  }
};

}  // namespace test
namespace {

class TestApplication {
 public:
  TestApplication()
      : device_(16, 16, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_) {}

  ApplicationContext& context() { return app_.context(); }
  AnimationRegistry& registry() { return context().animations(); }

 private:
  roo::byte raster_[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
};

class ProbeWidget final : public BasicWidget {
 public:
  explicit ProbeWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  void pauseSelfOnFrame(bool enabled) { pause_self_ = enabled; }
  uint32_t frameCount() const { return frame_count_; }

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override {
    (void)sample;
    ++frame_count_;
    if (pause_self_) context().animations().pause(*this, tag);
  }

 private:
  uint32_t frame_count_ = 0;
  bool pause_self_ = false;
};

struct CapacitySample {
  size_t live;
  uint16_t buckets;
  size_t snapshot;
};

CapacitySample Populate(TestApplication& fixture, size_t count,
                        std::vector<std::unique_ptr<ProbeWidget>>& widgets,
                        const AnimationSpec& spec) {
  widgets.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    widgets.push_back(std::make_unique<ProbeWidget>(fixture.context()));
    EXPECT_EQ(AnimationStatus::kOk,
              fixture.registry().start(*widgets.back(), 0, spec));
  }
  test::AnimationRegistryTestAccess::dispatch(
      fixture.registry(), roo_time::Uptime::Start() + roo_time::Seconds(1));
  return CapacitySample{
      count,
      test::AnimationRegistryTestAccess::trackCapacity(fixture.registry()),
      test::AnimationRegistryTestAccess::dispatchCapacity(fixture.registry())};
}

TEST(AnimationRegistryResources, TargetSizedRecordsStayWithinCeilings) {
  constexpr size_t kPointerAdjustment =
      sizeof(void*) > 4 ? 5 * (sizeof(void*) - 4) : 0;
  EXPECT_LE(sizeof(AnimationRegistry), 96U + kPointerAdjustment);
  EXPECT_LE(test::AnimationRegistryTestAccess::channelKeySize(),
            8U + 2 * (sizeof(void*) - 4));
  EXPECT_LE(test::AnimationRegistryTestAccess::trackSize(), 128U);
  EXPECT_LE(test::AnimationRegistryTestAccess::dispatchItemSize(),
            12U + 3 * (sizeof(void*) - 4));
  EXPECT_EQ(sizeof(Widget), sizeof(BasicWidget));
}

TEST(AnimationRegistryResources, ReportsCapacityAndRetainsItAcrossChurn) {
  for (size_t count : {0U, 1U, 4U, 16U, 64U}) {
    TestApplication fixture;
    std::vector<std::unique_ptr<ProbeWidget>> widgets;
    CapacitySample sample =
        Populate(fixture, count, widgets, AnimationSpec::customTime());
    std::cout << "tracks=" << sample.live << " buckets=" << sample.buckets
              << " snapshot_capacity=" << sample.snapshot << '\n';
    for (auto& widget : widgets) {
      EXPECT_EQ(AnimationStatus::kOk, fixture.registry().cancel(*widget, 0));
    }
    EXPECT_EQ(sample.buckets, test::AnimationRegistryTestAccess::trackCapacity(
                                  fixture.registry()));
    EXPECT_EQ(sample.snapshot,
              test::AnimationRegistryTestAccess::dispatchCapacity(
                  fixture.registry()));
  }
}

TEST(AnimationRegistryResources, WarmedFrameAndControlsAllocateNothing) {
  TestApplication fixture;
  std::vector<std::unique_ptr<ProbeWidget>> widgets;
  CapacitySample sample =
      Populate(fixture, 16, widgets, AnimationSpec::customTime());
  BeginAllocationTracking();
  const roo_time::Uptime next =
      roo_time::Uptime::Start() + roo_time::Seconds(1) + roo_time::Millis(20);
  test::AnimationRegistryTestAccess::dispatch(fixture.registry(), next);
  for (auto& widget : widgets) {
    EXPECT_EQ(AnimationStatus::kOk, fixture.registry().pause(*widget, 0));
    EXPECT_EQ(AnimationStatus::kOk, fixture.registry().resume(*widget, 0));
  }
  for (auto& widget : widgets) {
    EXPECT_EQ(AnimationStatus::kOk, fixture.registry().cancel(*widget, 0));
  }
  EndAllocationTracking();
  std::cout << "warmed_frame_allocations=" << g_allocation_count << '\n';
  EXPECT_EQ(0U, g_allocation_count);
  EXPECT_EQ(sample.buckets, test::AnimationRegistryTestAccess::trackCapacity(
                                fixture.registry()));
  EXPECT_EQ(
      sample.snapshot,
      test::AnimationRegistryTestAccess::dispatchCapacity(fixture.registry()));
  for (const auto& widget : widgets) EXPECT_EQ(2U, widget->frameCount());
}

TEST(AnimationRegistryResources, ReportsPeakActiveHeapByWorkload) {
  for (size_t count : {1U, 4U, 16U, 64U}) {
    TestApplication fixture;
    std::vector<std::unique_ptr<ProbeWidget>> widgets;
    widgets.reserve(count);
    for (size_t i = 0; i < count; ++i) {
      widgets.push_back(std::make_unique<ProbeWidget>(fixture.context()));
    }
    BeginAllocationTracking();
    for (auto& widget : widgets) {
      ASSERT_EQ(
          AnimationStatus::kOk,
          fixture.registry().start(*widget, 0, AnimationSpec::customTime()));
    }
    test::AnimationRegistryTestAccess::dispatch(
        fixture.registry(), roo_time::Uptime::Start() + roo_time::Seconds(1));
    EndAllocationTracking();
    std::cout << "tracks=" << count
              << " allocation_calls=" << g_allocation_count
              << " peak_active_heap=" << g_peak_bytes
              << " retained_active_heap=" << g_active_bytes << '\n';
  }
}

int64_t MeasureDispatchMicros(EasingKind easing, bool mutation_heavy) {
  TestApplication fixture;
  std::vector<std::unique_ptr<ProbeWidget>> widgets;
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Seconds(10));
  spec.legs = 0;
  spec.minimum_interval = roo_time::Duration();
  spec.easing.kind = easing;
  if (easing == EasingKind::kCubicBezier) {
    spec.easing.x1 = 0.2f;
    spec.easing.y1 = 0.0f;
    spec.easing.x2 = 0.0f;
    spec.easing.y2 = 1.0f;
  }
  Populate(fixture, 16, widgets, spec);
  for (auto& widget : widgets) widget->pauseSelfOnFrame(mutation_heavy);

  int64_t worst = 0;
  roo_time::Uptime now = roo_time::Uptime::Start() + roo_time::Seconds(2);
  for (int round = 0; round < 200; ++round) {
    if (mutation_heavy) {
      for (auto& widget : widgets) {
        fixture.registry().resume(*widget, 0);
      }
    }
    now += roo_time::Millis(1);
    const auto begin = std::chrono::steady_clock::now();
    test::AnimationRegistryTestAccess::dispatch(fixture.registry(), now);
    const auto end = std::chrono::steady_clock::now();
    worst = std::max<int64_t>(
        worst,
        std::chrono::duration_cast<std::chrono::microseconds>(end - begin)
            .count());
  }
  return worst;
}

TEST(AnimationRegistryResources, SixteenTrackDispatchStaysBelowTwoMillis) {
  const int64_t linear_us = MeasureDispatchMicros(EasingKind::kLinear, false);
  const int64_t bezier_us =
      MeasureDispatchMicros(EasingKind::kCubicBezier, false);
  const int64_t mutation_us = MeasureDispatchMicros(EasingKind::kLinear, true);
  std::cout << "worst_linear_us=" << linear_us
            << " worst_bezier_us=" << bezier_us
            << " worst_mutation_us=" << mutation_us << '\n';
  EXPECT_LT(linear_us, 2000);
  EXPECT_LT(bezier_us, 2000);
  EXPECT_LT(mutation_us, 2000);
}

TEST(AnimationRegistryResources, SettledTracksPublishNoRecurringWake) {
  TestApplication fixture;
  std::vector<std::unique_ptr<ProbeWidget>> widgets;
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Millis(10));
  Populate(fixture, 16, widgets, spec);

  test::AnimationRegistryTestAccess::dispatch(
      fixture.registry(),
      roo_time::Uptime::Start() + roo_time::Seconds(1) + roo_time::Millis(10));

  for (const auto& widget : widgets) {
    EXPECT_FALSE(fixture.registry().contains(*widget, 0));
  }
  EXPECT_EQ(
      roo_time::Uptime::Max(),
      test::AnimationRegistryTestAccess::nextDeadline(fixture.registry()));
}

}  // namespace
}  // namespace roo_windows
