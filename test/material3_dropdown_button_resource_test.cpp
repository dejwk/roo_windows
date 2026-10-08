#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/utilities/dropdown_button.h"

namespace {

struct alignas(std::max_align_t) AllocationHeader {
  size_t bytes;
  bool counted;
};

thread_local bool track_allocations = false;
thread_local size_t live_bytes = 0;
thread_local size_t peak_bytes = 0;

void* Allocate(size_t bytes, bool nothrow) {
  auto* header = static_cast<AllocationHeader*>(
      std::malloc(sizeof(AllocationHeader) + (bytes == 0 ? 1 : bytes)));
  if (header == nullptr) {
    if (nothrow) return nullptr;
    std::abort();
  }
  header->bytes = bytes;
  header->counted = track_allocations;
  if (header->counted) {
    live_bytes += bytes;
    peak_bytes = std::max(peak_bytes, live_bytes);
  }
  return header + 1;
}

void Deallocate(void* pointer) noexcept {
  if (pointer == nullptr) return;
  auto* header = static_cast<AllocationHeader*>(pointer) - 1;
  if (header->counted) live_bytes -= header->bytes;
  std::free(header);
}

}  // namespace

void* operator new(size_t bytes) { return Allocate(bytes, false); }
void* operator new[](size_t bytes) { return Allocate(bytes, false); }
void* operator new(size_t bytes, const std::nothrow_t&) noexcept {
  return Allocate(bytes, true);
}
void* operator new[](size_t bytes, const std::nothrow_t&) noexcept {
  return Allocate(bytes, true);
}
void operator delete(void* pointer) noexcept { Deallocate(pointer); }
void operator delete[](void* pointer) noexcept { Deallocate(pointer); }
void operator delete(void* pointer, size_t) noexcept { Deallocate(pointer); }
void operator delete[](void* pointer, size_t) noexcept { Deallocate(pointer); }
void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  Deallocate(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  Deallocate(pointer);
}

namespace roo_windows::material3 {
namespace {

class TestPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

static_assert(sizeof(DropdownButton) <=
                  sizeof(SurfaceWidget) + (sizeof(void*) == 4 ? 32 : 48),
              "DropdownButton must keep the closed object compact");

// Verifies closed selectors add no heap state and release every menu session.
TEST(DropdownButtonResource, ClosedCostAndRepeatedSessionRelease) {
  roo::byte raster[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      320, 240, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  TestPanel panel(app.context());
  app.addTaskFullScreen(panel);
  static constexpr const char* one[] = {"Off"};
  static constexpr const char* six[] = {"Off",         "Automatic", "Scheduled",
                                        "Maintenance", "Eco",       "Manual"};
  static constexpr const char* twenty[] = {
      "0",  "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19"};
  app.refresh();

  // Font metric caches may allocate once per glyph. Populate them before
  // measuring allocations owned by a closed selector instance.
  {
    DropdownButton warm_empty(app.context(), nullptr, 0);
    DropdownButton warm_single(app.context(), one);
    DropdownButton warm_medium(app.context(), six);
    DropdownButton warm_large(app.context(), twenty);
    for (int i = 0; i < 20; ++i) {
      warm_medium.getPreferredSize();
      warm_medium.measure(WidthSpec::Unspecified(0),
                          HeightSpec::Unspecified(0));
      warm_medium.setSelectedIndex(i % 6);
    }
  }

  track_allocations = true;
  size_t before_closed = live_bytes;
  {
    DropdownButton empty(app.context(), nullptr, 0);
    DropdownButton single(app.context(), one);
    DropdownButton medium(app.context(), six);
    DropdownButton large(app.context(), twenty);
    EXPECT_EQ(before_closed, live_bytes);
    for (int i = 0; i < 20; ++i) {
      medium.getPreferredSize();
      medium.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
      medium.setSelectedIndex(i % 6);
    }
    EXPECT_EQ(before_closed, live_bytes);
  }
  EXPECT_EQ(before_closed, live_bytes);
  track_allocations = false;

  DropdownButton button(app.context(), six);
  panel.add(WidgetRef(button), Rect(16, 16, 303, 63));
  app.refresh();

  track_allocations = true;
  ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
  size_t first_open_peak = peak_bytes;
  button.dismissMenu();
  app.refresh();
  size_t closed_baseline = live_bytes;
  for (int i = 0; i < 100; ++i) {
    ASSERT_EQ(MenuShowResult::kShown, button.showMenu());
    button.dismissMenu();
    app.refresh();
    EXPECT_EQ(closed_baseline, live_bytes);
  }
  track_allocations = false;
  std::cout << "dropdown closed_bytes=" << sizeof(DropdownButton)
            << " surface_bytes=" << sizeof(SurfaceWidget)
            << " first_open_peak_bytes=" << first_open_peak
            << " repeat_peak_bytes=" << peak_bytes
            << " retained_after_close=" << closed_baseline << '\n';
  panel.removeLast();
}

}  // namespace
}  // namespace roo_windows::material3
