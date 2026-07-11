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

// Example 5: Full-screen "Holy Grail" layout

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/text_label.h"

class NamedPanel : public FlexLayout {
 public:
  NamedPanel(ApplicationContext& context, const char* name,
             ::roo_windows::material3::ColorToken role = ::roo_windows::material3::ColorToken::kSurfaceContainer)
      : FlexLayout(context, FlexDirection::kRow),
        label_(context, name, font_body1()),
        role_(role) {
    setJustifyContent(JustifyContent::kCenter);
    setAlignItems(AlignItems::kCenter);
    add(label_);
  }

  ::roo_windows::material3::ColorToken containerRole() const override { return role_; }

  BorderStyle getBorderStyle() const override { return BorderStyle(0, 1); }

  Color getOutlineColor() const override {
    return theme().color.outlineVariant;
  }

 private:
  TextLabel label_;
  ::roo_windows::material3::ColorToken role_;
};

class HolyGrail : public FlexLayout {
 public:
  HolyGrail(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        header_(context, "Header", ::roo_windows::material3::ColorToken::kSurfaceContainerHigh),
        middle_(context, FlexDirection::kRow),
        nav_(context, "Nav", ::roo_windows::material3::ColorToken::kSecondaryContainer),
        content_(context, "Content", ::roo_windows::material3::ColorToken::kPrimaryContainer),
        sidebar_(context, "Sidebar", ::roo_windows::material3::ColorToken::kTertiaryContainer),
        footer_(context, "Footer", ::roo_windows::material3::ColorToken::kSurfaceContainerHigh) {
    setAlignItems(AlignItems::kStretch);

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    middle_.setAlignItems(AlignItems::kStretch);
    nav_.setMinimumDimensions(Dimensions(Scaled(60), 0));
    middle_.add(nav_, {.flex_grow = 0, .flex_shrink = 0});
    middle_.add(content_, {.flex_grow = 1, .flex_shrink = 1});
    sidebar_.setMinimumDimensions(Dimensions(Scaled(80), 0));
    middle_.add(sidebar_, {.flex_grow = 0, .flex_shrink = 0});
    add(middle_, {.flex_grow = 1, .flex_shrink = 1});
    add(footer_, {.flex_grow = 0, .flex_shrink = 0});
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::MatchParentHeight());
  }

 private:
  NamedPanel header_;
  FlexLayout middle_;
  NamedPanel nav_;
  NamedPanel content_;
  NamedPanel sidebar_;
  NamedPanel footer_;
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
HolyGrail holy_grail(app.context());
SingletonActivity activity(app, holy_grail);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
