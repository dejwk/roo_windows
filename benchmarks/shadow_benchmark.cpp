
#ifdef ROO_TESTING

#include "roo_display/fake/reference_device.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"

using namespace roo_testing_transducers;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flexViewport;
  roo_display::ReferenceComboDevice tft;

  Emulator()
      : viewport(), flexViewport(viewport, 1), tft(1024, 600, flexViewport) {}
} emulator;

#endif

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"

#ifdef ROO_TESTING
roo_display::ReferenceComboDevice& device = emulator.tft;
#else
#include "roo_display/products/makerfabs/esp32s3_parallel_ips_capacitive_70.h"
roo_display::products::makerfabs::Esp32s3ParallelIpsCapacitive1024x600 device(
    roo_display::Orientation().rotateUpsideDown());
#endif

roo_display::Display display(device);

namespace roo_windows {
namespace {

// class SolidWidget : public BasicWidget {
//  public:
//   SolidWidget(const Environment& env, roo_display::Color color, Dimensions
//   dims)
//       : BasicWidget(env), color_(color), dims_(dims) {}

//   roo_display::Color background() const override { return color_; }

//   void paint(const Canvas& canvas) const override { canvas.clear(); }

//   Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

//  private:
//   roo_display::Color color_;
//   Dimensions dims_;
// };

class DecoratedWidget : public BasicSurfaceWidget {
 public:
  DecoratedWidget(const Environment& env, roo_display::Color fill_color,
                  roo_display::Color outline_color, BorderStyle border_style,
                  uint8_t elevation, Dimensions dims)
      : BasicSurfaceWidget(env),
        fill_color_(fill_color),
        outline_color_(outline_color),
        border_style_(border_style),
        elevation_(elevation),
        dims_(dims) {}

  roo_display::Color background() const override { return fill_color_; }

  roo_display::Color getOutlineColor() const override { return outline_color_; }

  BorderStyle getBorderStyle() const override { return border_style_; }

  uint8_t getElevation() const override { return elevation_; }

  void paint(const Canvas& canvas) const override { canvas.clear(); }
  Margins getMargins() const override { return Margins(MarginSize::kHuge); }

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

 private:
  roo_display::Color fill_color_;
  roo_display::Color outline_color_;
  BorderStyle border_style_;
  uint8_t elevation_;
  Dimensions dims_;
};

class BenchmarkScene {
 public:
  static constexpr int16_t kWidth = 180;
  static constexpr int16_t kHeight = 128;

