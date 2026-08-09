// Learning goal: use fixed secondary tabs as a view filter rather than page
// navigation. Tap Today, Week, or Month to update the same history view.

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
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Secondary tabs that update one history view instead of navigating pages.
class HistoryTabs : public material3::Tabs {
 public:
  /// Creates filter tabs that update the supplied range and summary labels.
  HistoryTabs(ApplicationContext& context, TextLabel& range, TextLabel& summary)
      : material3::Tabs(context, material3::TabsVariant::kSecondary),
        range_(range),
        summary_(summary) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    static constexpr const char* kRanges[] = {"Today", "Last 7 days",
                                              "Last 30 days"};
    static constexpr const char* kSummaries[] = {
        "25.1°C to 27.6°C", "24.4°C to 28.0°C", "21.8°C to 28.3°C"};
    range_.setText(kRanges[new_index]);
    summary_.setText(kSummaries[new_index]);
  }

 private:
  TextLabel& range_;
  TextLabel& summary_;
};

/// Temperature history screen with a fixed secondary-tab filter.
class TemperatureHistory : public FlexLayout {
 public:
  /// Creates the history view and its Today, Week, and Month filters.
  explicit TemperatureHistory(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Temperature history",
               material3::text_style_title_large()),
        range_(context, "Last 7 days", material3::text_style_title_medium()),
        summary_(context, "24.4°C to 28.0°C",
                 material3::text_style_headline_medium()),
        tabs_(context, range_, summary_),
        today_(context, "Today"),
        week_(context, "Week"),
        month_(context, "Month") {
    setPadding(Padding(Scaled(12), Scaled(14)));
    setGap(Scaled(10));

    // Secondary tabs refine the current view; they do not replace the screen
    // or participate in back navigation.
    tabs_.addTab(today_);
    tabs_.addTab(week_);
    tabs_.addTab(month_);
    tabs_.setSelectedIndex(1, false);

    add(title_);
    add(tabs_);
    add(range_);
    add(summary_);
  }

 private:
  TextLabel title_;
  TextLabel range_;
  TextLabel summary_;
  HistoryTabs tabs_;
  material3::Tab today_;
  material3::Tab week_;
  material3::Tab month_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
TemperatureHistory temperature_history(app.context());
UiTask& task = app.addUiTaskFullScreen(temperature_history);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
