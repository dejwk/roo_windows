// Learning goal: badge a standard navigation destination with an unread count
// and clear that count when the user opens the destination.

// *************** EMULATOR SETUP BEGIN

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

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Change these pins and the touch calibration for your display board.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;
static constexpr int kTouchCsPin = 1;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Navigation bar that clears the alert badge when Alerts is selected.
class NotificationNavigation : public material3::NavigationBar {
 public:
  /// Creates navigation bound to visible feedback and its badged destination.
  NotificationNavigation(ApplicationContext& context, TextLabel& feedback,
                         material3::BadgedNavigationBarDestination& alerts)
      : material3::NavigationBar(context),
        feedback_(feedback),
        alerts_(alerts) {}

 protected:
  /// Maps destination selection to content and unread-count state.
  void onSelectedIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    if (new_index == 1) {
      alerts_.hideBadge();
      feedback_.setText("Alerts reviewed · no unread notifications");
    } else {
      feedback_.setText(new_index == 0 ? "Pool temperature is 27.4 °C"
                                       : "Automation settings ready");
    }
  }

 private:
  TextBlock& feedback_;
  material3::BadgedNavigationBarDestination& alerts_;
};

/// Compact controller whose alert destination carries an unread count.
class NotificationCount : public FlexLayout {
 public:
  /// Creates three standard destinations and initializes two unread alerts.
  explicit NotificationCount(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        content_(context, FlexDirection::kColumn),
        title_(context, "Pool controller", material3::text_style_title_large()),
        feedback_(context, "Pool temperature is 27.4 °C",
                  material3::text_style_body_large()),
        status_(context, "Status", &ic_outlined_24_navigation_home_work()),
        alerts_(context, "Alerts",
                &ic_outlined_24_action_circle_notifications()),
        settings_(context, "Settings",
                  &ic_outlined_24_action_display_settings()),
        navigation_(context, feedback_, alerts_) {
    content_.setPadding(Padding(Scaled(16), Scaled(18)));
    content_.setGap(Scaled(8));
    content_.add(title_);
    content_.add(feedback_);

    // Prefer the destination subclass over manually positioning Badge.
    alerts_.setBadgeValue(2);
    navigation_.add(status_);
    navigation_.add(alerts_);
    navigation_.add(settings_);

    add(content_, {.flex_grow = 1});
    add(navigation_);
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextBlock feedback_;
  material3::NavigationBarDestination status_;
  material3::BadgedNavigationBarDestination alerts_;
  material3::NavigationBarDestination settings_;
  NotificationNavigation navigation_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
NotificationCount notification_count(app.context());
SingletonActivity activity(app, notification_count);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
