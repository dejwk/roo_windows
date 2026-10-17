// Edit a device name in a separate draft: Back cancels, Clear empties it, and
// keyboard Done saves. The app default selects the material3 editor; it uses
// the application's one keyboard.

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

Ili9341spi<kCsPin, kDcPin, kRstPin> screen{Orientation().rotateLeft()};
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
#include "roo_windows/core/editor_destination.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

roo_scheduler::SchedulingService scheduler;
// Material 3 is the application default; no style override is needed.
Environment env(scheduler);
Application app(&env, display);
FlexLayout form(app.context(), FlexDirection::kColumn);
TextLabel name(app.context(), "Workshop controller",
               material3::text_style_body_large());
material3::Button rename_button(app.context(), "Rename device");
std::string device_name = "Workshop controller";

void setup() {
  InitDisplay();
  form.setPadding(Padding(Scaled(12)));
  form.setGap(Scaled(12));
  form.add(name);
  form.add(rename_button);
  Task& task = app.addTaskFullScreen(form);
#ifdef ROO_TESTING
  emulator_keys.connect(task);
#endif
  // The source setting changes only after confirmation and navigation removal.
  // These globals outlive the destination's temporary callback borrow.
  rename_button.setOnInteractiveChange([&task]() {
    app.context().editorDestination().triggerEdit(task.navigation(),
                                                  device_name, "Device name",
                                                  [](const std::string& value) {
                                                    device_name = value;
                                                    name.setText(device_name);
                                                  });
  });
  app.start();
  scheduler.run();
}

void loop() {}
