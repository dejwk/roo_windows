// Learning goal: choose button variants by action emphasis. Tap a pool action
// and observe the command state shown below the controls.

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
#include "roo_icons/outlined/24/av.h"
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
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"

namespace {

enum class PoolAction : uint8_t { kHeat, kCirculate, kStop };

/// Button that translates a press into one pool-controller command message.
class PoolActionButton : public material3::Button {
 public:
  /// Creates a button with its command and visible feedback target.
  PoolActionButton(ApplicationContext& context, roo::string_view label,
                   material3::ButtonVariant variant, TextLabel& feedback,
                   PoolAction action)
      : material3::Button(context, label, variant),
        feedback_(feedback),
        action_(action) {}

  /// Publishes this button's fixed pool command as visible feedback.
  void onClicked() override {
    static constexpr const char* kMessages[] = {"Solar heating started",
                                                "Circulation cycle started",
                                                "All pool equipment stopped"};
    feedback_.setText(kMessages[static_cast<uint8_t>(action_)]);
  }

 private:
  TextLabel& feedback_;
  PoolAction action_;
};

/// Pool action screen that communicates command emphasis through variants.
class PoolActions : public FlexLayout {
 public:
  /// Creates primary, secondary, and protective pool actions.
  explicit PoolActions(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pool actions", material3::text_style_title_large()),
        guidance_(context, "Filled is primary; tonal and outlined step down",
                  material3::text_style_body_medium(), kTop | kLeft),
        feedback_(context, "Choose an action",
                  material3::text_style_title_small()),
        actions_(context, FlexDirection::kRow),
        heat_(context, "Heat", material3::ButtonVariant::kFilled, feedback_,
              PoolAction::kHeat),
        circulate_(context, "Circulate", material3::ButtonVariant::kFilledTonal,
                   feedback_, PoolAction::kCirculate),
        stop_(context, "Stop", material3::ButtonVariant::kOutlined, feedback_,
              PoolAction::kStop) {
    setPadding(Padding(Scaled(12), Scaled(16)));
    setGap(Scaled(12));
    actions_.setGap(Scaled(6));
    actions_.setAlignItems(AlignItems::kCenter);

    heat_.setIcon(&ic_outlined_24_action_eco());
    circulate_.setIcon(&ic_outlined_24_action_cached());
    stop_.setIcon(&ic_outlined_24_av_stop());
    heat_.setMargins(MarginSize::kNone);
    circulate_.setMargins(MarginSize::kNone);
    stop_.setMargins(MarginSize::kNone);

    actions_.add(heat_);
    actions_.add(circulate_);
    actions_.add(stop_);

    add(title_);
    add(guidance_);
    add(actions_);
    add(feedback_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  TextLabel feedback_;
  FlexLayout actions_;
  PoolActionButton heat_;
  PoolActionButton circulate_;
  PoolActionButton stop_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
PoolActions pool_actions(app.context());
SingletonActivity activity(app, pool_actions);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
