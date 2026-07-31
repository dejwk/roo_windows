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
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class HeaderIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

class ExampleRail : public material3::NavigationRail {
 public:
  ExampleRail(ApplicationContext& context, TextLabel& status)
      : material3::NavigationRail(context), status_(status) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    static constexpr const char* kMessages[] = {
        "Home selected", "Inbox selected", "Saved selected"};
    status_.setText(kMessages[new_index]);
  }

 private:
  TextLabel& status_;
};

class NavigationRailScreen : public FlexLayout {
 public:
  explicit NavigationRailScreen(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kRow),
        status_(context, "Home selected", material2::text_style_h6()),
        header_(context, ic_outlined_24_navigation_menu()),
        home_(context, "Home", &ic_outlined_24_navigation_home_work()),
        inbox_(context, "Inbox", &ic_outlined_24_action_bookmark()),
        saved_(context, "Saved", &ic_outlined_24_action_done()),
        rail_(context, status_) {
    setPadding(Padding(Scaled(8)));
    setGap(Scaled(12));
    setAlignItems(AlignItems::kCenter);

    rail_.setHeader(WidgetRef(header_));
    rail_.add(WidgetRef(home_));
    rail_.add(WidgetRef(inbox_));
    rail_.add(WidgetRef(saved_));
    rail_.setSelectedIndex(1);
    inbox_.setBadgeValue(12);

    add(rail_, {.flex_grow = 0, .flex_shrink = 0});
    add(status_, {.flex_grow = 1, .flex_shrink = 1});
  }

 private:
  TextLabel status_;
  HeaderIcon header_;
  material3::NavigationRailDestination home_;
  material3::BadgedNavigationRailDestination inbox_;
  material3::NavigationRailDestination saved_;
  ExampleRail rail_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
NavigationRailScreen navigation_rail_screen(app.context());
SingletonActivity activity(app, navigation_rail_screen);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
