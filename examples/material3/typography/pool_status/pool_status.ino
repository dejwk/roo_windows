// Learning goal: apply the Material 3 type scale as a semantic hierarchy on a
// real status screen instead of presenting typography as a token catalog.

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
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Pool overview whose text styles express information importance.
class PoolStatus : public FlexLayout {
 public:
  /// Creates a status hierarchy from screen title through supporting metadata.
  explicit PoolStatus(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pool status", material3::text_style_title_large()),
        updated_(context, "UPDATED 14:32",
                 material3::text_style_label_medium()),
        temperature_(context, "27.4 °C", material3::text_style_display_small()),
        temperature_label_(context, "Water temperature",
                           material3::text_style_body_medium()),
        pump_(context, "Circulation pump on",
              material3::text_style_headline_small()),
        pump_detail_(context, "1,850 rpm · 96 L/min",
                     material3::text_style_body_large()),
        next_heading_(context, "Next automatic action",
                      material3::text_style_title_small()),
        next_detail_(context, "Solar assist at roof temperature 31.4 °C",
                     material3::text_style_body_medium()) {
    setPadding(Padding(Scaled(16), Scaled(8)));
    setGap(Scaled(2));

    // Large roles carry the glanceable value and current operating state.
    // Title, body, and label roles provide structure without competing with it.
    add(title_);
    add(updated_);
    add(temperature_);
    add(temperature_label_);
    add(pump_);
    add(pump_detail_);
    add(next_heading_);
    add(next_detail_);
  }

 private:
  TextLabel title_;
  TextLabel updated_;
  TextLabel temperature_;
  TextLabel temperature_label_;
  TextLabel pump_;
  TextLabel pump_detail_;
  TextLabel next_heading_;
  TextLabel next_detail_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
PoolStatus pool_status(app.context());
UiTask& task = app.addUiTaskFullScreen(pool_status);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
