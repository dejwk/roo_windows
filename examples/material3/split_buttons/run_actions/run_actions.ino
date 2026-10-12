// Learning goal: offer one default device run command with related
// alternatives. Tap Run for the default cycle or tap its chevron for Run once
// and Configure run.

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
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Adjust these pins and calibration values for the physical display board.
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
#include "roo_windows/material3/button/split_button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

// The menu keeps its rows and item labels between presentations. A real device
// would dispatch the command in onInvoked() and report its current state here.
class RunActionItem : public material3::StandardMenuItem {
 public:
  RunActionItem(TextLabel& feedback, const char* name, const char* result)
      : StandardMenuItem(material3::StandardMenuItemInit{name}),
        feedback_(feedback),
        result_(result) {}

  void onInvoked() override { feedback_.setText(result_); }

 private:
  TextLabel& feedback_;
  const char* result_;
};

class RunActions : public FlexLayout {
 public:
  explicit RunActions(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Device run", material3::text_style_title_large()),
        feedback_(context, "Ready", material3::text_style_title_small()),
        once_(context, feedback_, "Run once", "Single run started"),
        configure_(context, feedback_, "Configure run", "Run settings opened"),
        group_(context),
        menu_(context),
        run_(context, menu_, "Run") {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    group_.add(once_);
    group_.add(configure_);
    menu_.addGroup(group_);

    // The primary callback is independent of menu item invocation. The menu,
    // rows, and borrowed label literals all outlive the split button.
    run_.setOnInteractiveChange(
        [this] { feedback_.setText("Default run started"); });
    add(title_);
    add(run_);
    add(feedback_);
  }

 private:
  TextLabel title_;
  TextLabel feedback_;
  material3::MenuRow<RunActionItem> once_;
  material3::MenuRow<RunActionItem> configure_;
  material3::MenuGroup group_;
  material3::Menu menu_;
  material3::SplitButton run_;
};

}  // namespace

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
RunActions run_actions(app.context());
Task& task = app.addTaskFullScreen(run_actions);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
