// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flexViewport;

  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flexViewport(viewport, 1, FlexViewport::kRotationRight),
        display(flexViewport),
        touch(flexViewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                        true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Select the driver to match your display device.
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

// Set your configuration for the driver.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kBlPin = 20;

static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;

static constexpr int kTouchCsPin = 1;

// Uncomment if you have connected the BL pin to GPIO.

// #include "roo_display/backlit/esp32_ledc.h"
// LedcBacklit backlit(kBlPin, /* ledc channel */ 0);

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;

Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.init();
}

// *************** EXAMPLE STARTS HERE

// Example 4: Dashboard card grid

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/widgets/text_label.h"

class MetricCard : public FlexLayout {
 public:
  MetricCard(const Environment& env, const char* value, const char* caption)
      : FlexLayout(env, FlexDirection::kColumn),
        value_(env, value, font_h5()),
        caption_(env, caption, font_caption()) {
    setAlignItems(AlignItems::kStretch);
    setJustifyContent(JustifyContent::kCenter);
    setPadding(Padding(PaddingSize::kRegular));

    add(value_, {.flex_grow = 0});
    add(caption_, {.flex_grow = 0});
  }

 private:
  TextLabel value_;
  TextLabel caption_;
};

class DashboardGrid : public FlexLayout {
 public:
  DashboardGrid(const Environment& env)
      : FlexLayout(env, FlexDirection::kRow),
        temp_(env, "23 °C", "Temperature"),
        humidity_(env, "61 %", "Humidity"),
        pressure_(env, "1013 hPa", "Pressure"),
        co2_(env, "412 ppm", "CO₂"),
        wind_(env, "14 km/h", "Wind"),
        uv_(env, "3", "UV index") {
    setFlexWrap(FlexWrap::kWrap);
    setJustifyContent(JustifyContent::kSpaceBetween);
    setAlignContent(AlignContent::kFlexStart);
    setAlignItems(AlignItems::kStretch);
    setRowGap(Scaled(8));

    FlexLayout::Params card_params{
        .flex_grow = 1,
        .flex_shrink = 1,
        .flex_basis = FlexBasis::kZero,
    };

    add(temp_, card_params);
    add(humidity_, card_params);
    add(pressure_, card_params);
    add(co2_, card_params);
    add(wind_, card_params);
    add(uv_, card_params);
  }

 private:
  MetricCard temp_;
  MetricCard humidity_;
  MetricCard pressure_;
  MetricCard co2_;
  MetricCard wind_;
  MetricCard uv_;
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
DashboardGrid dashboard_grid(env);
SingletonActivity activity(app, dashboard_grid);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
