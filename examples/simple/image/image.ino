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
static constexpr int kCsPin = 5;
static constexpr int kDcPin = 17;
static constexpr int kRstPin = 27;
static constexpr int kBlPin = 16;

static constexpr int kTouchCsPin = 2;

// Uncomment if you have connected the BL pin to GPIO.

// #include "roo_display/backlit/esp32_ledc.h"
// LedcBacklit backlit(kBlPin, /* ledc channel */ 0);

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

#include "roo_display/shape/smooth.h"
#include "roo_windows/widgets/image.h"

roo_scheduler::Scheduler scheduler;
Environment env;
Application app(&env, display, scheduler);

// An 'image' really means any drawable, as understood by roo_display. We're
// using a simple geometric shape here, but you can also draw images in various
// formats from an SD card or PROGMEM, or any other synthetic content that is
// captured via the roo_display::Drawable interface. See
// https://github.com/dejwk/roo_display to learn more.
auto pie = SmoothPie({0, 0}, 40, 0, M_PI / 6, env.theme().color.primaryVariant);
Image img(env, pie);

SingletonActivity activity(app, img);

void setup() {
  SPI.begin();
  display.init();
}

void loop() {
  app.tick();
  scheduler.executeEligibleTasks(1);
}
