// Learning goal: use a bottom navigation bar to switch an application's
// visible top-level destination. Tap a destination or use Left/Right when the
// bar has keyboard focus.

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

// NavigationBar owns the selection model. This small subclass translates the
// selected route index into application content after the state has changed.
/// Bottom navigation that maps route changes to the adjacent content view.
class PoolNavigationBar : public material3::NavigationBar {
 public:
  /// Creates a navigation bar with borrowed visible-state labels.
  PoolNavigationBar(ApplicationContext& context, TextLabel& title,
                    TextLabel& detail)
      : material3::NavigationBar(context), title_(title), detail_(detail) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    static constexpr const char* kTitles[] = {"Pool status", "Schedules",
                                              "Settings"};
    static constexpr const char* kDetails[] = {
        "Water 27.4 °C · Pump running", "Next pump cycle at 18:30",
        "Connectivity and safety controls"};
    title_.setText(kTitles[new_index]);
    detail_.setText(kDetails[new_index]);
  }

 private:
  TextLabel& title_;
  TextLabel& detail_;
};

/// Compact pool controller screen with persistent bottom navigation.
class PoolController : public FlexLayout {
 public:
  /// Creates the destination content and its shared selection model.
  explicit PoolController(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        content_(context, FlexDirection::kColumn),
        title_(context, "Pool status", material3::text_style_title_large()),
        detail_(context, "Water 27.4 °C · Pump running",
                material3::text_style_body_large()),
        status_(context, "Choose a top-level destination",
                material3::text_style_body_medium()),
        home_(context, "Status", &ic_outlined_24_navigation_home_work()),
        schedule_(context, "Schedule", &ic_outlined_24_action_calendar_month()),
        settings_(context, "Settings",
                  &ic_outlined_24_action_display_settings()),
        navigation_(context, title_, detail_) {
    content_.setPadding(Padding(Scaled(16), Scaled(18)));
    content_.setGap(Scaled(8));
    content_.add(title_);
    content_.add(detail_);
    content_.add(status_);

    // Add destinations in the same stable order used by the callback above.
    // The first destination becomes selected when it is added.
    navigation_.add(home_);
    navigation_.add(schedule_);
    navigation_.add(settings_);

    add(content_, {.flex_grow = 1});
    add(navigation_);
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextLabel detail_;
  TextLabel status_;
  material3::NavigationBarDestination home_;
  material3::NavigationBarDestination schedule_;
  material3::NavigationBarDestination settings_;
  PoolNavigationBar navigation_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
PoolController pool_controller(app.context());
UiTask& task = app.addUiTaskFullScreen(pool_controller);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
