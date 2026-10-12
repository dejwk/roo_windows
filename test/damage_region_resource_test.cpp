#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <initializer_list>
#include <memory>
#include <new>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/damage_region.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/presentation_pin.h"

namespace {
bool track_allocations = false;
size_t allocation_count = 0;

void* Allocate(size_t size) {
  if (track_allocations) ++allocation_count;
  return std::malloc(size == 0 ? 1 : size);
}

[[gnu::noinline]] void FreeAllocation(void* ptr) noexcept { std::free(ptr); }
}  // namespace

void* operator new(size_t size) {
  void* ptr = Allocate(size);
  if (ptr == nullptr) std::abort();
  return ptr;
}

void* operator new[](size_t size) { return ::operator new(size); }

void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
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
namespace {

using roo_display::Argb8888;
using roo_display::BlendingMode;
using roo_display::Color;

constexpr int kWidth = 320;
constexpr int kHeight = 240;
const Rect kLeft(10, 10, 19, 19);
const Rect kRight(300, 220, 309, 229);
const Rect kCenter(150, 100, 169, 119);

/// Counts physical writes after all clipping and exclusion filters.
class RecordingDevice : public roo_display::OffscreenDevice<Argb8888> {
 public:
  explicit RecordingDevice(roo::byte* pixels)
      : OffscreenDevice(kWidth, kHeight, pixels, Argb8888()),
        touched_(kWidth * kHeight, 0) {}

  void resetCounters() {
    std::fill(touched_.begin(), touched_.end(), 0);
    writes_ = 0;
    unique_ = 0;
    addresses_ = 0;
    copies_ = 0;
  }

  size_t writes() const { return writes_; }
  size_t unique() const { return unique_; }
  size_t addresses() const { return addresses_; }
  size_t copies() const { return copies_; }
  uint16_t touched(int x, int y) const { return touched_[y * kWidth + x]; }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    ++addresses_;
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    recordStream(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(Color color, uint32_t count) override {
    recordStream(count);
    OffscreenDevice::fill(color, count);
  }

  void writePixels(BlendingMode mode, Color* colors, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) record(x[i], y[i]);
    OffscreenDevice::writePixels(mode, colors, x, y, count);
  }

  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) record(x[i], y[i]);
    OffscreenDevice::fillPixels(mode, color, x, y, count);
  }

  void writeRects(BlendingMode mode, Color* colors, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      recordRect(x0[i], y0[i], x1[i], y1[i]);
    }
    OffscreenDevice::writeRects(mode, colors, x0, y0, x1, y1, count);
  }

  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      recordRect(x0[i], y0[i], x1[i], y1[i]);
    }
    OffscreenDevice::fillRects(mode, color, x0, y0, x1, y1, count);
  }

  void blitCopy(int16_t src_x0, int16_t src_y0, int16_t src_x1, int16_t src_y1,
                int16_t dst_x0, int16_t dst_y0) override {
    ++copies_;
    recordRect(dst_x0, dst_y0, dst_x0 + src_x1 - src_x0,
               dst_y0 + src_y1 - src_y0);
    OffscreenDevice::blitCopy(src_x0, src_y0, src_x1, src_y1, dst_x0, dst_y0);
  }

 private:
  void record(int x, int y) {
    if (x < 0 || x >= kWidth || y < 0 || y >= kHeight) return;
    ++writes_;
    uint16_t& count = touched_[y * kWidth + x];
    if (count++ == 0) ++unique_;
  }

  void recordRect(int x0, int y0, int x1, int y1) {
    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) record(x, y);
    }
  }

  void recordStream(uint32_t count) {
    while (count-- > 0) {
      record(x_, y_);
      if (++x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
  }

  std::vector<uint16_t> touched_;
  size_t writes_ = 0;
  size_t unique_ = 0;
  size_t addresses_ = 0;
  size_t copies_ = 0;
  int left_ = 0;
  int right_ = 0;
  int x_ = 0;
  int y_ = 0;
};

class CountedSurface : public SurfaceWidget {
 public:
  explicit CountedSurface(ApplicationContext& context, uint8_t radius = 0)
      : SurfaceWidget(context), radius_(radius) {}

  Color background() const override { return Color(0xFF4466AA); }

  BorderStyle getBorderStyle() const override {
    return BorderStyle(radius_, 0);
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return {kWidth, kHeight};
  }

  void paint(PaintContext& ctx) const override {
    ++paint_calls;
    ctx.clear();
  }

  mutable size_t paint_calls = 0;

 private:
  uint8_t radius_;
};

class CountedPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;

  Color background() const override { return Color(0xFF4466AA); }

  size_t children_paints = 0;

 protected:
  void paintChildren(PaintContext& ctx) override {
    ++children_paints;
    Panel::paintChildren(ctx);
  }
};

