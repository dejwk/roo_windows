#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/split_button.h"

namespace {

thread_local bool counting = false;
thread_local size_t allocation_count = 0;

void* Allocate(size_t bytes) {
  if (counting) ++allocation_count;
  void* result = std::malloc(bytes == 0 ? 1 : bytes);
  if (result == nullptr) std::abort();
  return result;
}

}  // namespace

void* operator new(size_t bytes) { return Allocate(bytes); }
void* operator new[](size_t bytes) { return Allocate(bytes); }
void* operator new(size_t bytes, const std::nothrow_t&) noexcept {
  return Allocate(bytes);
}
void* operator new[](size_t bytes, const std::nothrow_t&) noexcept {
  return Allocate(bytes);
}
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, size_t) noexcept { std::free(pointer); }
void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  std::free(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  std::free(pointer);
}

namespace roo_windows::material3 {
namespace {

class TestPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

static_assert(sizeof(SplitButton) >= sizeof(Container) + 2 * sizeof(Button),
              "inline children must account for their complete Button storage");

// Verifies repeated layout measurement adds no heap state after font warmup;
// reports renderer and menu allocations separately for acceptance accounting.
TEST(SplitButtonResource, RetainedSizeAndRepeatedWork) {
  roo::byte raster[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      320, 240, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  TestPanel panel(app.context());
  app.addTaskFullScreen(panel);
  StandardMenuItem item(StandardMenuItemInit{"Run once"});
  MenuEntry row(app.context());
  row.setMenuItem(item);
  MenuGroup group(app.context());
  group.add(row);
  Menu menu(app.context());
  menu.addGroup(group);
  SplitButton button(app.context(), menu, "Run");
  button.setOnInteractiveChange([] {});
  panel.add(WidgetRef(button), Rect(24, 24, 183, 63));
  app.refresh();

  for (int i = 0; i < 8; ++i) {
    button.measure(WidthSpec::Exactly(160), HeightSpec::Exactly(40));
    app.refresh();
  }

  allocation_count = 0;
  counting = true;
  for (int i = 0; i < 100; ++i) {
    button.measure(WidthSpec::Exactly(160), HeightSpec::Exactly(40));
  }
  counting = false;
  EXPECT_EQ(0u, allocation_count);

  allocation_count = 0;
  counting = true;
  for (int i = 0; i < 4; ++i) {
    button.invalidateInterior();
    app.refresh();
  }
  counting = false;
  size_t renderer_allocations = allocation_count;

  allocation_count = 0;
  counting = true;
  EXPECT_EQ(MenuShowResult::kShown, button.showMenu());
  button.dismissMenu();
  counting = false;
  size_t menu_open_close_allocations = allocation_count;
  std::cout << "split_size=" << sizeof(SplitButton)
            << " container_size=" << sizeof(Container)
            << " button_size=" << sizeof(Button)
            << " menu_size=" << sizeof(Menu)
            << " renderer_allocations=" << renderer_allocations
            << " menu_open_close_allocations=" << menu_open_close_allocations
            << '\n';

  panel.removeLast();
  menu.clearGroups();
  group.clear();
}

}  // namespace
}  // namespace roo_windows::material3
