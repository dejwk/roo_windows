// Learning goal: separate settings sections and adjacent status panes with
// Material 3 dividers. Parent padding supplies the insets.

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

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/divider/divider.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

Ili9341spi<7, 2, 3> screen(Orientation().rotateLeft());
TouchXpt2046<1> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

class StatusPanel : public FlexLayout {
 public:
  explicit StatusPanel(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {
    // Dividers have no padding or margins. This common inset aligns separators
    // with the text, and the row's gap supplies space beside the vertical rule.
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    add(std::make_unique<TextLabel>(context, "Pool controller",
                                    material3::text_style_title_large()));
    add(std::make_unique<material3::HorizontalDivider>(context));
    auto panes = std::make_unique<FlexLayout>(context, FlexDirection::kRow);
    panes->setGap(Scaled(12));
    panes->add(std::make_unique<TextLabel>(context, "Water: 24 C",
                                           material3::text_style_body_large()));
    panes->add(std::make_unique<material3::VerticalDivider>(context));
    panes->add(std::make_unique<TextLabel>(context, "Pump: Auto",
                                           material3::text_style_body_large()));
    add(std::move(panes));
    add(std::make_unique<material3::HorizontalDivider>(context));
    add(std::make_unique<TextLabel>(context, "Next filter cycle: 18:00",
                                    material3::text_style_body_medium()));
  }
};

roo_scheduler::SchedulingService scheduler;
Environment environment(scheduler);
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif
StatusPanel panel(app.context());
Task& task = app.addTaskFullScreen(panel);

void setup() {
  SPI.begin(4, 5, 6);
  display.enableTurbo();
  display.init();
  app.start();
  scheduler.run();
}

void loop() {}
