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

// Example 3: Wrapping chip bar

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/widgets/button.h"

class ChipBar : public FlexLayout {
 public:
  ChipBar(const Environment& env)
      : FlexLayout(env, FlexDirection::kRow),
        chips_{
            SimpleButton(env, "All", Button::OUTLINED),
            SimpleButton(env, "Nearby", Button::OUTLINED),
            SimpleButton(env, "Open now", Button::OUTLINED),
            SimpleButton(env, "Top rated", Button::OUTLINED),
            SimpleButton(env, "Price", Button::OUTLINED),
            SimpleButton(env, "Food", Button::OUTLINED),
            SimpleButton(env, "Hotels", Button::OUTLINED),
            SimpleButton(env, "Activities", Button::OUTLINED),
        } {
    setFlexWrap(FlexWrap::kWrap);
    setJustifyContent(JustifyContent::kFlexStart);
    setAlignContent(AlignContent::kFlexStart);
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(PaddingSize::kSmall));
    setColumnGap(Scaled(8));
    setRowGap(Scaled(8));

    for (auto& chip : chips_) {
      add(chip, {.flex_grow = 0, .flex_shrink = 0});
    }
  }

 private:
  SimpleButton chips_[8];
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
ChipBar chip_bar(env);
SingletonActivity activity(app, chip_bar);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
