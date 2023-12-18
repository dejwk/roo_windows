#include "Arduino.h"

#include "roo_display.h"
#include "roo_material_icons.h"
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

#include "roo_material_icons/filled/action.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/widgets/icon.h"

roo_scheduler::Scheduler scheduler;
Environment env;
Application app(&env, display, scheduler);

// Simple container to put our button in. (Without it, the button would take
// full screen).
class MyPane : public HorizontalLayout {
 public:
  MyPane(const Environment& env)
      : HorizontalLayout(env),
        // Scaled (default).
        icon1_(env, SCALED_ROO_ICON(filled, action_rocket)),
        icon2_(env, ic_filled_18_action_rocket()),
        icon3_(env, ic_filled_24_action_rocket()),
        icon4_(env, ic_filled_36_action_rocket()),
        icon5_(env, ic_filled_48_action_rocket()) {
    add(icon1_);
    add(icon2_);
    add(icon3_);
    add(icon4_);
    icon5_.setColor(color::BlueViolet);
    add(icon5_);
  }

 private:
  Icon icon1_;
  Icon icon2_;
  Icon icon3_;
  Icon icon4_;
  Icon icon5_;
};

MyPane my_pane(env);
SingletonActivity activity(app, my_pane);

void setup() {
  SPI.begin();
  display.init();
}

void loop() {
  app.tick();
  scheduler.executeEligibleTasks(1);
}
