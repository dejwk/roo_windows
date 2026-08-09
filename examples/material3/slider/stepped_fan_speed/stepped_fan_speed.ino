// Learning goal: represent discrete equipment levels with a stepped slider.
// Drag or use arrow keys; ticks, the value bubble, and status stay
// synchronized.

// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#include "roo_windows/fake/fltk_key_source.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flex_viewport(viewport, 1, FlexViewport::kRotationRight),
        display(flex_viewport),
        touch(flex_viewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                         true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

roo_windows::fake::FltkKeySource emulator_keys;

#endif

// *************** DISPLAY SETUP BEGIN

#include <stdio.h>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Change these pins and the touch calibration for your display board.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;
static constexpr int kTouchCsPin = 1;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"

namespace {

/// Shows every valid discrete value and its interaction bubble.
constexpr material3::SliderStyle FanSpeedStyle() {
  material3::SliderStyle style;
  style.size = material3::SliderSize::kLarge;
  style.tick_mode = material3::SliderTickMode::kShowTicks;
  style.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return style;
}

/// Slider that binds discrete speed changes to equipment status text.
class FanSpeedSlider : public material3::Slider {
 public:
  /// Creates fan levels zero through five, with one level per step.
  FanSpeedSlider(ApplicationContext& context, TextLabel& status)
      : material3::Slider(context, {0.0f, 5.0f, 1.0f}, 2.0f,
                          material3::SliderVariant::kStandard, FanSpeedStyle()),
        status_(status) {
    updateStatus(value());
  }

  /// Updates application state whenever the snapped level changes.
  void onValueChange(float value, bool from_user) override {
    (void)from_user;
    updateStatus(value);
  }

  /// Formats the bubble as a fan multiplier.
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (scratch_size == 0) return {};
    int length = snprintf(scratch, scratch_size, "%.0fx", value);
    if (length < 0) length = 0;
    if (static_cast<size_t>(length) >= scratch_size) {
      length = static_cast<int>(scratch_size - 1);
    }
    return roo::string_view(scratch, static_cast<size_t>(length));
  }

 private:
  void updateStatus(float value) {
    status_.setTextf("Fan speed %.0f of 5", value);
  }

  TextLabel& status_;
};

/// Ventilation screen demonstrating a stepped semantic range.
class SteppedFanSpeed : public FlexLayout {
 public:
  /// Creates the discrete fan control lesson.
  explicit SteppedFanSpeed(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Ventilation", material3::text_style_title_large()),
        guidance_(context, "A step of 1 prevents unsupported fan speeds",
                  material3::text_style_body_medium(), kTop | kLeft),
        status_(context, "", material3::text_style_title_medium()),
        slider_(context, status_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(14));
    add(title_);
    add(guidance_);
    add(status_);
    add(slider_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  TextLabel status_;
  FanSpeedSlider slider_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
SteppedFanSpeed stepped_fan_speed(app.context());
UiTask& task = app.addUiTaskFullScreen(stepped_fan_speed);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
