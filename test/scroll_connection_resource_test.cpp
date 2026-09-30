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
#include "roo_windows/core/environment.h"
#include "roo_windows/core/widget.h"

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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using namespace material3;
using namespace test_support;
using ScrollConnectionResourceTest = RooWindowsRenderTestSized<320, 240>;
class ResourcePanel : public SimpleScrollablePanel {
 public:
  using SimpleScrollablePanel::onDrag;
  using SimpleScrollablePanel::onDragStart;
  using SimpleScrollablePanel::SimpleScrollablePanel;
};

// Verifies optional registration is the only allocation in connection dispatch.
TEST_F(ScrollConnectionResourceTest, RegistrationAndDispatchCosts) {
  AppBar bar(context(), AppBarVariant::kMediumFlexible);
  SimpleScrollablePanel panel(context());
  EXPECT_EQ(nullptr, context().scrollConnectionsIfPresent());
  BeginAllocationTracking();
  auto status =
      bar.setScrollBehavior(panel, AppBarScrollBehavior::kEnterAlways);
  size_t count = g_allocation_count;
  size_t bytes = g_active_bytes;
  EndAllocationTracking();
  ASSERT_EQ(ScrollConnectionStatus::kSuccess, status);
  auto connection = material3::internal::FindAppBarConnection(bar);
  BeginAllocationTracking();
  for (int i = 0; i < 1000; ++i) {
    roo_windows::internal::ScrollConnection::Dispatch dispatch(connection);
    connection->onPreScroll(-1);
    connection->onPreScroll(1);
    connection->onPositionChanged({0, -1}, {0, 0}, ScrollSource::kGeometry);
  }
  size_t dispatch_allocations = g_allocation_count;
  EndAllocationTracking();
  EXPECT_EQ(0u, dispatch_allocations);
  std::cout << "scroll registration: " << count << " allocations, " << bytes
            << " retained host bytes; connection object " << sizeof(*connection)
            << " bytes\n";
}

// Verifies a real scaffold can relayout a collapsing bar without allocating
// in the connected drag/layout path once initialized. Painting is excluded:
// the existing font renderer allocates a stream for each glyph.
TEST_F(ScrollConnectionResourceTest, ScaffoldDragDoesNotAllocate) {
  auto scaffold = std::make_unique<LayoutScaffold>(context());
  auto bar =
      std::make_unique<AppBar>(context(), AppBarVariant::kMediumFlexible);
  bar->setTitle("Equipment");
  auto panel = std::make_unique<ResourcePanel>(context());
  ResourcePanel* target = panel.get();
  panel->setContents(std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::White, Dimensions(320, 600)));
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar->setScrollBehavior(*panel, AppBarScrollBehavior::kEnterAlways));
  scaffold->setTopBar(std::move(bar));
  scaffold->setBody(std::move(panel));
  app_.add(std::move(scaffold), roo_display::Box(0, 0, 319, 239));
  ASSERT_TRUE(refresh());
  target->onDragStart(0, 0);
  BeginAllocationTracking();
  for (int i = 0; i < 40; ++i) {
    target->onDrag(0, 0, 0, -1);
  }
  for (int i = 0; i < 40; ++i) {
    target->onDrag(0, 0, 0, 1);
  }
  size_t count = g_allocation_count;
  EndAllocationTracking();
  EXPECT_EQ(0u, count);
}
// Verifies a flex column can relayout a collapsing bar without allocating
// in the connected drag/layout path once initialized. Painting is excluded:
// the existing font renderer allocates a stream for each glyph.
TEST_F(ScrollConnectionResourceTest, FlexDragDoesNotAllocate) {
  auto scaffold =
      std::make_unique<FlexLayout>(context(), FlexDirection::kColumn);
  auto bar =
      std::make_unique<AppBar>(context(), AppBarVariant::kMediumFlexible);
  bar->setTitle("Equipment");
  auto panel = std::make_unique<ResourcePanel>(context());
  ResourcePanel* target = panel.get();
  panel->setContents(std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::White, Dimensions(320, 600)));
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar->setScrollBehavior(*panel, AppBarScrollBehavior::kEnterAlways));
  scaffold->add(std::move(bar));
  scaffold->add(std::move(panel),
                {.flex_grow = 1, .flex_basis = FlexBasis::kZero});
  app_.add(std::move(scaffold), roo_display::Box(0, 0, 319, 239));
  ASSERT_TRUE(refresh());
  target->onDragStart(0, 0);
  BeginAllocationTracking();
  for (int i = 0; i < 40; ++i) {
    target->onDrag(0, 0, 0, -1);
  }
  for (int i = 0; i < 40; ++i) {
    target->onDrag(0, 0, 0, 1);
  }
  size_t count = g_allocation_count;
  EndAllocationTracking();
  EXPECT_EQ(0u, count);
}
}  // namespace
}  // namespace roo_windows
