#include "Arduino.h"

#include "roo_display.h"
#include "roo_icons.h"
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

#include "roo_icons/filled/24/action.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/dialogs/alert_dialog.h"
#include "roo_windows/widgets/button.h"

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);

// Simple container to put our button in. (Without it, the button would take
// full screen).
class MyPane : public VerticalLayout {
 public:
  MyPane(const Environment& env)
      : VerticalLayout(env),
        button1_(env, "Contained text", Button::CONTAINED),
        button2_(env, SCALED_ROO_ICON(filled, action_arrow_circle_right),
                 "Outlined combo", Button::OUTLINED),
        button3_(env, SCALED_ROO_ICON(filled, action_done), Button::TEXT),
        button4_(env, "Custom", Button::OUTLINED) {
    button4_.setInteriorColor(roo_display::color::Beige);
    button4_.setOutlineColor(roo_display::color::DeepPink);
    button4_.setContentColor(roo_display::color::SaddleBrown);
    button4_.setCornerRadius(20);
    button4_.setElevation(10, 2);
    button4_.setPadding(PADDING_LARGE);
    button4_.setMargins(MARGIN_LARGE);
    button4_.setFont(font_h5());
    // You can further customize the button by subclassing it. For example, you
    // can override getBorderStyle() to change the outline thickness.
    add(button1_);
    add(button2_);
    add(button3_);
    add(button4_);
    button1_.setOnInteractiveChange([this]() {
      getTask()->showAlertDialog("Notification", "Button 1 clicked.", {"OK"},
                                 nullptr);
    });
    button2_.setOnInteractiveChange([this]() {
      getTask()->showAlertDialog("Notification", "Button 2 clicked.", {"OK"},
                                 nullptr);
    });
    button3_.setOnInteractiveChange([this]() {
      getTask()->showAlertDialog("Notification", "Button 3 clicked.", {"OK"},
                                 nullptr);
    });
    button4_.setOnInteractiveChange([this]() {
      getTask()->showAlertDialog("Notification", "Button 4 clicked.", {"OK"},
                                 nullptr);
    });
  }

 private:
  SimpleButton button1_;
  SimpleButton button2_;
  SimpleButton button3_;
  SimpleButton button4_;
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