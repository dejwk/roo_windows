// Learning goal: run two independent display applications on one scheduler.
// The emulator and hardware paths both run the applications on separate
// displays.

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
  FltkViewport first_viewport;
  FltkViewport second_viewport;
  FlexViewport first_flex;
  FlexViewport second_flex;
  FakeIli9341Spi first_display;
  FakeIli9341Spi second_display;
  FakeXpt2046Spi first_touch;
  FakeXpt2046Spi second_touch;

  Emulator()
      : first_viewport(),
        second_viewport(),
        first_flex(first_viewport, 1, FlexViewport::kRotationRight),
        second_flex(second_viewport, 1, FlexViewport::kRotationRight),
        first_display(first_flex),
        second_display(second_flex),
        first_touch(first_flex, FakeXpt2046Spi::Calibration(
                                    269, 249, 3829, 3684, true, false, false)),
        second_touch(second_flex, FakeXpt2046Spi::Calibration(
                                      269, 249, 3829, 3684, true, false, false)) {
    FakeEsp32().attachSpiDevice(first_display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, first_display.cs());
    FakeEsp32().gpio.attachOutput(2, first_display.dc());
    FakeEsp32().gpio.attachOutput(3, first_display.rst());
    FakeEsp32().attachSpiDevice(first_touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, first_touch.cs());

    FakeEsp32().attachSpiDevice(second_display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(8, second_display.cs());
    FakeEsp32().gpio.attachOutput(10, second_display.dc());
    FakeEsp32().gpio.attachOutput(11, second_display.rst());
    FakeEsp32().attachSpiDevice(second_touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(9, second_touch.cs());
  }
} emulator;

roo_windows::fake::FltkKeySource emulator_keys;

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

using namespace roo_display;

static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;

Ili9341spi<7, 2, 3> first_screen(Orientation().rotateLeft());
TouchXpt2046<1> first_touch;
Display first_display(first_screen, first_touch,
                      TouchCalibration(269, 249, 3829, 3684,
                                       Orientation::LeftDown()));

Ili9341spi<8, 10, 11> second_screen(Orientation().rotateLeft());
TouchXpt2046<9> second_touch;
Display second_display(second_screen, second_touch,
                       TouchCalibration(269, 249, 3829, 3684,
                                        Orientation::LeftDown()));

void initDisplays() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  first_display.init();
  second_display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_field.h"

using namespace roo_windows;

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);

class EditorDestination : public Destination {
 public:
  explicit EditorDestination(ApplicationContext& context)
      : contents(context),
        field(context, font_body1(), "Type here",
              roo_display::kLeft | roo_display::kMiddle,
              TextField::UNDERLINE) {
    contents.add(field, roo_display::kLeft | roo_display::kTop);
  }

  Widget& getContents() override { return contents; }

  AlignedLayout contents;
  TextField field;
};

#ifdef ROO_TESTING
Application first_app(&env, first_display);
Application second_app(&env, second_display, emulator_keys, true);
#else
Application first_app(&env, first_display);
Application second_app(&env, second_display);
#endif
TextLabel first_label(first_app.context(), "Keyboard application",
                      material2::text_style_caption(),
                      kGravityCenter | kGravityMiddle);
NavigationHost editor_navigation;
EditorDestination editor_destination(second_app.context());

void setup() {
  initDisplays();

  first_app.addTaskFullScreen(first_label);
  Task& editor_task = second_app.addTaskFullScreen(editor_navigation);
  editor_navigation.push(editor_destination);
  // Keep the field focused, but leave keyboard presentation entirely to the
  // first application.
  editor_task.textFieldEditor().edit(&editor_destination.field, false);

  // Keyboard presentation stays with the first application, while its
  // semantic input synchronously edits the active field in the second.
  first_app.keyboard().connect(second_app);
  first_app.keyboard().show();

  // Each application schedules only its own work. Enter the shared scheduler
  // once, after every application has started.
  first_app.start();
  second_app.start();
  scheduler.run();
}

void loop() {}
