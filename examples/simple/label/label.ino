

#include "Arduino.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/widgets/text_label.h"

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

#include "roo_windows/widgets/text_label.h"

roo_scheduler::Scheduler scheduler;
Environment env;
Application app(&env, display, scheduler);
TextLabel label(env, "Hello World!", font_caption(), kCenter | kMiddle);
SingletonActivity activity(app, label);

void setup() {
  SPI.begin();
  display.init();
}

void loop() {
  app.tick();
  scheduler.executeEligibleTasks(1);
}