  BenchmarkScene(BorderStyle border_style, uint8_t elevation)
      : raster_(static_cast<size_t>(kWidth) * kHeight * 2),
        offscreen_(kWidth, kHeight, raster_.data(), roo_display::Argb4444()),
        // display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display),
        card_(nullptr) {
    auto backdrop = std::make_unique<roo_windows::FlexLayout>(env_);

    auto card = std::make_unique<DecoratedWidget>(
        env_, roo_display::Color(0xFFE9A334), roo_display::Color(0xFF0F7FBF),
        border_style, elevation, Dimensions(92, 58));
    card_ = card.get();
    backdrop->add(std::move(card));
    // app_.add(WidgetRef(std::move(card)), roo_display::Box(44, 30, 135, 87));
    app_.add(WidgetRef(std::move(backdrop)),
             roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

    // Warm initialization pass (layout + first paint).
    app_.refresh();
  }

  bool RenderOneFrame() {
    card_->invalidateInterior();
    return app_.refresh();
  }

 private:
  std::vector<roo::byte> raster_;
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  Application app_;
  DecoratedWidget* card_;
};

struct BenchmarkCase {
  const char* name;
  BorderStyle border_style;
  uint8_t elevation;
  bool equal_radii;
};

struct BenchmarkResult {
  double avg_us;
  double min_us;
  double max_us;
};

inline double fpsFromUs(double us) { return us <= 0 ? 0.0 : 1000000.0 / us; }

void PrintLine(const BenchmarkCase& c, double avg_us, double min_us,
               double max_us) {
  Serial.print(c.name);
  Serial.print(" | radii=");
  Serial.print(c.equal_radii ? "equal" : "mixed");
  Serial.print(" | avg_us=");
  Serial.print(avg_us, 2);
  Serial.print(" | min_us=");
  Serial.print(min_us, 2);
  Serial.print(" | max_us=");
  Serial.print(max_us, 2);
  Serial.print(" | fps=");
  Serial.println(fpsFromUs(avg_us), 2);
}

BenchmarkResult RunCase(const BenchmarkCase& c, int warmup, int iterations) {
  BenchmarkScene scene(c.border_style, c.elevation);
  for (int i = 0; i < warmup; ++i) {
    scene.RenderOneFrame();
  }

  double total_us = 0.0;
  double min_us = std::numeric_limits<double>::max();
  double max_us = 0.0;

  for (int i = 0; i < iterations; ++i) {
    uint32_t start = micros();
    if (!scene.RenderOneFrame()) {
      Serial.print("Refresh did not finish within deadline in case: ");
      Serial.println(c.name);
      break;
    }
    uint32_t elapsed = micros() - start;
    double us = elapsed;
    total_us += us;
    min_us = std::min(min_us, us);
    max_us = std::max(max_us, us);
  }

  return BenchmarkResult{
      iterations > 0 ? total_us / iterations : 0.0,
      iterations > 0 ? min_us : 0.0,
      iterations > 0 ? max_us : 0.0,
  };
}

}  // namespace
}  // namespace roo_windows

void RunBenchmark(int iterations, int warmup, int repeats) {
  using namespace roo_windows;

  std::vector<BenchmarkCase> cases = {
      {"sharp_no_outline_e0", BorderStyle(0, 0), 0, true},
      {"round_uniform_no_outline_e0", BorderStyle(14, 0), 0, true},
      {"round_variable_no_outline_e0", BorderStyle(4, 12, 18, 8, 0), 0, false},
      {"sharp_outline_e0", BorderStyle(0, SmallNumber::Of16ths(36)), 0, true},
      {"round_uniform_outline_e0", BorderStyle(14, SmallNumber::Of16ths(36)), 0,
       true},
      {"round_variable_outline_e0",
       BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)), 0, false},
      {"round_uniform_outline_e4", BorderStyle(14, SmallNumber::Of16ths(36)), 4,
       true},
      {"round_variable_outline_e4",
       BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)), 4, false},
      {"round_uniform_outline_e12", BorderStyle(14, SmallNumber::Of16ths(36)),
       12, true},
      {"round_variable_outline_e12",
       BorderStyle(4, 12, 18, 8, SmallNumber::Of16ths(36)), 12, false},
  };

  Serial.println();
  Serial.println("Shadow benchmark");
  Serial.print("iterations=");
  Serial.print(iterations);
  Serial.print(", warmup=");
  Serial.print(warmup);
  Serial.print(", repeats=");
  Serial.println(repeats);
  Serial.println(
      "----------------------------------------------"
      "--------------------------");

  double equal_sum = 0.0;
  double varying_sum = 0.0;
  int equal_count = 0;
  int varying_count = 0;

  for (const auto& c : cases) {
    double avg_sum = 0.0;
    double min_best = std::numeric_limits<double>::max();
    double max_worst = 0.0;

    for (int r = 0; r < repeats; ++r) {
      BenchmarkResult res = RunCase(c, warmup, iterations);
      avg_sum += res.avg_us;
      min_best = std::min(min_best, res.min_us);
      max_worst = std::max(max_worst, res.max_us);
    }

    double avg_us = avg_sum / repeats;
    if (c.equal_radii) {
      equal_sum += avg_us;
      ++equal_count;
    } else {
      varying_sum += avg_us;
      ++varying_count;
    }

    PrintLine(c, avg_us, min_best, max_worst);
  }

  Serial.println();
  if (equal_count > 0) {
    Serial.print("mean(equal radii)  : ");
    Serial.print(equal_sum / equal_count, 2);
    Serial.println(" us");
  }
  if (varying_count > 0) {
    Serial.print("mean(mixed radii)  : ");
    Serial.print(varying_sum / varying_count, 2);
    Serial.println(" us");
  }
}

void setup() {
  Serial.begin(115200);
  delay(250);
  device.initTransport();
  display.init();
}

void loop() {
  delay(200);

  // Keep defaults simple; if needed, this can be extended to parse runtime
  // values via serial commands.
  RunBenchmark(100, 10, 10);

#ifdef ROO_BENCHMARK_EXIT_AFTER_RUN
  std::exit(0);
#endif
}
