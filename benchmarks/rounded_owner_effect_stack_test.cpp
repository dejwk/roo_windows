// Host high-water probe for complete synchronous paints, including traversal,
// effect snapshots, composition, exclusion filters, and the offscreen driver.
// A caller-owned pthread stack is painted before thread creation and inspected
// only after join. This is a measured workload, not an embedded stack bound.
#include <pthread.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {
namespace {

class StackPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  bool isClickable() const override { return true; }
  bool clipsChildrenToRoundedBounds() const override { return true; }
  Color background() const override { return Color(0xFFE8DED0); }
  BorderStyle getBorderStyle() const override { return BorderStyle(16, 1); }
};

struct PaintSample {
  Application* app;
  uintptr_t origin = 0;
};

// Captures the stack origin outside the entire synchronous refresh call.
void* PaintOnStack(void* arg) {
  PaintSample& sample = *static_cast<PaintSample*>(arg);
  volatile char origin = 0;
  sample.origin = reinterpret_cast<uintptr_t>(&origin);
  if (sample.app != nullptr) sample.app->refresh();
  return nullptr;
}

// Measures deepest changed stack byte below the refresh caller's local origin.
size_t MeasurePaintStack(Application* app) {
  constexpr size_t kStackSize = 1024 * 1024;
  void* memory = nullptr;
  if (posix_memalign(&memory, 4096, kStackSize) != 0) std::abort();
  std::memset(memory, 0xA5, kStackSize);
  pthread_attr_t attr;
  if (pthread_attr_init(&attr) != 0 ||
      pthread_attr_setstack(&attr, memory, kStackSize) != 0) {
    std::abort();
  }
  PaintSample sample{app};
  pthread_t thread;
  if (pthread_create(&thread, &attr, PaintOnStack, &sample) != 0) std::abort();
  if (pthread_join(thread, nullptr) != 0) std::abort();
  pthread_attr_destroy(&attr);
  const auto* bytes = static_cast<const unsigned char*>(memory);
  size_t first = 0;
  while (first < kStackSize && bytes[first] == 0xA5) ++first;
  const uintptr_t lowest = reinterpret_cast<uintptr_t>(bytes + first);
  const size_t used = sample.origin > lowest ? sample.origin - lowest : 0;
  std::free(memory);
  return used;
}

// Verifies stack measurement covers actual full paints at increasing widget
// depth; prints the empty-thread floor separately from measured refresh costs.
TEST(RoundedOwnerEffectsStack, FullPaintHighWater) {
  std::printf("host_empty_thread_stack_bytes=%zu\n",
              MeasurePaintStack(nullptr));
  for (int depth : {1, 2, 4}) {
    std::array<roo::byte, 240 * 160 * 4> pixels{};
    roo_display::OffscreenDevice<roo_display::Argb8888> device(
        240, 160, pixels.data(), roo_display::Argb8888());
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    app.refresh();
    auto outer = std::make_unique<StackPanel>(app.context());
    StackPanel* owner = outer.get();
    StackPanel* parent = owner;
    for (int i = 1; i < depth; ++i) {
      auto child = std::make_unique<StackPanel>(app.context());
      StackPanel* next = child.get();
      next->setPressed(true);
      parent->add(std::move(child), Rect(2, 2, 170 - 8 * i, 106 - 8 * i));
      parent = next;
    }
    app.add(std::move(outer), roo_display::Box(24, 16, 215, 143));
    for (int style = 0; style < 4; ++style) {
      owner->setEnabled(true);
      owner->setPressed(style == 1);
      if (style == 2) owner->onShowPress(8, 8);
      if (style == 3) owner->setEnabled(false);
      for (int i = 0; i < 10; ++i) {
        owner->invalidateInterior();
        app.refresh();
      }
      owner->invalidateInterior();
      const size_t stack = MeasurePaintStack(&app);
      std::printf("host_paint_stack depth=%d owner_style=%d bytes=%zu\n", depth,
                  style, stack);
      EXPECT_GT(stack, 0u);
      EXPECT_LT(stack, 1024u * 1024u);
    }
  }
}

}  // namespace
}  // namespace roo_windows
