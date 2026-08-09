// Learning goal: assign responsive GridSpan values to dashboard cards. The
// same insertion order becomes a four-column compact dashboard, an eight-
// column medium layout, and a twelve-column expanded layout.

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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {
material3::GridLayout::Params CardParams(uint8_t compact, uint8_t medium,
                                         uint8_t expanded) {
  material3::GridSpan span;
  span.compact = compact;
  span.medium = medium;
  span.expanded = expanded;
  span.large = expanded;
  span.extra_large = expanded;
  material3::GridLayout::Params params;
  params.span = span;
  return params;
}

/// Dashboard whose cards repack according to their semantic span rules.
class PoolDashboard : public FlexLayout {
 public:
  explicit PoolDashboard(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pool dashboard", material3::text_style_title_large()),
        grid_(context),
        temperature_(context, "Water 27.4 °C",
                     material3::ButtonVariant::kFilled),
        solar_(context, "Solar gain 4.2 kW",
               material3::ButtonVariant::kFilledTonal),
        history_(context, "Pump history · 6 h today",
                 material3::ButtonVariant::kOutlined) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));
    // Compact 2+2, medium 4+4, and expanded 6+6 each fill one row.
    grid_.add(temperature_, CardParams(2, 4, 6));
    grid_.add(solar_, CardParams(2, 4, 6));
    grid_.add(history_, CardParams(4, 8, 12));
    add(title_);
    add(grid_, {.flex_grow = 1});
  }

 private:
  TextLabel title_;
  material3::GridLayout grid_;
  material3::Button temperature_;
  material3::Button solar_;
  material3::Button history_;
};
}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
PoolDashboard dashboard(app.context());
UiTask& task = app.addUiTaskFullScreen(dashboard);
void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
