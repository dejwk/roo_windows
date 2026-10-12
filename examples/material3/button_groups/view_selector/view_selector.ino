// Learning goal: select a schedule view with one connected-group callback.
// Tap Day/Week/Month, or use Tab, arrows and Enter/Space on a physical
// keyboard.

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
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
namespace {
class ScheduleView : public FlexLayout {
 public:
  explicit ScheduleView(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Schedule", material3::text_style_title_large()),
        day_(context, "Day"),
        week_(context, "Week"),
        month_(context, "Month"),
        period_(context),
        summary_(context, "This week's device runs",
                 material3::text_style_body_large()) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    // Entries and label literals outlive the group. Attachment indices remain
    // stable when direction or visibility changes.
    period_.add(day_);
    period_.add(week_);
    period_.add(month_);
    period_.setSelectionMask(1u << 1);
    // Only interactive selection changes invoke this callback. Choosing the
    // already-selected required option does nothing. Replace these descriptions
    // with your device's schedule data or navigation action.
    period_.setOnInteractiveChange([this] { showPeriod(); });
    add(title_);
    add(period_);
    add(summary_);
  }

 private:
  void showPeriod() {
    static const char* kDescriptions[] = {"Today's device runs",
                                          "This week's device runs",
                                          "This month's device runs"};
    summary_.setText(kDescriptions[period_.selectedIndex()]);
  }
  TextLabel title_;
  material3::ButtonGroupButton day_;
  material3::ButtonGroupButton week_;
  material3::ButtonGroupButton month_;
  material3::ConnectedButtonGroup period_;
  TextLabel summary_;
};
}  // namespace
roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
ScheduleView schedule_view(app.context());
Task& task = app.addTaskFullScreen(schedule_view);
void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
