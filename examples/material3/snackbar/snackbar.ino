// Material 3 snackbar queue, shared-track motion, semantic timeout,
// replacement, and obstacle-placement catalog.

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

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/snackbar/snackbar.h"

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif

material3::SnackbarHost host(app.context());
material3::Button short_button(app.context(), "Short message");
material3::Button queue_button(app.context(), "Queue messages");
material3::Button replace_button(app.context(), "Replace current");
material3::Button avoid_button(app.context(), "Lift above obstacle");
FlexLayout controls(app.context(), FlexDirection::kColumn);
material3::SnackbarRequest short_message, persistent, queued, replacement;
void setup() {
  short_message.configure("Settings saved");
  persistent.configure("Schedule removed", "Undo",
                       material3::SnackbarDuration::kDefault, true);
  queued.configure("Waiting message");
  replacement.configure("Updated feedback");
  short_button.setOnInteractiveChange([] {
    host.setSnackbarAvoidance(nullptr, 0);
    host.snackbars().show(short_message);
  });
  queue_button.setOnInteractiveChange([] {
    host.setSnackbarAvoidance(nullptr, 0);
    host.snackbars().show(persistent);
    host.snackbars().show(queued);
  });
  replace_button.setOnInteractiveChange([] {
    host.setSnackbarAvoidance(nullptr, 0);
    host.snackbars().replaceCurrent(replacement);
  });
  avoid_button.setOnInteractiveChange([] {
    const Rect band = host.bodyBounds();
    const Rect obstacle(band.xMin(), band.yMax() - Scaled(64) + 1, band.xMax(),
                        band.yMax());
    host.setSnackbarAvoidance(&obstacle, 1);
    host.snackbars().show(short_message);
  });
  controls.setGap(Scaled(8));
  controls.setPadding(Padding(Scaled(8)));
  controls.add(avoid_button);
  controls.add(short_button);
  controls.add(queue_button);
  controls.add(replace_button);
  host.setBody(controls);
  app.addTaskFullScreen(host);
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
