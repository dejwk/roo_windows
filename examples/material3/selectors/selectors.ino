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

#include <utility>

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
  display.enableTurbo();
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/checkbox/checkbox.h"
#include "roo_windows/material3/radio_button/radio_button.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

template <typename Control>
class SelectorRow : public FlexLayout {
 public:
  template <typename... Args>
  SelectorRow(ApplicationContext& context, const char* primary,
              const char* secondary, Args&&... args)
      : FlexLayout(context, FlexDirection::kRow),
        labels_(context, FlexDirection::kColumn),
        primary_(context, primary, font_body1()),
        secondary_(context, secondary, font_caption()),
        control_(context, std::forward<Args>(args)...) {
    setAlignItems(AlignItems::kCenter);
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(12));
    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});
    add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    add(control_, {.flex_grow = 0, .flex_shrink = 0});
  }

  Control& control() { return control_; }
  const Control& control() const { return control_; }

 private:
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  Control control_;
};

class SelectorScreen : public SimpleScrollablePanel {
 public:
  SelectorScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 selectors", font_body1()),
        subtitle_(context, "Tap each control to inspect state changes.",
                  font_caption()),
        top_divider_(context),
        checkbox_heading_(context, "Checkboxes", font_body1()),
        checkbox_status_(context, "", font_caption()),
        notifications_(context, "Notifications", "Starts enabled",
                       OnOffState::kOn),
        downloads_(context, "Offline downloads", "Starts indeterminate",
                   OnOffState::kIndeterminate),
        analytics_(context, "Anonymous analytics", "Starts disabled",
                   OnOffState::kOff),
        mid_divider_(context),
        radio_heading_(context, "Radio buttons", font_body1()),
        radio_status_(context, "", font_caption()),
        compact_(context, "Compact layout", "More density on small screens",
                 false),
        balanced_(context, "Balanced layout",
                  "Default spacing for mixed content", true),
        comfortable_(context, "Comfortable layout", "Extra padding for reading",
                     false),
        bottom_divider_(context),
        switch_heading_(context, "Switches", font_body1()),
        switch_status_(context, "", font_caption()),
        bluetooth_(context, "Bluetooth", "Nearby accessories", true),
        dark_theme_(context, "Dark theme", "Applies app-wide appearance",
                    false) {
    content_.setPadding(Scaled(12));
    content_.setGap(Scaled(4));
    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(top_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(checkbox_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(checkbox_status_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(notifications_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(downloads_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(analytics_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(mid_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(radio_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(radio_status_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(compact_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(balanced_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(comfortable_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(bottom_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(switch_heading_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(switch_status_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(bluetooth_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(dark_theme_, {.flex_grow = 0, .flex_shrink = 0});

    notifications_.control().setOnInteractiveChange(
        [this]() { updateCheckboxSummary(); });
    downloads_.control().setOnInteractiveChange(
        [this]() { updateCheckboxSummary(); });
    analytics_.control().setOnInteractiveChange(
        [this]() { updateCheckboxSummary(); });

    compact_.control().setOnInteractiveChange([this]() { selectDensity(0); });
    balanced_.control().setOnInteractiveChange([this]() { selectDensity(1); });
    comfortable_.control().setOnInteractiveChange(
        [this]() { selectDensity(2); });

    bluetooth_.control().setOnInteractiveChange(
        [this]() { updateSwitchSummary(); });
    dark_theme_.control().setOnInteractiveChange(
        [this]() { updateSwitchSummary(); });

    setContents(content_);

    updateCheckboxSummary();
    selectDensity(1);
    updateSwitchSummary();
  }

 private:
  void updateCheckboxSummary() {
    int selected =
        notifications_.control().isOn() + analytics_.control().isOn();
    int mixed =
        downloads_.control().onOffState() == OnOffState::kIndeterminate ? 1 : 0;
    if (downloads_.control().isOn()) {
      ++selected;
    }
    checkbox_status_.setTextf("%d selected, %d mixed", selected, mixed);
  }

  void selectDensity(int idx) {
    idx == 0 ? compact_.control().setOn() : compact_.control().setOff();
    idx == 1 ? balanced_.control().setOn() : balanced_.control().setOff();
    idx == 2 ? comfortable_.control().setOn() : comfortable_.control().setOff();
    switch (idx) {
      case 0:
        radio_status_.setText("Compact is active.");
        break;
      case 1:
        radio_status_.setText("Balanced is active.");
        break;
      default:
        radio_status_.setText("Comfortable is active.");
        break;
    }
  }

  void updateSwitchSummary() {
    int enabled = bluetooth_.control().isOn() + dark_theme_.control().isOn();
    switch_status_.setTextf("%d of 2 toggles enabled", enabled);
  }

  FlexLayout content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider top_divider_;
  TextLabel checkbox_heading_;
  TextLabel checkbox_status_;
  SelectorRow<material3::Checkbox> notifications_;
  SelectorRow<material3::Checkbox> downloads_;
  SelectorRow<material3::Checkbox> analytics_;
  HorizontalDivider mid_divider_;
  TextLabel radio_heading_;
  TextLabel radio_status_;
  SelectorRow<material3::RadioButton> compact_;
  SelectorRow<material3::RadioButton> balanced_;
  SelectorRow<material3::RadioButton> comfortable_;
  HorizontalDivider bottom_divider_;
  TextLabel switch_heading_;
  TextLabel switch_status_;
  SelectorRow<material3::Switch> bluetooth_;
  SelectorRow<material3::Switch> dark_theme_;
};

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
SelectorScreen selector_screen(app.context());
SingletonActivity activity(app, selector_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}
