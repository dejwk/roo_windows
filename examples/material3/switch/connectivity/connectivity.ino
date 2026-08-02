// Learning goal: model immediate boolean settings with switches. Toggle each
// connection and observe the resulting device state.

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
#include "roo_icons/outlined/24/navigation.h"
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
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Switch that reports its state after the base thumb transition begins.
class ConnectionSwitch : public material3::Switch {
 public:
  /// Creates a switch for the named connection.
  ConnectionSwitch(ApplicationContext& context, OnOffState state,
                   roo::string_view connection, TextLabel& feedback)
      : material3::Switch(context, state),
        connection_(connection),
        feedback_(feedback) {}

  /// Lets the base widget toggle, then publishes the new connection state.
  void onSingleTapUp(XDim x, YDim y) override {
    material3::Switch::onSingleTapUp(x, y);
    feedback_.setTextf("%.*s %s", static_cast<int>(connection_.size()),
                       connection_.data(), isOn() ? "connected" : "offline");
  }

 private:
  roo::string_view connection_;
  TextLabel& feedback_;
};

/// One named connection with a trailing boolean switch.
class ConnectionRow : public FlexLayout {
 public:
  /// Creates a connection row with its initial state.
  ConnectionRow(ApplicationContext& context, roo::string_view label,
                material3::Switch::OnOffState state, TextLabel& feedback)
      : FlexLayout(context, FlexDirection::kRow),
        label_(context, std::string(label), material3::text_style_body_large()),
        toggle_(context, state, label, feedback) {
    setAlignItems(AlignItems::kCenter);
    setGap(Scaled(12));
    toggle_.setSelectedIcon(&ic_outlined_24_action_done());
    toggle_.setUnselectedIcon(&ic_outlined_24_navigation_close());
    add(label_, {.flex_grow = 1, .flex_shrink = 1});
    add(toggle_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel label_;
  ConnectionSwitch toggle_;
};

/// Connectivity screen with independent boolean device settings.
class Connectivity : public FlexLayout {
 public:
  /// Creates local radio and cloud connection switches.
  explicit Connectivity(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Connectivity", material3::text_style_title_large()),
        guidance_(context, "Switch changes apply immediately",
                  material3::text_style_body_medium()),
        feedback_(context, "Wi-Fi connected · Cloud sync offline",
                  material3::text_style_title_small()),
        wifi_(context, "Wi-Fi", material3::Switch::OnOffState::kOn, feedback_),
        cloud_(context, "Cloud sync", material3::Switch::OnOffState::kOff,
               feedback_) {
    setPadding(Padding(Scaled(16), Scaled(16)));
    setGap(Scaled(12));
    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(guidance_, {.flex_grow = 0, .flex_shrink = 0});
    add(wifi_, {.flex_grow = 0, .flex_shrink = 0});
    add(cloud_, {.flex_grow = 0, .flex_shrink = 0});
    add(feedback_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel guidance_;
  TextLabel feedback_;
  ConnectionRow wifi_;
  ConnectionRow cloud_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
Connectivity connectivity(app.context());
SingletonActivity activity(app, connectivity);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
