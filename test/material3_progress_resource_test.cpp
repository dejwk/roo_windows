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

#include "roo_windows/core/panel.h"
#include "roo_windows/material3/progress_indicator/progress_indicator.h"

namespace roo_windows::material3 {
namespace {
class AllocationPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

// Verifies the complete warmed paint/registry path reuses retained storage at
// all waveform phases, including maximum disjoint segment/track shape count.
TEST(ProgressResourceTest, WarmedAnimationDoesNotAllocate) {
  roo::byte raster[260 * 100 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      260, 100, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  LinearProgressIndicator linear(app.context());
  CircularProgressIndicator circular(app.context());
  AllocationPanel panel(app.context());
  panel.add(linear, Rect(10, 10, 249, 13));
  panel.add(circular, Rect(100, 30, 147, 77));
  Task& task = app.addTaskFullScreen(panel);
  app.refresh();
  BeginAllocationTracking();
  linear.setIndeterminate();
  circular.setIndeterminate();
  app.refresh();
  for (int pass = 0; pass < 2; ++pass) {
    if (pass) BeginAllocationTracking();
    for (int phase = 0; phase < 5400; phase += 33) {
      app.context().animations().seek(linear, 0, roo_time::Millis(phase));
      app.context().animations().seek(circular, 0, roo_time::Millis(phase));
      app.refresh();
    }
    EndAllocationTracking();
    if (!pass) {
      std::cout << "Indeterminate admission/warmup allocations="
                << g_allocation_count << " peak=" << g_peak_bytes
                << " retained=" << g_active_bytes << std::endl;
    }
  }
  EXPECT_EQ(0u, g_allocation_count);
  std::cout << "Warmed progress allocations: " << g_allocation_count
            << std::endl;
  panel.removeAll();
  task.navigation().clear();
  app.refresh();
}
}  // namespace
}  // namespace roo_windows::material3
