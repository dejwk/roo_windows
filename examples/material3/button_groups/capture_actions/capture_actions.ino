// Learning goal: offer related capture actions with independent callbacks in
// a standard group. Press expansion preserves the strip width and fixed gaps.

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
#include "roo_icons/outlined/action.h"
#include "roo_icons/outlined/av.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
namespace {
class CaptureActions : public FlexLayout {
 public:
  explicit CaptureActions(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Field capture", material3::text_style_title_large()),
        capture_(context, "Capture"),
        save_(context, "Save", material3::ButtonGroupButtonStyle::kFilledTonal),
        review_(context, SCALED_ROO_ICON(outlined, action_history),
                material3::ButtonGroupIconStyle::kOutlined),
        actions_(context),
        feedback_(context, "Ready for a sample",
                  material3::text_style_body_large()) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    capture_.setIcon(&SCALED_ROO_ICON(outlined, av_fiber_manual_record));
    // Entries are borrowed and constructed before their group. Each action
    // keeps its own callback; the group only coordinates geometry and feedback.
    actions_.add(capture_);
    actions_.add(save_);
    actions_.add(review_);
    capture_.setOnInteractiveChange(
        [this] { feedback_.setText("Sample captured"); });
    save_.setOnInteractiveChange(
        [this] { feedback_.setText("Sample saved to storage"); });
    review_.setOnInteractiveChange(
        [this] { feedback_.setText("Capture history opened"); });
    // Replace these callbacks with your sensor and storage commands. The shared
    // controller delivers each action once after its visual click settles.
    add(title_);
    add(actions_);
    add(feedback_);
  }

 private:
  TextLabel title_;
  material3::ButtonGroupButton capture_;
  material3::ButtonGroupButton save_;
  material3::ButtonGroupIconButton review_;
  material3::ButtonGroup actions_;
  TextLabel feedback_;
};
}  // namespace
roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
CaptureActions capture_actions(app.context());
Task& task = app.addTaskFullScreen(capture_actions);
void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
