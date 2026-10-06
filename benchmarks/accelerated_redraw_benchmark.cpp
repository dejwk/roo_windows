// Host/model probe. Wire time is an explicitly synthetic RGB565 20 Mbit/s
// transfer cost; CPU time is host steady-clock time. Neither is hardware data.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <vector>

#include "gtest/gtest.h"
#include "roo_testing/system/timer.h"
#include "roo_windows.h"
#ifndef ROO_REDRAW_BASELINE
#include "roo_windows/containers/accelerated_scrollable_panel.h"
#endif
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#ifndef ROO_REDRAW_BASELINE
#include "roo_windows/core/background_deferral.h"
#endif
#include "roo_windows/core/paint_context.h"

namespace {
bool tracking = false;
size_t allocations = 0;
size_t allocated_bytes = 0;
void* Allocate(size_t size) {
  if (tracking) {
    ++allocations;
    allocated_bytes += size;
  }
  return std::malloc(size == 0 ? 1 : size);
}
}  // namespace

void* operator new(size_t size) {
  void* p = Allocate(size);
  if (p == nullptr) std::abort();
  return p;
}
void* operator new[](size_t size) { return ::operator new(size); }
void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}
void* operator new[](size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p, size_t) noexcept { std::free(p); }
void operator delete[](void* p, size_t) noexcept { std::free(p); }
void operator delete(void* p, const std::nothrow_t&) noexcept { std::free(p); }
void operator delete[](void* p, const std::nothrow_t&) noexcept {
  std::free(p);
}

namespace roo_windows {
namespace {
using namespace roo_display;
using Clock = std::chrono::steady_clock;

#ifdef ROO_REDRAW_BASELINE
using OptionalProbePanel = SimpleScrollablePanel;
#else
using OptionalProbePanel = AcceleratedScrollablePanel;
#endif

// Lets the same workload measure an archived complete-paint baseline.
void ClearOfferedBackground(PaintContext& ctx) {
#ifdef ROO_REDRAW_BASELINE
  ctx.clear();
#else
  ctx.clearDeferrableBackground();
#endif
}

class ProbeKeys : public KeySource {
 public:
  int drain(KeyEvent* events, int capacity) override {
    if (!pending_ || capacity == 0) return 0;
    events[0] = {KeyPhase::kDown, KeyCode::kUnknown, 0, 0};
    pending_ = false;
    latency = (roo_time::Uptime::Now() - ready_).inMicros();
    ++deliveries;
    return 1;
  }

  void post() {
    ready_ = roo_time::Uptime::Now();
    pending_ = true;
    notifyReady();
  }

  int64_t latency = 0;
  int deliveries = 0;

 private:
  bool hasPendingEvents() const override { return pending_; }
  bool pending_ = false;
  roo_time::Uptime ready_;
};

// Counts every physical submission and models only pixel wire time. Simulated
// hardware copies have zero transfer time and must be reported separately.
class ProbeDevice : public OffscreenDevice<Argb8888> {
 public:
  ProbeDevice(int width, int height, roo::byte* pixels, ProbeKeys& keys)
      : OffscreenDevice(width, height, pixels, Argb8888()),
        width_(width),
        stamps_(width * height, 0),
        keys_(keys) {}

  void beginFrame() {
    ++frame;
    pixels = 0;
    copied_pixels = 0;
    duplicates = 0;
    first_white = roo_time::Uptime::Max();
    input_posted_ = false;
  }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    left_ = x_ = x0;
    right_ = x1;
    y_ = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* colors, uint32_t count) override {
    if (first_white == roo_time::Uptime::Max() &&
        std::find(colors, colors + count, color::White) != colors + count) {
      first_white = roo_time::Uptime::Now();
    }
    record(count);
    OffscreenDevice::write(colors, count);
  }

  void fill(Color value, uint32_t count) override {
    if (value == color::White && first_white == roo_time::Uptime::Max()) {
      first_white = roo_time::Uptime::Now();
    }
    record(count);
    OffscreenDevice::fill(value, count);
  }

