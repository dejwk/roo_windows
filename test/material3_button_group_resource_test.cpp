#include <cstdlib>
#include <iostream>
#include <new>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/cached_measure.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/button/toggle_icon_button.h"

namespace {

struct AllocationRecord {
  void* pointer = nullptr;
  size_t bytes = 0;
};

AllocationRecord records[4096];
bool counting = false;
size_t allocations = 0;
size_t live_bytes = 0;
size_t peak_bytes = 0;
// The ledger itself has fixed static storage and cannot affect measurements.
void* Allocate(size_t bytes) {
  void* pointer = std::malloc(bytes == 0 ? 1 : bytes);
  if (pointer == nullptr) std::abort();
  if (counting) {
    ++allocations;
    bool recorded = false;
    for (AllocationRecord& record : records) {
      if (record.pointer == nullptr) {
        record = {pointer, bytes};
        recorded = true;
        break;
      }
    }
    if (!recorded) std::abort();
    live_bytes += bytes;
    peak_bytes = std::max(peak_bytes, live_bytes);
  }
  return pointer;
}

void Free(void* pointer) {
  if (pointer == nullptr) return;
  for (AllocationRecord& record : records) {
    if (record.pointer == pointer) {
      live_bytes -= record.bytes;
      record = {};
      break;
    }
  }
  std::free(pointer);
}

void Begin() {
  for (AllocationRecord& record : records) record = {};
  allocations = live_bytes = peak_bytes = 0;
  counting = true;
}

void End() { counting = false; }

void Report(const char* stage) {
  std::cout << stage << " allocations=" << allocations
            << " retained=" << live_bytes << " peak=" << peak_bytes << '\n';
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

void operator delete(void* pointer) noexcept { Free(pointer); }

void operator delete[](void* pointer) noexcept { Free(pointer); }

void operator delete(void* pointer, size_t) noexcept { Free(pointer); }

void operator delete[](void* pointer, size_t) noexcept { Free(pointer); }

void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  Free(pointer);
}

void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  Free(pointer);
}

namespace roo_windows::test {

struct AnimationRegistryTestAccess {
  static size_t Count(const AnimationRegistry& registry) {
    return registry.tracks_.size();
  }
};
}  // namespace roo_windows::test

