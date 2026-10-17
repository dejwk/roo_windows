#include <cstdlib>
#include <new>

#include "gtest/gtest.h"
#include "roo_windows/core/editor_destination.h"
#include "roo_windows_render_test_support.h"

namespace {
bool tracking = false;
size_t allocations = 0;
size_t allocated_bytes = 0;
[[gnu::noinline]] void FreeAllocation(void* ptr) noexcept { std::free(ptr); }
}  // namespace

void* operator new(size_t size) {
  if (tracking) {
    ++allocations;
    allocated_bytes += size;
  }
  if (void* ptr = std::malloc(size == 0 ? 1 : size)) return ptr;
  throw std::bad_alloc();
}
void* operator new[](size_t size) { return ::operator new(size); }
void* operator new(size_t size, const std::nothrow_t&) noexcept {
  if (tracking) {
    ++allocations;
    allocated_bytes += size;
  }
  return std::malloc(size == 0 ? 1 : size);
}
void* operator new[](size_t size, const std::nothrow_t& tag) noexcept {
  return ::operator new(size, tag);
}
void operator delete(void* ptr) noexcept { FreeAllocation(ptr); }
void operator delete[](void* ptr) noexcept { FreeAllocation(ptr); }
void operator delete(void* ptr, size_t) noexcept { FreeAllocation(ptr); }
void operator delete[](void* ptr, size_t) noexcept { FreeAllocation(ptr); }
void operator delete(void* ptr, const std::nothrow_t&) noexcept {
  FreeAllocation(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
  FreeAllocation(ptr);
}

namespace roo_windows {
// Verifies editor controls allocate only on first use, reports that allocation
// separately, and confirms repeated access retains one object without growth.
TEST(EditorDestinationResources, LazyAllocationAndReuse) {
  for (DesignSystem style :
       {DesignSystem::kMaterial2, DesignSystem::kMaterial3}) {
    roo::byte pixels[32 * 32 * 2] = {};
    roo_display::OffscreenDevice<roo_display::Argb4444> device(32, 32, pixels,
                                                               {});
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler, DefaultTheme(), DefaultKeyboardColorTheme(),
                    style);
    Application app(&env, display);
    EXPECT_EQ(nullptr, app.context().editorDestinationIfPresent());
    allocations = 0;
    allocated_bytes = 0;
    tracking = true;
    EditorDestination& destination = app.context().editorDestination();
    tracking = false;
    EXPECT_GT(allocations, 0u);
    const std::string prefix =
        style == DesignSystem::kMaterial2 ? "m2_" : "m3_";
    RecordProperty(prefix + "lazy_allocations", std::to_string(allocations));
    RecordProperty(prefix + "lazy_requested_bytes",
                   std::to_string(allocated_bytes));
    allocations = 0;
    tracking = true;
    EditorDestination* repeated = &app.context().editorDestination();
    tracking = false;
    EXPECT_EQ(&destination, repeated);
    EXPECT_EQ(0u, allocations);
  }
}
}  // namespace roo_windows
