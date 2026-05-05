// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flexViewport;

  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flexViewport(viewport, 1, FlexViewport::kRotationRight),
        display(flexViewport),
        touch(flexViewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                        true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Select the driver to match your display device.
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

// Set your configuration for the driver.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kBlPin = 20;

static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;

static constexpr int kTouchCsPin = 1;

// Uncomment if you have connected the BL pin to GPIO.

// #include "roo_display/backlit/esp32_ledc.h"
// LedcBacklit backlit(kBlPin, /* ledc channel */ 0);

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.init();
}

// *************** EXAMPLE STARTS HERE

// Example 1: Toolbar
//
// A horizontal row at the bottom of the screen with a navigation back-button
// on the left, a centered title label that stretches to fill the available
// space, and a row of three icon action-buttons on the right, all aligned to
// the center of the cross axis.

#include "roo_icons/filled/24/action.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

class Toolbar : public FlexLayout {
 public:
  Toolbar(const Environment& env)
      : FlexLayout(env, FlexDirection::kRow),
        back_(env, ic_filled_24_navigation_arrow_back()),
        title_(env, "My screen", font_body1()),
        search_(env, ic_filled_24_action_search()),
        settings_(env, ic_filled_24_action_settings()),
        overflow_(env, ic_filled_24_navigation_more_vert()) {
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(PaddingSize::kSmall, PaddingSize::kNone));
    setGap(Scaled(4));
    add(back_, {.flex_grow = 0, .flex_shrink = 0});
    add(title_, {.flex_grow = 1, .flex_shrink = 1});
    add(search_, {.flex_grow = 0, .flex_shrink = 0});
    add(settings_, {.flex_grow = 0, .flex_shrink = 0});
    add(overflow_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  Icon back_;
  TextLabel title_;
  Icon search_;
  Icon settings_;
  Icon overflow_;
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
Toolbar toolbar(env);
SingletonActivity activity(app, toolbar);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
