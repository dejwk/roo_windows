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

#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/widgets/button.h"

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);

// Simple container to put our button in. (Without it, the button would take
// full screen).
class MyPane : public AlignedLayout {
 public:
  MyPane(const Environment& env)
      : AlignedLayout(env), button_(env, "Click me!") {
    add(button_, kCenter | kMiddle);
    button_.setOnInteractiveChange([this]() {
      getTask()->showAlertDialog("Notification", "The button has been clicked.",
                                 {"OK"}, nullptr);
    });
  }

 private:
  SimpleButton button_;
};

MyPane my_pane(env);
SingletonActivity activity(app, my_pane);

void setup() {
  SPI.begin();
  display.init();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}