  void writePixels(BlendingMode mode, Color* values, int16_t* x, int16_t* y,
                   uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(values + i, 1);
    }
  }

  void fillPixels(BlendingMode mode, Color value, int16_t* x, int16_t* y,
                  uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(value, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* values, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      fillRects(mode, values[i], x0 + i, y0 + i, x1 + i, y1 + i, 1);
    }
  }

  void fillRects(BlendingMode mode, Color value, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(value, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }

  void blitCopy(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t dx,
                int16_t dy) override {
    OffscreenDevice::blitCopy(x0, y0, x1, y1, dx, dy);
    for (int y = dy; y <= dy + y1 - y0; ++y) {
      for (int x = dx; x <= dx + x1 - x0; ++x) {
        uint32_t& stamp = stamps_[y * width_ + x];
        if (stamp == frame) ++duplicates;
        stamp = frame;
        ++copied_pixels;
      }
    }
  }

  int maxWriteAge() const {
    return frame - *std::min_element(stamps_.begin(), stamps_.end());
  }

  bool model_wire = false;
  bool post_input = false;
  uint32_t frame = 0;
  uint64_t pixels = 0;
  uint64_t copied_pixels = 0;
  uint64_t duplicates = 0;
  roo_time::Uptime first_white = roo_time::Uptime::Max();

 private:
  void record(uint32_t count) {
    if (post_input && !input_posted_) {
      input_posted_ = true;
      keys_.post();
    }
    pixels += count;
    uint32_t remaining = count;
    while (remaining != 0) {
      uint32_t span = std::min<uint32_t>(remaining, right_ - x_ + 1);
      uint32_t* start = stamps_.data() + y_ * width_ + x_;
      duplicates += std::count(start, start + span, frame);
      std::fill(start, start + span, frame);
      remaining -= span;
      x_ += span;
      if (x_ > right_) {
        x_ = left_;
        ++y_;
      }
    }
    if (model_wire) system_time_lag_ns(static_cast<uint64_t>(count) * 800);
  }

  int width_;
  std::vector<uint32_t> stamps_;
  ProbeKeys& keys_;
  int left_ = 0;
  int right_ = 0;
  int x_ = 0;
  int y_ = 0;
  bool input_posted_ = false;
};

class ProbeScene : public SurfaceWidget {
 public:
  ProbeScene(ApplicationContext& ctx, int width, int height, int clears,
             int overlays, bool opaque, bool nested)
      : SurfaceWidget(ctx),
        width_(width),
        height_(height),
        clears_(clears),
        overlays_(overlays),
        opaque_(opaque),
        nested_(nested) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return {static_cast<XDim>(width_), static_cast<YDim>(height_ + 512)};
  }

  void paint(PaintContext& ctx) const override {
    Rect clip = ctx.localClip();
    for (int i = 0; i < overlays_; ++i) {
      ctx.addOverlayShape(
          SmoothFilledCircle(FpPoint{float(20 + i * 7 % (width_ - 40)),
                                     float(20 + i * 17 % height_)},
                             10, Color(0x504060A0)));
    }
    // All white foreground marks are mandatory and supply a measurable physical
    // output event. Opaque row tiles intentionally leave no optional gaps.
    int period = 32;
    for (int y = clip.yMin() / period * period; y <= clip.yMax(); y += period) {
      Rect stripe(opaque_ ? 0 : 20, y, opaque_ ? width_ - 1 : width_ / 3,
                  y + (opaque_ ? period - 1 : 7));
      ctx.fillRect(stripe, color::White);
      ctx.addExclusion(stripe);
    }
    if (nested_) paintNestedRow(ctx);
    for (int i = 0; i < 2; ++i) {
      PaintDecoration decoration;
      decoration.bounds = Rect(10, 40 + i * 90, width_ - 11, 66 + i * 90);
      decoration.background = i == 0 ? color::Green : color::Red;
      decoration.corner_radii = {8, 8, 8, 8};
      decoration.outline_width = 1;
      decoration.outline_color = color::White;
      decoration.elevation = 2;
      ctx.addDecoration(decoration);
    }
    // Vertical slices keep the palette tied to content X coordinates so cached
    // vertical translations have the same semantics as complete repainting.
    for (int i = 0; i < clears_; ++i) {
      Rect region(width_ * i / clears_, clip.yMin(),
                  width_ * (i + 1) / clears_ - 1, clip.yMax());
      PaintContext fill = ctx.clipped(region);
      fill.setBgcolor(i % 2 == 0 ? Color(0xFF23344A) : Color(0xFF384A5B));
      ClearOfferedBackground(fill);
      ctx.addExclusion(region);
    }
  }

