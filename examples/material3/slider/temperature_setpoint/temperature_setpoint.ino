// Learning goal: model a temperature setpoint directly in degrees Celsius.
// Drag the slider and observe the application label and value bubble update.

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

/// Returns a readable touch target with a bubble during interaction.
constexpr material3::SliderStyle SetpointStyle() {
  material3::SliderStyle style;
  style.size = material3::SliderSize::kLarge;
  style.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return style;
}

/// Slider that publishes its semantic Celsius value to application UI.
class SetpointSlider : public material3::Slider {
 public:
  /// Creates a 0.5-degree setpoint slider and its visible value binding.
  SetpointSlider(ApplicationContext& context, TextLabel& value_label)
      : material3::Slider(context, {18.0f, 32.0f, 0.5f}, 27.0f,
                          material3::SliderVariant::kStandard, SetpointStyle()),
        value_label_(value_label) {
    updateLabel(value());
  }

  /// Reflects user and programmatic changes in the screen's state label.
  void onValueChange(float value, bool from_user) override {
    (void)from_user;
    updateLabel(value);
  }

  /// Formats the transient value bubble without allocating memory.
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (scratch_size == 0) return {};
    int length = snprintf(scratch, scratch_size, "%.1f °C", value);
    if (length < 0) length = 0;
    if (static_cast<size_t>(length) >= scratch_size) {
      length = static_cast<int>(scratch_size - 1);
    }
    return roo::string_view(scratch, static_cast<size_t>(length));
  }

 private:
  void updateLabel(float value) { value_label_.setTextf("%.1f °C", value); }

  TextLabel& value_label_;
};

/// Heating screen that keeps UI state in the slider's semantic domain.
class TemperatureSetpoint : public FlexLayout {
 public:
  /// Creates the heating setpoint lesson.
  explicit TemperatureSetpoint(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Heating setpoint",
               material3::text_style_title_large()),
        guidance_(context, "Range: 18.0 °C to 32.0 °C in 0.5-degree steps",
                  material3::text_style_body_medium(), kTop | kLeft),
        value_(context, "", material3::text_style_display_small()),
        slider_(context, value_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(14));
    add(title_);
    add(guidance_);
    add(value_);
    add(slider_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  TextLabel value_;
  SetpointSlider slider_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
TemperatureSetpoint temperature_setpoint(app.context());
UiTask& task = app.addUiTaskFullScreen(temperature_setpoint);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
