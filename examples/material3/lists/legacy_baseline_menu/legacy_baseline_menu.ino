// Learning goal: retain the square, full-width-divided baseline list style for
// a compatibility menu. New application lists should use the default variant.

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
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Compatibility menu using the opt-in baseline list variant.
class LegacyServiceMenu : public FlexLayout {
 public:
  /// Creates a baseline menu with shortcut labels and full-width dividers.
  explicit LegacyServiceMenu(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Legacy service menu",
               material3::text_style_title_large()),
        guidance_(context, "Use only when matching an existing square menu",
                  material3::text_style_body_medium()),
        menu_(context),
        refresh_key_(context, "R", material3::text_style_label_medium()),
        edit_key_(context, "E", material3::text_style_label_medium()),
        remove_key_(context, "Del", material3::text_style_label_medium()),
        refresh_(context, material3::StandardListItemInit::OneLine(
                              "Refresh", nullptr, &refresh_key_)),
        edit_(context, material3::StandardListItemInit::OneLine(
                           "Edit schedule", nullptr, &edit_key_)),
        remove_(context, material3::StandardListItemInit::OneLine(
                             "Remove", nullptr, &remove_key_)) {
    setPadding(Padding(Scaled(16), Scaled(12)));
    setGap(Scaled(10));

    // The baseline variant is deliberately isolated in this legacy example.
    menu_.setVariant(material3::ListVariant::kBaseline);
    material3::ListDividerPolicy dividers;
    dividers.mode = material3::DividerMode::kFullWidth;
    menu_.setDividerPolicy(dividers);

    menu_.add(refresh_);
    menu_.add(edit_);
    menu_.add(remove_);
    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(guidance_, {.flex_grow = 0, .flex_shrink = 0});
    add(menu_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel guidance_;
  material3::List menu_;
  TextLabel refresh_key_;
  TextLabel edit_key_;
  TextLabel remove_key_;
  material3::ListRow<material3::StandardListItem> refresh_;
  material3::ListRow<material3::StandardListItem> edit_;
  material3::ListRow<material3::StandardListItem> remove_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
LegacyServiceMenu legacy_service_menu(app.context());
SingletonActivity activity(app, legacy_service_menu);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