 private:
  void paintNestedRow(PaintContext& ctx) const {
    Clipper& clipper = ctx.clipperForFramework();
    Rect bounds(width_ / 3, height_ / 2, width_ * 2 / 3, height_ / 2 + 40);
    Box device_bounds =
        bounds.translate(ctx.canvas().dx(), ctx.canvas().dy()).asBox();
    internal::RoundedClip& mask =
        clipper.prepareRoundedClip(this, device_bounds, BorderStyle(12, 1));
    {
      PaintContext row = ctx.clipped(bounds);
      row.setBgcolor(color::Red);
      internal::RoundedClipScope rounded(row, mask);
      Rect stripe(bounds.xMin(), bounds.yMin(), bounds.xMax(),
                  bounds.yMin() + 7);
      row.fillRect(stripe, color::White);
      row.addExclusion(stripe);
      ClearOfferedBackground(row);
      row.addExclusion(bounds);
    }
    clipper.addRoundedDecoration(this, ctx.canvas().clip_box(), device_bounds,
                                 2, color::Red, BorderStyle(12, 1),
                                 color::White);
  }

  int width_;
  int height_;
  int clears_;
  int overlays_;
  bool opaque_;
  bool nested_;
};

struct FrameStats {
  int paints = 0;
  size_t exclusion_capacity = 0;
};

template <typename Base>
class ProbePanel : public Base {
 public:
  ProbePanel(ApplicationContext& ctx, WidgetRef content, ProbeDevice& device,
             FrameStats& stats)
      : Base(ctx, std::move(content)), device_(device), stats_(stats) {}

 protected:
  void paintWidgetContents(PaintContext& ctx) override {
    device_.beginFrame();
    ++stats_.paints;
    Base::paintWidgetContents(ctx);
    stats_.exclusion_capacity =
        std::max(stats_.exclusion_capacity,
                 ctx.clipperForFramework().exclusions().capacity());
  }

 private:
  ProbeDevice& device_;
  FrameStats& stats_;
};

class ProbeFrame : public AlignedLayout {
 public:
  ProbeFrame(ApplicationContext& ctx, WidgetRef child) : AlignedLayout(ctx) {
    add(std::move(child));
  }

  bool clipsChildrenToRoundedBounds() const override { return true; }

  BorderStyle getBorderStyle() const override { return BorderStyle(16, 1); }
};

int64_t Percentile(std::vector<int64_t> values, int percentage) {
  if (values.empty()) return -1;
  std::sort(values.begin(), values.end());
  return values[(values.size() - 1) * percentage / 100];
}

// Advances only scheduler-owned work until one actual refresh has completed.
void PaintNext(roo_scheduler::SchedulingService& scheduler,
               const FrameStats& stats) {
  int previous = stats.paints;
  for (int i = 0; i < 4 && stats.paints == previous; ++i) {
    roo_time::Uptime next = scheduler.getNearestExecutionTime();
    ASSERT_NE(next, roo_time::Uptime::Max());
    if (next > roo_time::Uptime::Now()) {
      system_time_lag_ns((next - roo_time::Uptime::Now()).inMicros() * 1000);
    }
    scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  }
  ASSERT_EQ(previous + 1, stats.paints);
}

