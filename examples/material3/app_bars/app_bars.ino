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

#include <stdio.h>

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
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class AppBarsScreen : public ScrollablePanel {
 public:
  explicit AppBarsScreen(ApplicationContext& context)
      : ScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 app bars", font_h6()),
        subtitle_(context,
                  "Title, standalone, and top-edge search-entry surfaces.",
                  font_caption()),
        divider_(context),
        app_bar_heading_(context, "Title-based top app bar", font_subtitle2()),
        title_app_bar_(context, material3::AppBarVariant::kSmall),
        search_app_bar_heading_(context, "Search app bar", font_subtitle2()),
        search_app_bar_(context),
        passive_heading_(context, "Default passive leading search icon",
                         font_subtitle2()),
        passive_search_(context),
        custom_heading_(context, "Custom leading and trailing widget slots",
                        font_subtitle2()),
        custom_search_(context),
        leading_(context, ic_outlined_24_action_done()),
        first_trailing_(context, ic_outlined_24_navigation_close()),
        second_trailing_(context, ic_outlined_24_navigation_more_vert()),
        app_bar_leading_(context, ic_outlined_24_navigation_menu()),
        app_bar_trailing_(context, ic_outlined_24_navigation_more_vert()),
        search_app_bar_inner_(context, ic_outlined_24_action_search()),
        search_app_bar_outer_(context, ic_outlined_24_navigation_more_vert()),
        feedback_(context, "Tap a search surface to simulate opening search.",
                  font_caption()),
        activations_(0) {
    content_.setPadding(Padding(Scaled(12), Scaled(10)));
    content_.setGap(Scaled(8));

    passive_search_.setDisplayText("Search messages");
    custom_search_.setDisplayText("Search with custom slots");
    custom_search_.setLeading(leading_);
    custom_search_.setTrailing(0, first_trailing_);
    custom_search_.setTrailing(1, second_trailing_);
    title_app_bar_.setTitle("Inbox");
    title_app_bar_.setLeading(app_bar_leading_);
    title_app_bar_.setTrailing(0, app_bar_trailing_);
    search_app_bar_.setDisplayText("Search messages");
    search_app_bar_.setInnerTrailing(0, search_app_bar_inner_);
    search_app_bar_.setTrailing(0, search_app_bar_outer_);
    passive_search_.setOnInteractiveChange([this]() { showActivation(); });
    custom_search_.setOnInteractiveChange([this]() { showActivation(); });
    search_app_bar_.setOnInteractiveChange([this]() { showActivation(); });

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(app_bar_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(title_app_bar_, {.flex_grow = 0, .flex_shrink = 1});
    content_.add(search_app_bar_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(search_app_bar_, {.flex_grow = 0, .flex_shrink = 1});
    content_.add(passive_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(passive_search_, {.flex_grow = 0, .flex_shrink = 1});
    content_.add(custom_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(custom_search_, {.flex_grow = 0, .flex_shrink = 1});
    content_.add(feedback_, {.flex_grow = 0, .flex_shrink = 0});
    setContents(content_);
  }

 private:
  void showActivation() {
    ++activations_;
    snprintf(message_, sizeof(message_), "Search entry activated %u time%s.",
             activations_, activations_ == 1 ? "" : "s");
    feedback_.setText(message_);
  }

  FlexLayout content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider divider_;
  TextLabel app_bar_heading_;
  material3::AppBar title_app_bar_;
  TextLabel search_app_bar_heading_;
  material3::SearchAppBar search_app_bar_;
  TextLabel passive_heading_;
  material3::SearchBar passive_search_;
  TextLabel custom_heading_;
  material3::SearchBar custom_search_;
  Icon leading_;
  Icon first_trailing_;
  Icon second_trailing_;
  Icon app_bar_leading_;
  Icon app_bar_trailing_;
  Icon search_app_bar_inner_;
  Icon search_app_bar_outer_;
  TextLabel feedback_;
  char message_[48];
  unsigned int activations_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
AppBarsScreen app_bars_screen(app.context());
SingletonActivity activity(app, app_bars_screen);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
