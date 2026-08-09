// Learning goal: navigate between caller-owned destination roots without
// giving their ownership or lifetime to the windowing framework.

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

using namespace roo_display;

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
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_windows;

class LabelDestination : public Destination {
 public:
  LabelDestination(ApplicationContext& context, const char* text)
      : label_(context, text, material2::text_style_caption(),
               kGravityCenter | kGravityMiddle) {}

  Widget& getContents() override { return label_; }

 private:
  TextLabel label_;
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
NavigationHost navigation;
LabelDestination home(app.context(), "Pool controls");
LabelDestination details(app.context(), "Pump details");

void setup() {
  initDisplay();

  // The application owns the host and both destinations. The framework only
  // attaches the current destination's widget, so inactive roots stay detached.
  app.addTaskFullScreen(navigation);
  navigation.push(home);
  navigation.push(details);

  // A hardware Back key returns to the home destination. At the root, the
  // task's optional Back callback can perform an application-specific action.
  app.start();
  scheduler.run();
}

void loop() {}
