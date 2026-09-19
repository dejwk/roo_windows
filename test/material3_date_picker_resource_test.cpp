#include <cstdlib>
#include <new>

#include "gtest/gtest.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows_render_test_support.h"

namespace {
bool tracking = false;
size_t allocations = 0;
}  // namespace

void* operator new(size_t size) {
  if (tracking) ++allocations;
  if (void* p = std::malloc(size ? size : 1)) return p;
  std::abort();
}
void* operator new[](size_t size) { return ::operator new(size); }
// Keep nothrow allocations on the same malloc/free path under ASan.
void* operator new(size_t size, const std::nothrow_t&) noexcept {
  if (tracking) ++allocations;
  return std::malloc(size ? size : 1);
}
void* operator new[](size_t size, const std::nothrow_t& tag) noexcept {
  return ::operator new(size, tag);
}
void operator delete(void* p, const std::nothrow_t&) noexcept { std::free(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept {
  std::free(p);
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }
void operator delete[](void* p, size_t) noexcept { std::free(p); }

namespace roo_windows::material3 {
namespace {
class DatePickerResource
    : public test_support::RooWindowsRenderTestSized<400, 576> {};

// Verifies calendar navigation and draft changes have no warmed allocation
// cost; records the upstream glyph-stream allocation cost separately from
// picker state.
TEST_F(DatePickerResource, NavigationDoesNotAllocate) {
  Panel content(context());
  Task& task = app_.addTaskFullScreen(content);
  ModalDatePicker picker(context());
  picker.setValue(roo_time::CivilDay::FromYmd(2024, 2, 29));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(PresentationStartResult::kStarted, picker.open(task));
  auto& panel =
      *static_cast<internal::DatePickerPanel*>(task.focus().scopeRoot());
  ASSERT_TRUE(refresh());
  for (int pass = 0; pass < 2; ++pass) {
    allocations = 0;
    tracking = pass == 1;
    for (int i = 0; i < 24; ++i) {
      panel.activateHeader(i % 2 ? 0 : 3);
      panel.selectDate(roo_time::CivilDay::FromYmd(2024, 2, i % 28 + 1));
    }
    tracking = false;
  }
  EXPECT_EQ(0u, allocations);
  allocations = 0;
  for (int i = 0; i < 10; ++i) {
    panel.selectDate(roo_time::CivilDay::FromYmd(2024, 2, i + 1));
    tracking = true;
    refresh();
    tracking = false;
  }
  RecordProperty("paint_allocations_10_frames", std::to_string(allocations));
  picker.dismiss();
  task.navigation().clear();
}
}  // namespace
}  // namespace roo_windows::material3