void RunCase(int width, int height, int clears, int overlays, int depth,
             bool opaque, int delta, int mode) {
  std::vector<roo::byte> framebuffer(width * height * 4, roo::byte{0});
  ProbeKeys keys;
  ProbeDevice device(width, height, framebuffer.data(), keys);
  Display display(device);
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  Application app(&env, display, keys, false);
  FrameStats stats;
  WidgetRef scene(std::make_unique<ProbeScene>(
      app.context(), width, height, clears, overlays, opaque, depth != 0));
  SimpleScrollablePanel* panel;
  std::unique_ptr<Widget> view;
  if (mode == 1) {
    auto widget = std::make_unique<ProbePanel<OptionalProbePanel>>(
        app.context(), std::move(scene), device, stats);
    panel = widget.get();
    view = std::move(widget);
  } else if (mode == 2) {
    auto widget = std::make_unique<ProbePanel<ScrollableBlitPanel>>(
        app.context(), std::move(scene), device, stats);
    panel = widget.get();
    view = std::move(widget);
  } else {
    auto widget = std::make_unique<ProbePanel<SimpleScrollablePanel>>(
        app.context(), std::move(scene), device, stats);
    panel = widget.get();
    view = std::move(widget);
  }
  for (int i = 0; i < depth; ++i) {
    view = std::make_unique<ProbeFrame>(app.context(), std::move(view));
  }
  app.add(std::move(view), display.extents());
#ifndef ROO_REDRAW_BASELINE
  app.window().setAdvisoryPaintBudget(roo_time::Millis(16));
#endif
  app.refresh();
  app.start();
  scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  device.model_wire = true;
  device.post_input = true;
  constexpr int kFrames = 300;
  std::vector<int64_t> cpu;
  std::vector<int64_t> duration;
  std::vector<int64_t> foreground;
  std::vector<int64_t> input;
  cpu.reserve(kFrames);
  duration.reserve(kFrames);
  foreground.reserve(kFrames);
  input.reserve(kFrames);
  uint64_t pixel_sum = 0;
  uint64_t copy_sum = 0;
  uint64_t duplicate_sum = 0;
  size_t allocation_sum = 0;
  int max_age = 0;
  roo_time::Uptime previous_foreground = roo_time::Uptime::Max();
  int missing_foreground = 0;
  int position = 0;
  int direction = -1;
  for (int frame = 0; frame < kFrames * 2; ++frame) {
    if (position - delta < -480) direction = 1;
    if (position + delta > 0) direction = -1;
    position += direction * delta;
    panel->scrollTo(0, position);
    allocations = 0;
    tracking = frame >= kFrames;
    Clock::time_point start = Clock::now();
    PaintNext(scheduler, stats);
    int64_t host_us = std::chrono::duration_cast<std::chrono::microseconds>(
                          Clock::now() - start)
                          .count();
    tracking = false;
    if (frame < kFrames) continue;
    allocation_sum += allocations;
    cpu.push_back(host_us);
    duration.push_back(device.pixels * 800 / 1000);
    if (device.first_white != roo_time::Uptime::Max()) {
      if (previous_foreground != roo_time::Uptime::Max()) {
        foreground.push_back(
            (device.first_white - previous_foreground).inMicros());
      }
      previous_foreground = device.first_white;
    } else {
      ++missing_foreground;
    }
    input.push_back(keys.latency);
    pixel_sum += device.pixels;
    copy_sum += device.copied_pixels;
    duplicate_sum += device.duplicates;
    max_age = std::max(max_age, device.maxWriteAge());
  }
  device.post_input = false;
  // Pending input can dispatch before the already scheduled cleanup refresh.
  bool cleanup_needed = app.root().isDirty();
  Clock::time_point cleanup_start = Clock::now();
  if (cleanup_needed) PaintNext(scheduler, stats);
  int64_t cleanup_cpu = std::chrono::duration_cast<std::chrono::microseconds>(
                            Clock::now() - cleanup_start)
                            .count();
  if (!cleanup_needed) cleanup_cpu = 0;
  int64_t cleanup_wire = cleanup_needed ? device.pixels * 800 / 1000 : 0;
  EXPECT_EQ(duplicate_sum, 0u);
  // An unconditional complete repair must not alter the settled image.
  std::vector<roo::byte> settled = framebuffer;
  app.root().invalidateInterior();
  app.refresh();
  EXPECT_EQ(settled, framebuffer);
  std::printf(
      "redraw_case,%d,%d,%d,%d,%d,%d,%d,%d,%llu,%llu,%lld,%lld,%lld,%lld,%lld,%"
      "lld,%lld,%lld,%lld,%lld,%zu,%d,%d,%zu\n",
      width, height, clears, overlays, depth, opaque, delta, mode,
      static_cast<unsigned long long>(pixel_sum / kFrames),
      static_cast<unsigned long long>(copy_sum / kFrames),
      static_cast<long long>(Percentile(cpu, 50)),
      static_cast<long long>(Percentile(cpu, 95)),
      static_cast<long long>(Percentile(duration, 50)),
      static_cast<long long>(Percentile(duration, 95)),
      static_cast<long long>(Percentile(foreground, 50)),
      static_cast<long long>(Percentile(foreground, 95)),
      static_cast<long long>(Percentile(input, 50)),
      static_cast<long long>(Percentile(input, 95)),
      static_cast<long long>(cleanup_cpu), static_cast<long long>(cleanup_wire),
      allocation_sum, max_age, missing_foreground, stats.exclusion_capacity);
  std::fflush(stdout);
}