namespace roo_windows::material3 {
namespace {

class ResourcePanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

template <class Group>
class InspectableGroup : public Group {
 public:
  using Group::Group;
  size_t capacity() const { return this->entries_.capacity(); }
};

// Measures ownership growth separately from font/renderer first-use work.
template <class Group>
void CheckResources(int count, bool icons) {
  std::vector<roo::byte> raster(320 * 240 * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(320, 240,
                                                             raster.data(), {});
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  ResourcePanel panel(app.context());
  MonoIcon icon = ic_outlined_24_action_home();
  std::vector<std::unique_ptr<ButtonGroupButton>> labels;
  std::vector<std::unique_ptr<ButtonGroupIconButton>> artwork;
  for (int i = 0; i < count; ++i) {
    if (icons)
      artwork.push_back(
          std::make_unique<ButtonGroupIconButton>(app.context(), icon));
    else
      labels.push_back(
          std::make_unique<ButtonGroupButton>(app.context(), "Capture"));
  }
  InspectableGroup<Group> group(app.context());
  EXPECT_EQ(0u, group.capacity());
  Begin();
  for (int i = 0; i < count; ++i) {
    if (icons)
      group.add(*artwork[i]);
    else
      group.add(*labels[i]);
  }
  End();
  std::cout << "group=" << sizeof(Group) << " count=" << count
            << " icons=" << icons << " capacity=" << group.capacity()
            << " vector_bytes=" << group.capacity() * sizeof(void*) << '\n';
  Report("add");
  EXPECT_EQ(group.capacity() * sizeof(void*), live_bytes);
  EXPECT_EQ(
      0u, test::AnimationRegistryTestAccess::Count(app.context().animations()));
  app.addTaskFullScreen(panel);
  panel.add(WidgetRef(group), Rect(12, 24, 307, 79));
  Begin();
  Dimensions natural =
      group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  group.layout(Rect(0, 0, natural.width() - 1, natural.height() - 1));
  End();
  Report("first_layout_font_baseline");
  if (icons) {
    EXPECT_EQ(0u, allocations);
  }
  Begin();
  app.refresh();
  End();
  Report("first_paint_glyph_baseline");
  Begin();
  for (int i = 0; i < 20; ++i) {
    group.measure(WidthSpec::Exactly(296), HeightSpec::Exactly(56));
    group.layout(Rect(12, 24, 307, 79));
  }
  End();
  Report("warmed_layout");
  EXPECT_EQ(0u, allocations);
  if (count > 0) {
    internal::ButtonGroupEntry& target =
        icons ? static_cast<internal::ButtonGroupEntry&>(*artwork.back())
              : static_cast<internal::ButtonGroupEntry&>(*labels.back());
    // The last entry may be clipped at 32 members, so use the first for a
    // presented first-use click; projection costs are measured independently.
    Widget& first = icons ? static_cast<Widget&>(*artwork.front())
                          : static_cast<Widget&>(*labels.front());
    Begin();
    first.onSingleTapUp(10, 10);
    End();
    Report("first_click_service_baseline");
    EXPECT_EQ(0u, allocations);
    Begin();
    for (int i = 0; i < 20; ++i) {
      target.onClickAnimationChanged(i / 20.0f);
      target.setPressed(i % 2 == 0);
      group.measure(WidthSpec::Exactly(296), HeightSpec::Exactly(56));
      group.layout(Rect(12, 24, 307, 79));
    }
    End();
    Report("warmed_samples_and_layout");
    EXPECT_EQ(0u, allocations);
    first.onCancel();
    target.onCancel();
    Begin();
    for (int i = 0; i < 5; ++i) {
      first.onSingleTapUp(10, 10);
      first.onCancel();
    }
    End();
    Report("warmed_click_service");
    EXPECT_EQ(0u, allocations);
    Begin();
    app.root().invalidateDescending();
    app.refresh();
    End();
    Report("warmed_paint_framework_baseline");
  }
  size_t capacity = group.capacity();
  Begin();
  for (int cycle = 0; cycle < 5; ++cycle) {
    group.clear();
    for (int i = 0; i < count; ++i) {
      if (icons)
        group.add(*artwork[i]);
      else
        group.add(*labels[i]);
    }
  }
  End();
  Report("repeated_add_clear");
  EXPECT_EQ(0u, allocations);
  EXPECT_EQ(capacity, group.capacity());
  EXPECT_EQ(
      0u, test::AnimationRegistryTestAccess::Count(app.context().animations()));
  panel.removeAll();
}

// Records cold font and ordinary surface-pipeline work separately from group
// vector storage. This executes before grouped scenes consume shared caches.
TEST(ButtonGroupResource, StandaloneFontAndRendererBaseline) {
  std::vector<roo::byte> raster(320 * 240 * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(320, 240,
                                                             raster.data(), {});
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  ResourcePanel panel(app.context());
  Button button(app.context(), "Capture");
  app.addTaskFullScreen(panel);
  panel.add(WidgetRef(button), Rect(12, 24, 127, 63));
  Begin();
  button.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  End();
  Report("standalone_first_font_metrics");
  Begin();
  app.refresh();
  End();
  Report("standalone_first_paint");
  Begin();
  app.root().invalidateDescending();
  app.refresh();
  End();
  Report("standalone_warmed_paint");
  Begin();
  button.onSingleTapUp(10, 10);
  End();
  Report("standalone_first_click_service");
  button.onCancel();
  panel.removeAll();
}

// Verifies incremental budgets, unchanged standalone sizes and zero added
// allocations on first icon layout and warmed grouped layout/click sample
// paths.
TEST(ButtonGroupResource, BoundedStorageAndHotPaths) {
  std::cout << "Widget=" << sizeof(Widget)
            << " SurfaceWidget=" << sizeof(SurfaceWidget)
            << " Container=" << sizeof(Container)
            << " Button=" << sizeof(Button)
            << " IconButton=" << sizeof(IconButton)
            << " ToggleIconButton=" << sizeof(ToggleIconButton)
            << " label=" << sizeof(ButtonGroupButton)
            << " icon=" << sizeof(ButtonGroupIconButton)
            << " standard=" << sizeof(ButtonGroup)
            << " connected=" << sizeof(ConnectedButtonGroup)
            << " ComponentTheme=" << sizeof(ComponentTheme)
            << " Material3Theme=" << sizeof(Material3Theme)
            << " Theme=" << sizeof(Theme)
            << " CachedMeasure=" << sizeof(CachedMeasure) << '\n';
  for (int count : {0, 3, 5, 32}) {
    for (bool icons : {false, true}) {
      CheckResources<ButtonGroup>(count, icons);
      CheckResources<ConnectedButtonGroup>(count, icons);
    }
  }
}

// Verifies first and repeated mask normalization/projection do not allocate.
TEST(ButtonGroupResource, SelectionHasNoAllocation) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(scheduler, env.theme(), env.keyboardColorTheme());
  ConnectedButtonGroup group(context);
  for (int i = 0; i < 32; ++i)
    group.add(std::make_unique<ButtonGroupButton>(context, "View"));
  group.setSelectionMode(ConnectedButtonGroupSelectionMode::kMultiple);
  Begin();
  for (int i = 0; i < 100; ++i)
    group.setSelectionMask(i % 2 == 0 ? UINT32_MAX : 1u << 31);
  End();
  Report("mask_projection");
  EXPECT_EQ(0u, allocations);
}

static_assert(sizeof(ButtonGroupButton) - sizeof(SurfaceWidget) <=
              (sizeof(void*) == 4 ? 20 : 40));
static_assert(sizeof(ButtonGroupIconButton) - sizeof(SurfaceWidget) <=
              (sizeof(void*) == 4 ? 12 : 24));
static_assert(sizeof(ButtonGroup) - sizeof(Container) <=
              (sizeof(void*) == 4 ? 16 : 32));
static_assert(sizeof(ConnectedButtonGroup) - sizeof(Container) <=
              (sizeof(void*) == 4 ? 24 : 40));
static_assert(sizeof(Button) == (sizeof(void*) == 4 ? 40 : 72));
static_assert(sizeof(IconButton) == (sizeof(void*) == 4 ? 32 : 56));
static_assert(sizeof(ToggleIconButton) == (sizeof(void*) == 4 ? 40 : 72));
static_assert(sizeof(ComponentTheme) == 29);
static_assert(sizeof(CachedMeasure) == 16);
}  // namespace
}  // namespace roo_windows::material3
