// Learning goal: show an explicitly Material 2 confirmation with automatic
// dialog lifetime management and handle the selected action.

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
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/dialogs/alert_dialog.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

Ili9341spi<7, 2, 3> screen(Orientation().rotateLeft());
TouchXpt2046<1> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

class ConfirmationPane : public FlexLayout {
 public:
  explicit ConfirmationPane(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        status_(context, "Saved schedule ready", material2::text_style_body1()),
        apply_(context, "Apply settings") {
    setPadding(Padding(Scaled(20)));
    setGap(Scaled(16));
    add(status_);
    add(apply_);
    apply_.setOnInteractiveChange([this]() {
      Task* owner = getTask();
      if (owner == nullptr) return;
      // The helper uses this task's application and owns the alert until its
      // callback returns. Busy admission leaves the existing dialog intact.
      material2::ShowAlertDialog(*owner, "Apply settings?",
                                 "The controller will use the new schedule.",
                                 {"Cancel", "Apply"}, [owner](int action) {
                                   if (action != 1) return;
                                   // Completion runs after detachment, so it
                                   // may open another alert. This callback
                                   // borrows only the interaction task, not
                                   // this pane.
                                   material2::ShowAlertDialog(
                                       *owner, "Settings applied",
                                       "The new schedule is active.", {"OK"},
                                       nullptr);
                                 });
    });
  }

  ~ConfirmationPane() override { removeAll(); }

 private:
  TextLabel status_;
  material2::SimpleButton apply_;
};

roo_scheduler::SchedulingService scheduler;
Environment environment(scheduler);
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif
ConfirmationPane pane(app.context());
Task& task = app.addTaskFullScreen(pane);

void setup() {
  SPI.begin(4, 5, 6);
  display.enableTurbo();
  display.init();
  app.start();
  scheduler.run();
}

void loop() {}
