// Learning goal: activate a standalone search entry from a dashboard. Tap the
// search surface or its Clear action and observe the visible state update.

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

class ActionIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

class EquipmentSearch : public FlexLayout {
 public:
  explicit EquipmentSearch(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Find equipment", material3::text_style_title_large()),
        search_(context),
        clear_(context, ic_outlined_24_navigation_close()),
        feedback_(context, "Try searching pumps, valves, or sensors",
                  material3::text_style_body_medium()) {
    setPadding(Padding(Scaled(16), Scaled(18)));
    setGap(Scaled(14));

    // SearchBar is an entry surface, not an editable text field. Its callback
    // should open your application's focused search experience.
    search_.setDisplayText("Search equipment");
    search_.setTrailing(0, clear_);
    search_.setOnInteractiveChange(
        [this]() { feedback_.setText("Focused search requested"); });

    // Interactive slots receive their own callback instead of activating the
    // surrounding search surface.
    clear_.setOnInteractiveChange([this]() {
      search_.setDisplayText("Search equipment");
      feedback_.setText("Search prompt restored");
    });

    add(title_);
    add(search_);
    add(feedback_);
  }

 private:
  TextLabel title_;
  material3::SearchBar search_;
  ActionIcon clear_;
  TextLabel feedback_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
EquipmentSearch equipment_search(app.context());
Task& task = app.addTaskFullScreen(equipment_search);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
