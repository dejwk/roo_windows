// Learning goal: use a segmented Material 3 list to group related schedule
// entries while preserving clear first, middle, and last row shapes.

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
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"
#include "../list_example_layout.h"

namespace {

/// Daily circulation schedule presented as one visual group.
class CirculationSchedule : public SimpleScrollablePanel {
 public:
  /// Creates three related schedule entries in a segmented list.
  explicit CirculationSchedule(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context),
        title_(context, "Circulation schedule",
               material3::text_style_title_large()),
        guidance_(context, "Today's automatic equipment cycles",
                  material3::text_style_body_medium(), kTop | kLeft),
        schedule_(context),
        morning_(context, material3::StandardListItemInit::TwoLine(
                              "06:30  Pool pump", "Weekdays · 90 minutes")),
        solar_(context, material3::StandardListItemInit::TwoLine(
                            "09:15  Solar assist", "When roof water is warm")),
        evening_(context, material3::StandardListItemInit::TwoLine(
                              "18:45  Spa filter", "Daily · 45 minutes")) {
    content_.setPadding(Padding(Scaled(16), Scaled(12)));
    content_.setGap(Scaled(10));

    // Segmented style supplies the grouped gaps and position-aware shapes.
    schedule_.setStyle(material3::ListStyle::kSegmented);
    material3::ListEntryVisualContext active;
    active.selected = true;
    solar_.setVisualContext(active);

    schedule_.add(morning_);
    schedule_.add(solar_);
    schedule_.add(evening_);

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(guidance_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(schedule_, {.flex_grow = 0, .flex_shrink = 0});
    setContents(content_);
  }

 private:
  material3_examples::FullWidthColumn content_;
  TextLabel title_;
  TextBlock guidance_;
  material3::List schedule_;
  material3::ListRow<material3::StandardListItem> morning_;
  material3::ListRow<material3::StandardListItem> solar_;
  material3::ListRow<material3::StandardListItem> evening_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
CirculationSchedule circulation_schedule(app.context());
SingletonActivity activity(app, circulation_schedule);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
