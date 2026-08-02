// Learning goal: add a familiar pictogram inside a larger slider track.
// Adjust the pool light and observe that the icon remains part of the control.

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
#include "roo_icons/outlined/24/device.h"
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

/// Makes enough room in the track for an inset icon.
constexpr material3::SliderStyle BrightnessStyle() {
  material3::SliderStyle style;
  style.size = material3::SliderSize::kLarge;
  style.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return style;
}

/// Icon slider that publishes pool-light brightness to status text.
class BrightnessSlider : public material3::SliderWithInsetIcon {
 public:
  /// Creates a brightness slider and installs its non-owning pictogram.
  BrightnessSlider(ApplicationContext& context, TextLabel& value_label)
      : material3::SliderWithInsetIcon(context, {0.0f, 100.0f, 5.0f}, 65.0f,
                                       material3::SliderVariant::kStandard,
                                       BrightnessStyle()),
        value_label_(value_label) {
    setIcon(&ic_outlined_24_device_brightness_high());
    updateLabel(value());
  }

  /// Reflects brightness changes in the application state label.
  void onValueChange(float value, bool from_user) override {
    (void)from_user;
    updateLabel(value);
  }

  /// Formats the value bubble as a percentage.
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (scratch_size == 0) return {};
    int length = snprintf(scratch, scratch_size, "%.0f%%", value);
    if (length < 0) length = 0;
    if (static_cast<size_t>(length) >= scratch_size) {
      length = static_cast<int>(scratch_size - 1);
    }
    return roo::string_view(scratch, static_cast<size_t>(length));
  }

 private:
  void updateLabel(float value) {
    value_label_.setTextf("Pool light: %.0f%%", value);
  }

  TextLabel& value_label_;
};

/// Lighting screen demonstrating a standard inset-icon use case.
class InsetIconSlider : public FlexLayout {
 public:
  /// Creates the pool-light brightness lesson.
  explicit InsetIconSlider(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pool lighting", material3::text_style_title_large()),
        guidance_(context, "Inset icons work best on larger standard sliders",
                  material3::text_style_body_medium(), kTop | kLeft),
        value_(context, "", material3::text_style_title_medium()),
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
  BrightnessSlider slider_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
InsetIconSlider inset_icon_slider(app.context());
SingletonActivity activity(app, inset_icon_slider);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
