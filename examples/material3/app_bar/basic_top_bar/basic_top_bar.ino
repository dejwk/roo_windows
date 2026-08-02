// Learning goal: add a compact top app bar whose navigation and action slots
// update visible application state. Tap Menu or Refresh to exercise the flow.

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

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
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
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace {

// Icons are passive by default. App-bar affordances opt in to click handling.
class ActionIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

class PoolOverview : public FlexLayout {
 public:
  explicit PoolOverview(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        app_bar_(context),
        menu_(context, ic_outlined_24_navigation_menu()),
        refresh_(context, ic_outlined_24_navigation_refresh()),
        content_(context, FlexDirection::kColumn),
        temperature_(context, "Water 27.4 °C",
                     material3::text_style_headline_medium()),
        equipment_(context, "Pump running · Solar heating ready",
                   material3::text_style_body_large()),
        feedback_(context, "Last updated just now",
                  material3::text_style_body_medium()) {
    // A small app bar is the recommended choice when content, rather than the
    // screen title, should receive most of the vertical space.
    app_bar_.setTitle("Pool overview");
    app_bar_.setLeading(menu_);
    app_bar_.setTrailing(0, refresh_);

    menu_.setOnInteractiveChange(
        [this]() { feedback_.setText("Navigation menu requested"); });
    refresh_.setOnInteractiveChange(
        [this]() { feedback_.setText("Sensor readings refreshed"); });

    content_.setPadding(Padding(Scaled(16), Scaled(18)));
    content_.setGap(Scaled(10));
    content_.add(temperature_);
    content_.add(equipment_);
    content_.add(feedback_);

    // Keep the bar flush with the screen edge; inset only the screen body.
    add(app_bar_);
    add(content_, {.flex_grow = 1});
  }

 private:
  material3::AppBar app_bar_;
  ActionIcon menu_;
  ActionIcon refresh_;
  FlexLayout content_;
  TextLabel temperature_;
  TextLabel equipment_;
  TextLabel feedback_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
PoolOverview pool_overview(app.context());
SingletonActivity activity(app, pool_overview);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
