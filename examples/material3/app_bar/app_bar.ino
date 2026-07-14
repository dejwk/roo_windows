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

class ActionIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

class AppBarScreen : public ScrollablePanel {
 public:
  explicit AppBarScreen(ApplicationContext& context)
      : ScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        heading_(context, "Material 3 app bars", font_h6()),
        description_(context,
                     "Small, medium, and large title-based top app bars.",
                     font_caption()),
        divider_(context),
        small_(context, material3::AppBarVariant::kSmall),
        medium_(context, material3::AppBarVariant::kMediumFlexible),
        large_(context, material3::AppBarVariant::kLargeFlexible),
        small_leading_(context, ic_outlined_24_navigation_menu()),
        small_action_(context, ic_outlined_24_navigation_more_vert()),
        medium_leading_(context, ic_outlined_24_navigation_arrow_back()),
        medium_action_(context, ic_outlined_24_action_search()),
        large_leading_(context, ic_outlined_24_navigation_menu()),
        large_action_(context, ic_outlined_24_navigation_more_vert()) {
    // Top app bars own the screen edge. Keep the showcase's vertical rhythm
    // without adding an outer horizontal inset to each app-bar container.
    content_.setPadding(Padding(0, Scaled(10)));
    content_.setGap(Scaled(8));

    small_.setTitle("Inbox");
    small_.setLeading(small_leading_);
    small_.setTrailing(0, small_action_);

    medium_.setTitle("Recently viewed");
    medium_.setSubtitle("Medium flexible app bar");
    medium_.setLeading(medium_leading_);
    medium_.setTrailing(0, medium_action_);
    medium_.setSurfaceState(material3::AppBarSurfaceState::kScrolled);

    large_.setTitle("Your library");
    large_.setSubtitle("Large flexible app bar");
    large_.setLeading(large_leading_);
    large_.setTrailing(0, large_action_);

    content_.add(heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(description_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(small_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(medium_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(large_, {.flex_grow = 0, .flex_shrink = 0});
    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel heading_;
  TextLabel description_;
  HorizontalDivider divider_;
  material3::AppBar small_;
  material3::AppBar medium_;
  material3::AppBar large_;
  ActionIcon small_leading_;
  ActionIcon small_action_;
  ActionIcon medium_leading_;
  ActionIcon medium_action_;
  ActionIcon large_leading_;
  ActionIcon large_action_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
AppBarScreen app_bar_screen(app.context());
SingletonActivity activity(app, app_bar_screen);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