class MovingPin final : public PresentationPin {
 public:
  explicit MovingPin(Rect& bounds) : bounds_(bounds) {}

 protected:
  Rect boundsInWindow() const override { return bounds_; }

  void paint(PaintContext& ctx) const override {
    ctx.fillRect(bounds_, Color(0xFF22CC44));
    ctx.addExclusion(bounds_);
  }

 private:
  Rect& bounds_;
};

enum class Scene {
  kCorners,
  kCenter,
  kFull,
  kOpaqueChildren,
  kHiddenChild,
  kRounded,
  kSimpleScroll,
  kBlitScroll,
};

const char* SceneName(Scene scene) {
  switch (scene) {
    case Scene::kCorners:
      return "corners";
    case Scene::kCenter:
      return "center";
    case Scene::kFull:
      return "full";
    case Scene::kOpaqueChildren:
      return "children";
    case Scene::kHiddenChild:
      return "hidden";
    case Scene::kRounded:
      return "rounded";
    case Scene::kSimpleScroll:
      return "simple_scroll";
    case Scene::kBlitScroll:
      return "blit_scroll";
  }
  std::abort();
}

struct Metrics {
  size_t writes;
  size_t unique;
  size_t addresses;
  size_t copies;
  size_t hooks;
  size_t allocations;
  double cpu_us;
};

double CpuMicros(const timespec& start, const timespec& end) {
  return (end.tv_sec - start.tv_sec) * 1000000.0 +
         (end.tv_nsec - start.tv_nsec) / 1000.0;
}

Metrics Measure(Scene scene) {
  constexpr int kWarmFrames = 8;
  constexpr int kFrames = 100;
  std::vector<roo::byte> pixels(kWidth * kHeight * 4);
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  CountedSurface* leaf = nullptr;
  CountedSurface* left = nullptr;
  CountedSurface* right = nullptr;
  SimpleScrollablePanel* scroll = nullptr;
  if (scene == Scene::kSimpleScroll || scene == Scene::kBlitScroll) {
    auto surface = std::make_unique<CountedSurface>(app.context());
    leaf = surface.get();
    if (scene == Scene::kSimpleScroll) {
      auto viewport = std::make_unique<SimpleScrollablePanel>(
          app.context(), std::move(surface));
      scroll = viewport.get();
      app.add(std::move(viewport), roo_display::Box(0, 0, 319, 159));
    } else {
      auto viewport = std::make_unique<ScrollableBlitPanel>(app.context(),
                                                            std::move(surface));
      scroll = viewport.get();
      app.add(std::move(viewport), roo_display::Box(0, 0, 319, 159));
    }
  } else if (scene == Scene::kOpaqueChildren || scene == Scene::kHiddenChild) {
    auto panel = std::make_unique<CountedPanel>(app.context());
    auto left_child = std::make_unique<CountedSurface>(app.context());
    left = left_child.get();
    panel->add(std::move(left_child),
               scene == Scene::kHiddenChild ? kCenter : kLeft);
    if (scene == Scene::kHiddenChild) {
      left->setVisibility(Visibility::kInvisible);
    } else {
      auto right_child = std::make_unique<CountedSurface>(app.context());
      right = right_child.get();
      panel->add(std::move(right_child), kRight);
    }
    app.add(std::move(panel), roo_display::Box(0, 0, 319, 239));
  } else {
    auto surface = std::make_unique<CountedSurface>(
        app.context(), scene == Scene::kRounded ? 16 : 0);
    leaf = surface.get();
    app.add(std::move(surface), roo_display::Box(0, 0, 319, 239));
  }
  app.refresh();

  const auto submit = [&](int frame) {
    switch (scene) {
      case Scene::kCorners:
      case Scene::kRounded:
        app.root().invalidateDescending(kLeft);
        app.root().invalidateDescending(kRight);
        break;
      case Scene::kCenter:
      case Scene::kHiddenChild:
        app.root().invalidateDescending(kCenter);
        break;
      case Scene::kFull:
        app.root().invalidateDescending();
        break;
      case Scene::kOpaqueChildren:
        left->invalidateInterior();
        right->invalidateInterior();
        break;
      case Scene::kSimpleScroll:
      case Scene::kBlitScroll:
        scroll->scrollTo(0, frame % 2 == 0 ? -80 : 0);
        break;
    }
  };

  for (int i = 0; i < kWarmFrames; ++i) {
    submit(i);
    app.refresh();
  }
  if (leaf != nullptr) leaf->paint_calls = 0;
  if (left != nullptr) left->paint_calls = 0;
  if (right != nullptr) right->paint_calls = 0;
  allocation_count = 0;
  Metrics totals{};
  for (int i = 0; i < kFrames; ++i) {
    device.resetCounters();
    timespec start{};
    timespec end{};
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    track_allocations = true;
    submit(i);
    app.refresh();
    track_allocations = false;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
    totals.writes += device.writes();
    totals.unique += device.unique();
    totals.addresses += device.addresses();
    totals.copies += device.copies();
    totals.cpu_us += CpuMicros(start, end);
  }
  totals.hooks =
      leaf != nullptr
          ? leaf->paint_calls
          : left->paint_calls + (right != nullptr ? right->paint_calls : 0);
  totals.allocations = allocation_count;
  return totals;
}

