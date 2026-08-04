// Learning goal: size controls for a constrained equipment toolbar. Tap a
// service action to see which compact command was requested.

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
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace {

enum class ServiceAction : uint8_t { kPrime, kBackwash, kReset };

/// Compact service button that reports its command through virtual dispatch.
class ServiceButton : public material3::Button {
 public:
  /// Creates a compact button with a fixed service command.
  ServiceButton(ApplicationContext& context, roo::string_view label,
                material3::ButtonVariant variant, TextLabel& feedback,
                ServiceAction action)
      : material3::Button(context, label, variant),
        feedback_(feedback),
        action_(action) {}

  /// Publishes this button's fixed service command as visible feedback.
  void onClicked() override {
    static constexpr const char* kMessages[] = {"Pump priming requested",
                                                "Filter backwash requested",
                                                "Alarm reset requested"};
    feedback_.setText(kMessages[static_cast<uint8_t>(action_)]);
  }

 private:
  TextLabel& feedback_;
  ServiceAction action_;
};

/// Compact icon-only service action for constrained toolbars.
class ServiceIconButton : public material3::IconButton {
 public:
  ServiceIconButton(ApplicationContext& context, const MonoIcon& icon,
                    TextLabel& feedback, ServiceAction action)
      : material3::IconButton(context, icon),
        feedback_(feedback),
        action_(action) {}

  void onClicked() override {
    static constexpr const char* kMessages[] = {"Pump priming requested",
                                                "Filter backwash requested",
                                                "Alarm reset requested"};
    feedback_.setText(kMessages[static_cast<uint8_t>(action_)]);
  }

 private:
  TextLabel& feedback_;
  ServiceAction action_;
};

/// Space-constrained maintenance toolbar using small button tokens.
class CompactControls : public FlexLayout {
 public:
  /// Creates a toolbar that contrasts extra-small and reduced-padding sizes.
  explicit CompactControls(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Service toolbar", material3::text_style_title_large()),
        guidance_(context, "Use compact sizes only where screen space is tight",
                  material3::text_style_body_medium(), kTop | kLeft),
        toolbar_(context, FlexDirection::kRow),
        feedback_(context, "No service command pending",
                  material3::text_style_title_small()),
        prime_(context, "Prime", material3::ButtonVariant::kFilledTonal,
               feedback_, ServiceAction::kPrime),
        backwash_(context, "Backwash", material3::ButtonVariant::kOutlined,
                  feedback_, ServiceAction::kBackwash),
        reset_(context, ic_outlined_24_action_done(), feedback_,
               ServiceAction::kReset) {
    setPadding(Padding(Scaled(12), Scaled(16)));
    setGap(Scaled(14));
    toolbar_.setGap(Scaled(6));
    toolbar_.setAlignItems(AlignItems::kCenter);

    prime_.setSize(material3::ButtonSize::kExtraSmall);
    backwash_.setSize(material3::ButtonSize::kSmall);
    backwash_.setSmallButtonPadding(material3::SmallButtonPadding::kReduced);
    reset_.setSize(material3::ButtonSize::kSmall);
    reset_.setStyle(material3::IconButtonStyle::kStandard);
    prime_.setIcon(&ic_outlined_24_action_cached());
    backwash_.setIcon(&ic_outlined_24_action_build());
    prime_.setMargins(MarginSize::kNone);
    backwash_.setMargins(MarginSize::kNone);

    toolbar_.add(prime_);
    toolbar_.add(backwash_);
    toolbar_.add(reset_);

    add(title_);
    add(guidance_);
    add(toolbar_);
    add(feedback_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  FlexLayout toolbar_;
  TextLabel feedback_;
  ServiceButton prime_;
  ServiceButton backwash_;
  ServiceIconButton reset_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
CompactControls compact_controls(app.context());
SingletonActivity activity(app, compact_controls);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
