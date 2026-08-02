// Learning goal: compose a readable settings list from standard Material 3
// rows, including wrapped supporting text and a trailing control.

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
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

// Creates a two-line row whose supporting text can wrap on compact displays.
material3::StandardListItemInit WrappedSetting(roo::string_view headline,
                                               roo::string_view supporting) {
  material3::StandardListItemInit init =
      material3::StandardListItemInit::TwoLine(headline, supporting);
  init.supporting_policy.overflow = material3::TextOverflowPolicy::kWrap;
  init.supporting_policy.max_lines = 2;
  return init;
}

/// Pool settings screen built from standard rows and inset dividers.
class PoolSettings : public FlexLayout {
 public:
  /// Creates settings with one-line, wrapped, and trailing-control rows.
  explicit PoolSettings(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pool settings", material3::text_style_title_large()),
        guidance_(context, "Review automation and safety defaults",
                  material3::text_style_body_medium()),
        list_(context),
        safety_lock_(context, material3::Switch::OnOffState::kOn),
        pump_mode_(context, material3::StandardListItemInit::TwoLine(
                                "Pool pump", "Automatic")),
        solar_delta_(context,
                     WrappedSetting("Solar delta",
                                    "Start when the roof loop is 4 °C warmer "
                                    "than the pool")),
        safety_(context, material3::StandardListItemInit::OneLine(
                             "Safety lock", nullptr, &safety_lock_)) {
    setPadding(Padding(Scaled(16), Scaled(12)));
    setGap(Scaled(10));

    // Inset dividers align with text rather than cutting through the screen.
    material3::ListDividerPolicy dividers;
    dividers.mode = material3::DividerMode::kInset;
    dividers.start_inset = 16;
    dividers.end_inset = 16;
    list_.setDividerPolicy(dividers);

    list_.add(pump_mode_);
    list_.add(solar_delta_);
    list_.add(safety_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(guidance_, {.flex_grow = 0, .flex_shrink = 0});
    add(list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel guidance_;
  material3::List list_;
  material3::Switch safety_lock_;
  material3::ListRow<material3::StandardListItem> pump_mode_;
  material3::ListRow<material3::StandardListItem> solar_delta_;
  material3::ListRow<material3::StandardListItem> safety_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
PoolSettings pool_settings(app.context());
SingletonActivity activity(app, pool_settings);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