// Sparse root damage leaves the broad envelope gap completely untouched, and
// a nested middle container is rejected before its descendants paint.
TEST(DamageRegionResourceTest, DistantDamageWritesOnlyTwoSmallBoxes) {
  std::vector<roo::byte> pixels(kWidth * kHeight * 4);
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  auto root_panel = std::make_unique<CountedPanel>(app.context());
  auto middle = std::make_unique<CountedPanel>(app.context());
  CountedPanel* middle_ptr = middle.get();
  auto middle_leaf = std::make_unique<CountedSurface>(app.context());
  CountedSurface* middle_leaf_ptr = middle_leaf.get();
  middle->add(std::move(middle_leaf), Rect(0, 0, 19, 19));
  root_panel->add(std::move(middle), kCenter);
  app.add(std::move(root_panel), roo_display::Box(0, 0, 319, 239));
  app.refresh();
  device.resetCounters();
  middle_ptr->children_paints = 0;
  middle_leaf_ptr->paint_calls = 0;
  app.root().invalidateDescending(kLeft);
  app.root().invalidateDescending(kRight);
  app.refresh();
  EXPECT_EQ(device.writes(), 200u);
  EXPECT_EQ(device.unique(), 200u);
  EXPECT_EQ(middle_ptr->children_paints, 0u);
  EXPECT_EQ(middle_leaf_ptr->paint_calls, 0u);
  EXPECT_FALSE(app.root().isDirty());
  device.resetCounters();
  app.refresh();
  EXPECT_EQ(device.writes(), 0u);
}

