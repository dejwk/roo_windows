#pragma once

#ifdef ROO_TESTING
#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#include "roo_windows/fake/fltk_key_source.h"
#endif

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

namespace material3_menu_example {

#ifdef ROO_TESTING
// The emulator supplies the same 320x240 display, touch controller, and key
// source used by the sketch. A hardware build skips this adapter entirely.
struct Emulator {
  roo_testing_transducers::FltkViewport viewport;
  roo_testing_transducers::FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flex_viewport(viewport, 1,
                      roo_testing_transducers::FlexViewport::kRotationRight),
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
};

inline Emulator emulator;
inline roo_windows::fake::FltkKeySource emulator_keys;
#endif

// This is the physical-device-facing stack. Change pin numbers and calibration
// here when adapting any menu example to another display board.
inline roo_display::Ili9341spi<7, 2, 3> screen(
    roo_display::Orientation().rotateLeft());
inline roo_display::TouchXpt2046<1> touch;
inline roo_display::Display display(
    screen, touch,
    roo_display::TouchCalibration(269, 249, 3829, 3684,
                                  roo_display::Orientation::LeftDown()));
inline roo_scheduler::Scheduler scheduler;
inline roo_windows::Environment environment(scheduler);
#ifdef ROO_TESTING
inline roo_windows::Application app(&environment, display, emulator_keys, true);
#else
inline roo_windows::Application app(&environment, display);
#endif

inline void Start() {
  // Roo's scheduler owns the application loop, so Arduino loop() stays empty.
  SPI.begin(4, 5, 6);
  display.enableTurbo();
  display.init();
  app.start();
  scheduler.run();
}

}  // namespace material3_menu_example
