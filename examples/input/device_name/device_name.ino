// Edit a device name in place with physical keys or a serial terminal.
// Build the entire library with ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD=0.
// The Bazel target selects that library configuration automatically.

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
      : viewport({.noise_bits = 1}),
        flex_viewport(viewport, 1, FlexViewport::kRotationNone),
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

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;
static constexpr int kTouchCsPin = 1;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen{Orientation()};
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

// Adjust the SPI pins and touch calibration above for your display.
void InitDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

#include "roo_windows/containers/flex_layout.h"
#if ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD
#error \
    "Build this example and roo_windows with ROO_WINDOWS_ENABLE_SOFTWARE_KEYBOARD=0"
#endif
#include "roo_windows/material3/text_field/text_field.h"

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
Application app(&env, display);
FlexLayout form(app.context(), FlexDirection::kColumn);
material3::TextField place(app.context(), "Device name");
TextInputEmitter serial_input;

void setup() {
  InitDisplay();
  Serial.begin(115200);
  serial_input.connect(app);
  place.setSupportingText("Type on the keyboard or serial terminal");
  form.setPadding(Padding(Scaled(8)));
  form.add(place);
  Task& task = app.addTaskFullScreen(form);
#ifdef ROO_TESTING
  emulator_keys.connect(task);
#endif
  place.edit();
  app.start();
}

// Serial is an ASCII semantic-input adapter for physical boards; a USB/BLE
// keyboard can instead supply normalized KeyEvents through KeySource.
void loop() {
  while (Serial.available() > 0) {
    int value = Serial.read();
    if (value == 8 || value == 127) {
      serial_input.deleteBackward();
    } else if (value >= 32 && value < 127) {
      serial_input.commitRune(value);
    }
  }
  scheduler.executeEligibleTasksUpToNow();
}