// Root-local, full, overflow and empty requests keep the pending region and
// widget dirty flags consistent after the initial display paint.
TEST(DamageRegionResourceTest, RootDamageCapacityAndLifecycle) {
  std::vector<roo::byte> pixels(kWidth * kHeight * 4);
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  auto surface = std::make_unique<CountedSurface>(app.context());
  CountedSurface* leaf = surface.get();
  app.add(std::move(surface), roo_display::Box(0, 0, 319, 239));
  app.refresh();

  const Rect requests[] = {Rect(10, 10, 11, 11), Rect(50, 30, 51, 31),
                           Rect(100, 70, 101, 71), Rect(150, 110, 151, 111),
                           Rect(200, 150, 201, 151)};
  internal::DamageRegion expected;
  device.resetCounters();
  for (const Rect& request : requests) {
    expected.add(request.asBox());
    app.root().invalidateDescending(request);
  }
  ASSERT_EQ(expected.size(), 4);
  app.refresh();
  size_t expected_writes = 0;
  for (int y = 0; y < kHeight; ++y) {
    for (int x = 0; x < kWidth; ++x) {
      const bool damaged = expected.covers(roo_display::Box(x, y, x, y));
      expected_writes += damaged;
      EXPECT_EQ(device.touched(x, y), damaged ? 1 : 0);
    }
  }
  EXPECT_EQ(device.writes(), expected_writes);
  EXPECT_EQ(device.unique(), expected_writes);
  EXPECT_FALSE(app.root().isDirty());

  device.resetCounters();
  app.root().invalidateInterior(kCenter);
  app.refresh();
  EXPECT_EQ(device.writes(), 400u);
  device.resetCounters();
  app.root().invalidateDescending();
  app.refresh();
  EXPECT_EQ(device.writes(), size_t(kWidth * kHeight));

  device.resetCounters();
  const size_t before = leaf->paint_calls;
  leaf->setDirty(Rect(0, 0, -1, -1));
  ASSERT_TRUE(app.root().isDirty());
  app.refresh();
  EXPECT_EQ(device.writes(), 0u);
  EXPECT_EQ(leaf->paint_calls, before);
  EXPECT_FALSE(app.root().isDirty());
}

// A pin jump reports two distant footprints; the old surface and new pin
// each write 100 pixels without touching the broad gap between them.
TEST(DamageRegionResourceTest, MovingPinWritesOnlyOldAndNewFootprints) {
  Rect pin_bounds = kLeft;
  std::vector<roo::byte> pixels(kWidth * kHeight * 4);
  RecordingDevice device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  auto panel = std::make_unique<CountedPanel>(app.context());
  auto anchor = std::make_unique<CountedSurface>(app.context());
  CountedSurface* anchor_ptr = anchor.get();
  panel->add(std::move(anchor), Rect(150, 100, 150, 100));
  app.add(std::move(panel), roo_display::Box(0, 0, 319, 239));
  app.refresh();
  ASSERT_EQ(
      anchor_ptr->showPresentationPin(std::make_unique<MovingPin>(pin_bounds)),
      PresentationPinShowResult::kShown);
  app.refresh();

  device.resetCounters();
  pin_bounds = kRight;
  app.refresh();
  EXPECT_EQ(device.writes(), 200u);
  EXPECT_EQ(device.unique(), 200u);
  EXPECT_EQ(device.touched(15, 15), 1u);
  EXPECT_EQ(device.touched(305, 225), 1u);
  EXPECT_EQ(device.touched(155, 115), 0u);
  EXPECT_FALSE(app.root().isDirty());
  anchor_ptr->hidePresentationPin();
}

// Verifies the baseline recorder sees area amplification and supplies
// repeatable control metrics without asserting that today's excess writes must
// remain.
TEST(DamageRegionResourceTest, CurrentRendererCharacterization) {
  for (Scene scene :
       {Scene::kCorners, Scene::kCenter, Scene::kFull, Scene::kOpaqueChildren,
        Scene::kHiddenChild, Scene::kRounded, Scene::kSimpleScroll,
        Scene::kBlitScroll}) {
    const Metrics metrics = Measure(scene);
    std::printf(
        "scene=%s frames=100 total_writes=%zu unique_pixels=%zu "
        "addresses=%zu copies=%zu hooks=%zu allocations=%zu cpu_us=%.1f\n",
        SceneName(scene), metrics.writes, metrics.unique, metrics.addresses,
        metrics.copies, metrics.hooks, metrics.allocations, metrics.cpu_us);
    EXPECT_GT(metrics.writes, 0u);
    if (scene == Scene::kHiddenChild) {
      EXPECT_EQ(metrics.hooks, 0u);
    } else {
      EXPECT_GT(metrics.hooks, 0u);
    }
    EXPECT_GE(metrics.writes, metrics.unique);
  }
}

}  // namespace
}  // namespace roo_windows
