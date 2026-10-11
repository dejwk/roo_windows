#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <new>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/toolbar/toolbar.h"

namespace {

struct alignas(std::max_align_t) AllocationHeader {
  size_t bytes;
  bool counted;
};

thread_local bool tracking = false;
thread_local size_t allocations = 0;
thread_local size_t live_bytes = 0;

void* Allocate(size_t bytes, bool nothrow) {
  auto* header = static_cast<AllocationHeader*>(
      std::malloc(sizeof(AllocationHeader) + (bytes == 0 ? 1 : bytes)));
  if (header == nullptr) {
    if (nothrow) return nullptr;
    std::abort();
  }
  header->bytes = bytes;
  header->counted = tracking;
  if (tracking) {
    ++allocations;
    live_bytes += bytes;
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

constexpr size_t AlignUp(size_t value, size_t alignment) {
  return (value + alignment - 1) / alignment * alignment;
}

static_assert(sizeof(DockedToolbar) <=
              AlignUp(sizeof(Material3Container) + sizeof(std::vector<void*>) +
                          alignof(DockedToolbar),
                      alignof(DockedToolbar)));
static_assert(sizeof(FloatingToolbar) <=
              AlignUp(sizeof(Material3Container) + sizeof(std::vector<void*>) +
                          alignof(FloatingToolbar),
                      alignof(FloatingToolbar)));

class Probe : public Widget {
 public:
  explicit Probe(ApplicationContext& context) : Widget(context) {}
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(40, 40);
  }
};

ApplicationContext MakeContext(Environment& env) {
  return ApplicationContext(env.scheduler(), env.theme(),
                            env.keyboardColorTheme());
}

// Verifies vector payload scales with supplied controls, capacity survives
// clear, and both toolbar instances stay within one vector plus alignment.
TEST(Material3ToolbarResource, TypeSizeAndRetainedCapacity) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  Probe children[8] = {Probe(context), Probe(context), Probe(context),
                       Probe(context), Probe(context), Probe(context),
                       Probe(context), Probe(context)};
  const size_t baseline = live_bytes;
  {
    DockedToolbar docked(context);
    tracking = true;
    for (int i = 0; i < 4; ++i) docked.add(children[i]);
    const size_t four_bytes = live_bytes - baseline;
    for (int i = 4; i < 8; ++i) docked.add(children[i]);
    const size_t eight_bytes = live_bytes - baseline;
    EXPECT_GE(eight_bytes, four_bytes);
    docked.clear();
    EXPECT_EQ(eight_bytes, live_bytes - baseline);
    tracking = false;
    std::cout << "docked_instance=" << sizeof(DockedToolbar)
              << " base=" << sizeof(Material3Container)
              << " four_payload=" << four_bytes
              << " eight_payload=" << eight_bytes << '\n';
  }
  EXPECT_EQ(baseline, live_bytes);
  {
    FloatingToolbar floating(context);
    tracking = true;
    for (Probe& child : children) floating.add(child);
    const size_t eight_bytes = live_bytes - baseline;
    floating.clear();
    EXPECT_EQ(eight_bytes, live_bytes - baseline);
    tracking = false;
    std::cout << "floating_instance=" << sizeof(FloatingToolbar)
              << " base=" << sizeof(Material3Container)
              << " eight_payload=" << eight_bytes << '\n';
  }
  EXPECT_EQ(baseline, live_bytes);
}

// Verifies repeated toolbar geometry and no-op setters allocate no scratch
// collections after child-vector growth has settled.
TEST(Material3ToolbarResource, SteadyGeometryHasNoToolbarAllocations) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar docked(context);
  FloatingToolbar floating(context);
  Probe docked_children[4] = {Probe(context), Probe(context), Probe(context),
                              Probe(context)};
  Probe floating_children[4] = {Probe(context), Probe(context), Probe(context),
                                Probe(context)};
  for (int i = 0; i < 4; ++i) {
    docked.add(docked_children[i], static_cast<ToolbarSlot>(i % 3));
    floating.add(floating_children[i]);
  }
  docked.setShowGroupDividers(true);
  for (int i = 0; i < 3; ++i) {
    docked.measure(WidthSpec::Exactly(320), HeightSpec::AtMost(128));
    docked.layout(Rect(0, 0, 319, 63));
    floating.measure(WidthSpec::AtMost(320), HeightSpec::AtMost(128));
    floating.layout(Rect(0, 0, 191, 63));
  }
  allocations = 0;
  tracking = true;
  for (int i = 0; i < 40; ++i) {
    docked.measure(WidthSpec::Exactly(320), HeightSpec::AtMost(128));
    docked.layout(Rect(0, 0, 319, 63));
    docked.setShowGroupDividers(true);
    docked.setArrangement(DockedToolbarArrangement::kEvenlySpaced);
    floating.measure(WidthSpec::AtMost(320), HeightSpec::AtMost(128));
    floating.layout(Rect(0, 0, 191, 63));
    floating.setOrientation(FloatingToolbarOrientation::kHorizontal);
    floating.setElevated(true);
  }
  tracking = false;
  EXPECT_EQ(0u, allocations);
  floating.clear();
  docked.clear();
}

