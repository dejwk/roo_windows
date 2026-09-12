// Learning goal: show known progress and unknown-duration Material activity.
// Compare values, direction and motion, then open the dialog or status menu.

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

#include <stdio.h>

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
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/dialog/full_screen_dialog.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/progress_indicator/progress_indicator.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
FlexLayout catalog(app.context(), FlexDirection::kColumn);
FlexLayout value_controls(app.context(), FlexDirection::kRow);
FlexLayout appearance_controls(app.context(), FlexDirection::kRow);
FlexLayout presentation_controls(app.context(), FlexDirection::kRow);
material3::LinearProgressIndicator linear(app.context());
material3::CircularProgressIndicator circular(app.context());
material3::Button next(app.context(), "Next value");
material3::Button unknown(app.context(), "Unknown duration");
material3::Button direction(app.context(), "Mirror direction");

material3::Button motion(app.context(), "Toggle motion");

material3::Button details(app.context(), "Progress dialog");
material3::Button menu_button(app.context(), "Open menu");
material3::CircularProgressIndicator dialog_progress(app.context());
material3::FullScreenDialog progress_dialog(app.context(), dialog_progress);
material3::MenuGroup menu_group(app.context());
material3::Menu menu(app.context());
TextLabel status(app.context(), "Progress indicators",
                 material3::text_style_body_medium());

void setup() {
  initDisplay();
  catalog.setPadding(Padding(Scaled(8)));
  catalog.setGap(Scaled(2));
  catalog.add(status);
  catalog.add(linear);
  catalog.add(circular);
  value_controls.add(next);
  value_controls.add(unknown);
  appearance_controls.add(direction);
  appearance_controls.add(motion);
  presentation_controls.add(details);
  presentation_controls.add(menu_button);
  catalog.add(value_controls);
  catalog.add(appearance_controls);
  catalog.add(presentation_controls);
  menu_group.add(
      std::make_unique<material3::MenuRow<material3::StandardMenuItem>>(
          app.context(),
          material3::StandardMenuItemInit{"Operation status", {}}));
  menu.addGroup(menu_group);
  menu_button.setOnInteractiveChange(
      [] { menu.show(*catalog.getTask(), menu_button); });
  progress_dialog.setHeaderTitle("Working…");
  dialog_progress.setIndeterminate();
  details.setOnInteractiveChange(
      [] { progress_dialog.show(*catalog.getTask()); });
  motion.setOnInteractiveChange([] {
    bool enabled = !linear.motionEnabled();
    linear.setMotionEnabled(enabled);
    circular.setMotionEnabled(enabled);
  });
  next.setOnInteractiveChange([] {
    status.setText("Known progress");
    float value = linear.progress() >= 1 ? 0 : linear.progress() + 0.25f;
    linear.setProgress(value);
    circular.setProgress(value);
  });
  unknown.setOnInteractiveChange([] {
    status.setText("Working…");
    linear.setIndeterminate();
    circular.setIndeterminate();
  });
  direction.setOnInteractiveChange([] {
    linear.setLayoutDirection(linear.layoutDirection() ==
                                      LayoutDirection::kLeftToRight
                                  ? LayoutDirection::kRightToLeft
                                  : LayoutDirection::kLeftToRight);
  });
  app.addTaskFullScreen(catalog);
  app.start();
  scheduler.run();
}
void loop() {}
