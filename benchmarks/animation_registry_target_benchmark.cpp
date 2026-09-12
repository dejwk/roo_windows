// ESP32-C3 runtime benchmark for the animation-registry Phase 16 gate.
//
// This source is copied into the PlatformIO target fixture described in
// docs/animation_registry_acceptance.md. It is not part of the library.
#include <Arduino.h>
#include <esp_cpu.h>
#include <esp_rom_sys.h>

#include <algorithm>
#include <array>
#include <memory>

#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/animation_registry.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows::test {

struct AnimationRegistryTestAccess {
  static void dispatch(AnimationRegistry& registry, roo_time::Uptime now) {
    registry.beginFrame(now);
    while (registry.dispatchNext()) {
    }
    registry.endFrame();
  }
};

namespace {

constexpr uint32_t kCpuHz = 160000000;
constexpr uint32_t kTwoMillisCycles = kCpuHz / 500;

class TestApplication {
 public:
  TestApplication()
      : device_(16, 16, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_) {}

  ApplicationContext& context() { return app_.context(); }
  AnimationRegistry& registry() { return context().animations(); }

 private:
  roo::byte raster_[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
};

class ProbeWidget final : public BasicWidget {
 public:
  explicit ProbeWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }

  void pauseSelfOnFrame(bool enabled) { pause_self_ = enabled; }

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override {
    (void)sample;
    if (pause_self_) context().animations().pause(*this, tag);
  }

 private:
  bool pause_self_ = false;
};

uint32_t MeasureDispatchCycles(EasingKind easing, bool mutation_heavy) {
  TestApplication fixture;
  std::array<std::unique_ptr<ProbeWidget>, 16> widgets;
  AnimationSpec spec = AnimationSpec::value(0.0f, 1.0f, roo_time::Seconds(10));
  spec.minimum_interval = roo_time::Duration();
  spec.easing.kind = easing;
  if (easing == EasingKind::kCubicBezier) {
    spec.easing.x1 = 0.2f;
    spec.easing.y1 = 0.0f;
    spec.easing.x2 = 0.0f;
    spec.easing.y2 = 1.0f;
  }
  for (auto& widget : widgets) {
    widget = std::make_unique<ProbeWidget>(fixture.context());
    fixture.registry().start(*widget, 0, spec);
  }
  AnimationRegistryTestAccess::dispatch(
      fixture.registry(), roo_time::Uptime::Start() + roo_time::Seconds(1));
  for (auto& widget : widgets) widget->pauseSelfOnFrame(mutation_heavy);

  uint32_t worst = 0;
  roo_time::Uptime now = roo_time::Uptime::Start() + roo_time::Seconds(2);
  for (int round = 0; round < 200; ++round) {
    if (mutation_heavy) {
      for (auto& widget : widgets) fixture.registry().resume(*widget, 0);
    }
    now += roo_time::Millis(1);
    const uint32_t begin = esp_cpu_get_cycle_count();
    AnimationRegistryTestAccess::dispatch(fixture.registry(), now);
    const uint32_t elapsed = esp_cpu_get_cycle_count() - begin;
    worst = std::max(worst, elapsed);
  }
  return worst;
}

}  // namespace
}  // namespace roo_windows::test

void setup() {
  delay(100);
  const uint32_t linear = roo_windows::test::MeasureDispatchCycles(
      roo_windows::EasingKind::kLinear, false);
  const uint32_t bezier = roo_windows::test::MeasureDispatchCycles(
      roo_windows::EasingKind::kCubicBezier, false);
  const uint32_t mutation = roo_windows::test::MeasureDispatchCycles(
      roo_windows::EasingKind::kLinear, true);
  esp_rom_printf("linear_cycles=%lu bezier_cycles=%lu mutation_cycles=%lu\n",
                 static_cast<unsigned long>(linear),
                 static_cast<unsigned long>(bezier),
                 static_cast<unsigned long>(mutation));
  esp_rom_printf(linear < roo_windows::test::kTwoMillisCycles &&
                         bezier < roo_windows::test::kTwoMillisCycles &&
                         mutation < roo_windows::test::kTwoMillisCycles
                     ? "ANIMATION_BENCHMARK_PASS\n"
                     : "ANIMATION_BENCHMARK_FAIL\n");
}

void loop() { delay(1000); }
