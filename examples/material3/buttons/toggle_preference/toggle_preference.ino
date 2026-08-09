// Learning goal: compare Material 3 icon-button types, then use the toggle
// type for a persistent pool-equipment preference.

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
#include "roo_icons/outlined/24/action.h"
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
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// A compact comparison of momentary icon actions and one saved preference.
class TogglePreference : public FlexLayout {
 public:
  /// Creates the icon-button comparison and preference state feedback.
  explicit TogglePreference(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pool maintenance",
               material3::text_style_title_large()),
        guidance_(
            context,
            "Choose a container type by action importance; save reminders.",
            material3::text_style_body_medium(), kTop | kLeft),
        actions_(context, FlexDirection::kRow),
        standard_(context, ic_outlined_24_action_build(),
                  material3::IconButtonStyle::kStandard),
        filled_(context, ic_outlined_24_action_done(),
                material3::IconButtonStyle::kFilled),
        tonal_(context, ic_outlined_24_action_cached(),
               material3::IconButtonStyle::kFilledTonal),
        outlined_(context, ic_outlined_24_action_delete(),
                  material3::IconButtonStyle::kOutlined),
        reminder_(context, ic_outlined_24_action_bookmark(),
                  &ic_outlined_24_action_favorite(),
                  material3::IconButtonStyle::kOutlined),
        feedback_(context, "Reminder preference is off",
                  material3::text_style_title_small()) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(16));
    actions_.setGap(Scaled(8));
    standard_.setSize(material3::ButtonSize::kSmall);
    filled_.setSize(material3::ButtonSize::kSmall);
    tonal_.setSize(material3::ButtonSize::kSmall);
    outlined_.setSize(material3::ButtonSize::kSmall);
    reminder_.setSize(material3::ButtonSize::kMedium);

    standard_.setOnInteractiveChange(
        [this]() { feedback_.setText("Standard action requested"); });
    filled_.setOnInteractiveChange(
        [this]() { feedback_.setText("Primary action requested"); });
    tonal_.setOnInteractiveChange(
        [this]() { feedback_.setText("Routine action requested"); });
    outlined_.setOnInteractiveChange(
        [this]() { feedback_.setText("Secondary action requested"); });

    // ToggleIconButton commits its state before this callback, so application
    // code can use isSelected() as the single source of truth.
    reminder_.setOnInteractiveChange([this]() {
      feedback_.setText(reminder_.isSelected() ? "Reminder preference is saved"
                                               : "Reminder preference is off");
    });

    actions_.add(standard_);
    actions_.add(filled_);
    actions_.add(tonal_);
    actions_.add(outlined_);

    add(title_);
    add(guidance_);
    add(actions_);
    add(reminder_);
    add(feedback_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  FlexLayout actions_;
  material3::IconButton standard_;
  material3::IconButton filled_;
  material3::IconButton tonal_;
  material3::IconButton outlined_;
  material3::ToggleIconButton reminder_;
  TextLabel feedback_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
TogglePreference toggle_preference(app.context());
Task& task = app.addTaskFullScreen(toggle_preference);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
