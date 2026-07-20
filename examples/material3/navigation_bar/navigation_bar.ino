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
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

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
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class NavigationBarScreen : public ScrollablePanel {
 public:
  explicit NavigationBarScreen(ApplicationContext& context)
      : ScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        heading_(context, "Material 3 navigation bar", font_h6()),
        description_(context,
                     "Tap a destination or use Left and Right with the bar "
                     "focused.",
                     font_caption()),
        divider_(context),
        compact_heading_(context, "Compact vertical destinations",
                         font_body1()),
        medium_heading_(context, "Medium horizontal destinations",
                        font_body1()),
        compact_home_(context, "Home", &ic_outlined_24_navigation_home_work()),
        compact_inbox_(context, "Inbox", &ic_outlined_24_action_bookmark()),
        compact_saved_(context, "Saved", &ic_outlined_24_action_done()),
        compact_bar_(context),
        medium_home_(context, "Home", &ic_outlined_24_navigation_home_work()),
        medium_inbox_(context, "Inbox", &ic_outlined_24_action_bookmark()),
        medium_saved_(context, "Saved", &ic_outlined_24_action_done()),
        medium_bar_(context) {
    content_.setPadding(Padding(0, Scaled(10)));
    content_.setGap(Scaled(8));

    compact_bar_.add(WidgetRef(compact_home_));
    compact_bar_.add(WidgetRef(compact_inbox_));
    compact_bar_.add(WidgetRef(compact_saved_));
    compact_bar_.setSelectedIndex(1);

    medium_bar_.setLayout(material3::NavigationBarLayout::kHorizontal);
    medium_bar_.add(WidgetRef(medium_home_));
    medium_bar_.add(WidgetRef(medium_inbox_));
    medium_bar_.add(WidgetRef(medium_saved_));
    medium_bar_.setSelectedIndex(1);

    content_.add(heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(description_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(compact_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(compact_bar_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(medium_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(medium_bar_, {.flex_grow = 0, .flex_shrink = 0});
    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel heading_;
  TextLabel description_;
  HorizontalDivider divider_;
  TextLabel compact_heading_;
  TextLabel medium_heading_;
  material3::NavigationBarDestination compact_home_;
  material3::NavigationBarDestination compact_inbox_;
  material3::NavigationBarDestination compact_saved_;
  material3::NavigationBar compact_bar_;
  material3::NavigationBarDestination medium_home_;
  material3::NavigationBarDestination medium_inbox_;
  material3::NavigationBarDestination medium_saved_;
  material3::NavigationBar medium_bar_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
NavigationBarScreen navigation_bar_screen(app.context());
SingletonActivity activity(app, navigation_bar_screen);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
