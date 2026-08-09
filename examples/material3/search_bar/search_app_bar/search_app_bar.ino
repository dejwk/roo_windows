// Learning goal: place search at the top edge and distinguish the embedded
// search action from outer app-bar navigation and menu actions.

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
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class ActionIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

class LogSearch : public FlexLayout {
 public:
  explicit LogSearch(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        search_app_bar_(context),
        back_(context, ic_outlined_24_navigation_arrow_back()),
        scope_(context, ic_outlined_24_action_search()),
        more_(context, ic_outlined_24_navigation_more_vert()),
        content_(context, FlexDirection::kColumn),
        title_(context, "Recent controller events",
               material3::text_style_title_medium()),
        feedback_(context, "Tap Search events to begin",
                  material3::text_style_body_medium()) {
    search_app_bar_.setDisplayText("Search events");
    // Leading and trailing slots live on the outer app-bar surface. Inner
    // trailing slots belong to the search entry and refine the query.
    search_app_bar_.setLeading(back_);
    search_app_bar_.setInnerTrailing(0, scope_);
    search_app_bar_.setTrailing(0, more_);

    search_app_bar_.setOnInteractiveChange(
        [this]() { feedback_.setText("Event search opened"); });
    back_.setOnInteractiveChange(
        [this]() { feedback_.setText("Back to diagnostics"); });
    scope_.setOnInteractiveChange(
        [this]() { feedback_.setText("Search limited to warnings"); });
    more_.setOnInteractiveChange(
        [this]() { feedback_.setText("Log actions requested"); });

    content_.setPadding(Padding(Scaled(16), Scaled(18)));
    content_.setGap(Scaled(10));
    content_.add(title_);
    content_.add(feedback_);

    // SearchAppBar owns the top edge; only ordinary content receives padding.
    add(search_app_bar_);
    add(content_, {.flex_grow = 1});
  }

 private:
  material3::SearchAppBar search_app_bar_;
  ActionIcon back_;
  ActionIcon scope_;
  ActionIcon more_;
  FlexLayout content_;
  TextLabel title_;
  TextLabel feedback_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
LogSearch log_search(app.context());
Task& task = app.addTaskFullScreen(log_search);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
