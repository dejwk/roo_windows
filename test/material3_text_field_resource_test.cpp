#include <cstdlib>
#include <new>

#include "gtest/gtest.h"
#include "roo_windows/material3/text_field/secure_text_field.h"
#include "roo_windows_render_test_support.h"

namespace {
bool tracking = false;
size_t allocations = 0;
}  // namespace

void* operator new(size_t size) {
  if (tracking) ++allocations;
  if (void* p = std::malloc(size ? size : 1)) return p;
  throw std::bad_alloc();
}
void* operator new[](size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }
void operator delete[](void* p, size_t) noexcept { std::free(p); }

namespace roo_windows::material3 {
namespace {
class ResourceTest : public test_support::RooWindowsRenderTestSized<240, 100> {
};

// Verifies warmed visual-state and reveal updates do not grow editor storage;
// records the separate, deferred roo_display stream cost of painting them.
TEST_F(ResourceTest, WarmedEditorUpdatesDoNotAllocate) {
  SecureTextField field(context(), "Password", TextFieldVariant::kOutlined);
  field.setText("a sufficiently long password to avoid small string storage");
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(field.requestFocus());
  KeyEvent key;
  key.code = KeyCode::kEnter;
  key.phase = KeyPhase::kDown;
  ASSERT_TRUE(field.onKeyEvent(key));
  ASSERT_TRUE(refresh());
  for (int pass = 0; pass < 2; ++pass) {
    allocations = 0;
    tracking = pass == 1;
    for (int i = 0; i < 20; ++i) {
      field.setRevealed(i % 2);
      field.setHover(i % 3 == 0);
      task.textFieldEditor().setSelection(2, 7);
      context().animations().seek(field, 0, roo_time::Millis(i * 500));
    }
    tracking = false;
  }
  EXPECT_EQ(0u, allocations);

  ASSERT_TRUE(refresh());
  allocations = 0;
  for (int i = 0; i < 20; ++i) {
    field.setRevealed(i % 2);
    field.setHover(i % 3 == 0);
    tracking = true;
    app_.refresh();
    tracking = false;
  }
  // This is a characterization, not a zero-allocation paint guarantee. See
  // roo_display/docs/value_owned_raw_streams_design.md for the deferred fix.
  RecordProperty("warmed_paint_allocations_20_frames",
                 std::to_string(allocations));
  task.navigation().clear();
}
}  // namespace
}  // namespace roo_windows::material3
