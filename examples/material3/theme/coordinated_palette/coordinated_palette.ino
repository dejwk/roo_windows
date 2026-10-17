// Learning goal: select a complete light/dark palette and compare its colors
// on primary, secondary, tertiary, neutral, and borrowed Material 2 controls.

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

#include "Arduino.h"
#include "palette_preview.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/core/task.h"

using namespace roo_display;
using namespace roo_windows;

Ili9341spi<7, 2, 3> screen(Orientation().rotateLeft());
TouchXpt2046<1> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

roo_scheduler::SchedulingService scheduler;
Theme theme;
Environment environment(scheduler, theme);
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif
void setup() {
  SPI.begin(4, 5, 6);
  display.enableTurbo();
  display.init();
  // A navigable task supplies the owner needed by menus and text editing.
  // Keep its borrowed preview alive throughout the scheduler loop.
  coordinated_palette_example::PalettePreview preview(app, theme);
  Task& task = app.addTaskFullScreen(preview);
  app.start();
  scheduler.run();
  task.navigation().clear();
}

void loop() {}