// Measures the specified workload matrix with 300 warm and 300 moving paints
// per case, followed by stationary cleanup. The environment limit enables a
// quick smoke run; unset it for the complete matrix.
TEST(AcceleratedRedrawBenchmark, Matrix) {
  const char* limit_text = std::getenv("ROO_REDRAW_CASE_LIMIT");
  int limit = limit_text == nullptr ? 0 : std::atoi(limit_text);
  int cases = 0;
  std::printf(
      "record,width,height,clears,overlays,depth,opaque,delta,mode,pixels_mean,"
      "copy_pixels_mean,cpu_median_us,cpu_p95_us,model_refresh_median_us,model_"
      "refresh_p95_us,white_interval_median_us,white_interval_p95_us,input_"
      "median_us,input_p95_us,cleanup_cpu_us,cleanup_model_us,allocations,max_"
      "write_age,missing_white,exclusion_capacity\n");
  for (int size : {0, 1}) {
    for (int clears : {1, 20, 100}) {
      for (int overlays : {0, 8, 32}) {
        for (int depth : {0, 1, 3}) {
          for (bool opaque : {false, true}) {
            for (int delta : {2, 8, 24}) {
              for (int mode : {0, 1, 2}) {
                if (limit > 0 && cases >= limit) return;
                RunCase(size == 0 ? 240 : 320, size == 0 ? 320 : 480, clears,
                        overlays, depth, opaque, delta, mode);
                ++cases;
              }
            }
          }
        }
      }
    }
  }
}

// Reports actual host ABI sizes without claiming embedded-target measurements.
TEST(AcceleratedRedrawBenchmark, AbiSizes) {
  size_t scope_size = 0;
  size_t suspension_size = 0;
#ifndef ROO_REDRAW_BASELINE
  scope_size = sizeof(internal::BackgroundDeferralScope);
  suspension_size = sizeof(internal::BackgroundDeferralSuspension);
#endif
  std::printf(
      "abi Widget=%zu Container=%zu SimpleScrollablePanel=%zu "
      "AcceleratedScrollablePanel=%zu Canvas=%zu PaintContext=%zu "
      "DisplayWindow=%zu Clipper=%zu Overlay=%zu Scope=%zu Suspension=%zu\n",
      sizeof(Widget), sizeof(Container), sizeof(SimpleScrollablePanel),
      sizeof(OptionalProbePanel), sizeof(Canvas), sizeof(PaintContext),
      sizeof(DisplayWindow), sizeof(Clipper), sizeof(internal::ClippedOverlay),
      scope_size, suspension_size);
  EXPECT_LE(sizeof(PaintContext), sizeof(Canvas) + sizeof(void*));
}

}  // namespace
}  // namespace roo_windows