// Verifies warmed divider painting allocates no scratch collection in the
// toolbar. Rounded renderer allocations are measured separately by host tests.
TEST(Material3ToolbarResource, DividerPaintingHasNoSteadyAllocations) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  DockedToolbar docked(context);
  Probe first(context);
  Probe second(context);
  docked.add(first, ToolbarSlot::kLeading);
  docked.add(second, ToolbarSlot::kTrailing);
  docked.setShowGroupDividers(true);
  docked.measure(WidthSpec::Exactly(320), HeightSpec::Exactly(64));
  docked.layout(Rect(0, 0, 319, 63));

  std::vector<roo::byte> raster(320 * 64 * 2);
  roo_display::OffscreenDevice<roo_display::Rgb565> device(
      320, 64, raster.data(), roo_display::Rgb565());
  roo_display::Display display(device);
  roo_display::Surface surface(
      display.output(), 0, 0, roo_display::Box(0, 0, 319, 63), false,
      roo_display::color::White, roo_display::FillMode::kVisible,
      roo_display::BlendingMode::kSourceOver);
  Canvas canvas(&surface);
  internal::ClipperState state;
  for (int i = 0; i < 3; ++i) {
    Clipper clipper(state, canvas.out());
    PaintContext ctx(canvas, clipper);
    docked.paint(ctx);
  }
  allocations = 0;
  tracking = true;
  for (int i = 0; i < 20; ++i) {
    Clipper clipper(state, canvas.out());
    PaintContext ctx(canvas, clipper);
    docked.paint(ctx);
  }
  tracking = false;
  EXPECT_EQ(0u, allocations);
  docked.clear();
}

// Verifies each zoom build scales toolbar chrome once around already measured
// child pixels, with the same floor in both floating orientations.
TEST(Material3ToolbarResource, ZoomScalesOnlyToolbarTokens) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context = MakeContext(env);
  FloatingToolbar floating(context);
  Dimensions empty =
      floating.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(64), empty.width());
  EXPECT_EQ(Scaled(64), empty.height());
  EXPECT_EQ(Scaled(8), floating.getPadding().left());
  Probe child(context);
  floating.add(child);
  Dimensions horizontal =
      floating.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(std::max(Scaled(64), 2 * Scaled(8) + 40), horizontal.width());
  EXPECT_EQ(std::max(Scaled(64), 2 * Scaled(8) + 40), horizontal.height());
  floating.setOrientation(FloatingToolbarOrientation::kVertical);
  Dimensions vertical =
      floating.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(horizontal.width(), vertical.width());
  EXPECT_EQ(horizontal.height(), vertical.height());
  floating.clear();
}

// Records complete rounded-frame allocations separately from the same frame
// with a real Material button. Neither count is a toolbar bookkeeping budget.
TEST(Material3ToolbarResource, RendererAndRealChildAllocationReport) {
  for (bool real_child : {false, true}) {
    constexpr int kWidth = 240;
    constexpr int kHeight = 128;
    std::vector<roo::byte> raster(kWidth * kHeight * 2);
    roo_display::OffscreenDevice<roo_display::Rgb565> device(
        kWidth, kHeight, raster.data(), roo_display::Rgb565());
    roo_display::Display display(device);
    roo_scheduler::SchedulingService scheduler;
    Environment env(scheduler);
    Application app(&env, display);
    auto toolbar = std::make_unique<FloatingToolbar>(app.context());
    FloatingToolbar* toolbar_ptr = toolbar.get();
    if (real_child) {
      toolbar->add(std::make_unique<Button>(app.context(), "Pen",
                                            ButtonVariant::kFilledTonal));
    } else {
      toolbar->add(std::make_unique<Probe>(app.context()));
    }
    app.add(std::move(toolbar), roo_display::Box(16, 16, 175, 79));
    app.refresh();
    allocations = 0;
    for (int i = 0; i < 4; ++i) {
      toolbar_ptr->invalidateDescending();
      toolbar_ptr->invalidateInterior();
      tracking = true;
      app.refresh();
      tracking = false;
    }
    std::cout << (real_child ? "rounded_with_button=" : "rounded_with_probe=")
              << allocations << " / 4 frames\n";
  }
}

}  // namespace
}  // namespace roo_windows::material3
