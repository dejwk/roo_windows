// Learning goal: change shared Material 3 card surface roles without adding
// per-card overrides.
//
// Try changing the ColorToken values in ApplicationTheme(). Every matching
// component picks up its shared role, while a card that calls
// setContainerRole() keeps its local override. The selected role still uses
// the application's Material palette, so it remains useful in light and dark
// themes without copying literal colors into each card.

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
#include "roo_windows/material3/card/flex_card.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kTouchCsPin = 1;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

const Theme& ApplicationTheme() {
  // Material3Theme is application-owned shared data. Widgets only borrow it,
  // so keep this storage static (or otherwise longer lived than the app and
  // all of its widgets).
  static const material3::Material3Theme material = [] {
    // Start with all normal Material defaults, then replace only the shared
    // card surface slots this application wants to customize.
    material3::Material3Theme result = DefaultTheme().material3Theme();

    // Play with any neutral surface role here: kSurface,
    // kSurfaceContainerLowest, kSurfaceContainerLow, kSurfaceContainer,
    // kSurfaceContainerHigh, or kSurfaceContainerHighest. This changes all
    // filled cards that do not have an explicit container-role override.
    result.components.card.filledContainer =
        material3::ColorToken::kSurfaceContainerLow;

    // Elevated and outlined cards have independent slots. Uncomment or edit
    // outlinedContainer to see that changing one card style does not alter
    // the others.
    result.components.card.elevatedContainer =
        material3::ColorToken::kSurfaceContainer;
    // result.components.card.outlinedContainer =
    //     material3::ColorToken::kSurfaceContainerHigh;

    // Dialog surfaces are independent of cards and search panels. Try this
    // with any BasicDialog in the application; full-screen dialogs have their
    // own fullScreenContainer slot and keep the Material surface default.
    result.components.dialog.basicContainer =
        material3::ColorToken::kSurfaceContainer;
    return result;
  }();

  // Theme keeps a non-owning pointer to `material`; MakeFrameworkTheme()
  // separately derives the framework-level colors from the same palette.
  static const Theme theme = {material3::MakeFrameworkTheme(material),
                              &material};
  return theme;
}

class SurfaceRoleDemo : public material3::FlexCard {
 public:
  explicit SurfaceRoleDemo(ApplicationContext& context)
      : FlexCard(context),
        title_(context, "Shared card surface role",
               material3::text_style_title_large()),
        detail_(context,
                "Filled cards use surfaceContainerLow from the app theme.",
                material3::text_style_body_medium()) {
    // This card has no setContainerRole() call, so its filled surface comes
    // from material.components.card.filledContainer above.
    add(title_);
    add(detail_);
  }

 private:
  TextLabel title_;
  TextLabel detail_;
};

roo_scheduler::Scheduler scheduler;
Environment environment(scheduler, ApplicationTheme());
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif
SurfaceRoleDemo demo(app.context());
Task& task = app.addTaskFullScreen(demo);

void setup() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
  app.start();
  scheduler.run();
}

void loop() {}
