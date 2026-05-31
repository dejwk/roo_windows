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

// Example 2: Settings list row

#include "roo_icons/filled/24/device.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_label.h"

class SettingRow : public FlexLayout {
 public:
  SettingRow(ApplicationContext& context,
             const roo_display::Pictogram& icon_def, const char* primary,
             const char* secondary, bool initial_on = false)
      : FlexLayout(context, FlexDirection::kRow),
        icon_(context, icon_def),
        labels_(context, FlexDirection::kColumn),
        primary_(context, primary, font_body1()),
        secondary_(context, secondary, font_caption()),
        toggle_(context, initial_on) {
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(12));
    add(icon_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0});
    labels_.add(secondary_, {.flex_grow = 0});
    add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    add(toggle_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  Icon icon_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  Switch toggle_;
};

class SettingsScreen : public FlexLayout {
 public:
  SettingsScreen(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        wifi_(context, ic_filled_24_device_wifi_tethering(), "Wi-Fi",
              "Connected to HomeNet", true),
        div1_(context),
        bt_(context, ic_filled_24_device_bluetooth(), "Bluetooth", "Off",
            false),
        div2_(context),
        brightness_(context, ic_filled_24_device_brightness_medium(),
                    "Brightness", "70 %", true) {
    add(wifi_);
    add(div1_);
    add(bt_);
    add(div2_);
    add(brightness_);
  }

 private:
  SettingRow wifi_;
  HorizontalDivider div1_;
  SettingRow bt_;
  HorizontalDivider div2_;
  SettingRow brightness_;
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
SettingsScreen settings_screen(app.context());
SingletonActivity activity(app, settings_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
