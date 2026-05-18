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

#include <functional>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_icons/outlined/24/action.h"
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
#include "roo_windows/material3/button/button.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class ButtonRow : public FlexLayout {
 public:
  ButtonRow(const Environment& env, const char* title,
            material3::ButtonVariant variant)
      : FlexLayout(env, FlexDirection::kColumn),
        title_(env, title, font_caption()),
        button_(env, "Action", variant),
        button_with_icon_(env, "Action", variant),
        disabled_(env, "Disabled", variant) {
    setPadding(Padding(Scaled(12), Scaled(6)));
    setGap(Scaled(6));

    button_with_icon_.setIcon(&ic_outlined_24_action_done());

    disabled_.setEnabled(false);

    button_.setOnInteractiveChange([this]() { handleClick(); });
    button_with_icon_.setOnInteractiveChange([this]() { handleClick(); });

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(button_, {.flex_grow = 0, .flex_shrink = 0});
    add(button_with_icon_, {.flex_grow = 0, .flex_shrink = 0});
    add(disabled_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  void handleClick() {
    counter_++;
    char buf[24];
    snprintf(buf, sizeof(buf), "clicked %d", counter_);
    title_.setText(buf);
  }

  TextLabel title_;
  material3::Button button_;
  material3::Button button_with_icon_;
  material3::Button disabled_;
  int counter_ = 0;
};

class ButtonScreen : public ScrollablePanel {
 public:
  ButtonScreen(const Environment& env)
      : ScrollablePanel(env),
        content_(env, FlexDirection::kColumn),
        title_(env, "Material 3 buttons", font_h6()),
        subtitle_(env, "Phase 1 \xe2\x80\x94 five variants", font_caption()),
        divider_(env),
        elevated_(env, "Elevated", material3::ButtonVariant::kElevated),
        filled_(env, "Filled", material3::ButtonVariant::kFilled),
        tonal_(env, "Filled tonal", material3::ButtonVariant::kFilledTonal),
        outlined_(env, "Outlined", material3::ButtonVariant::kOutlined),
        text_(env, "Text", material3::ButtonVariant::kText) {
    content_.setPadding(Padding(Scaled(12), Scaled(8)));
    content_.setGap(Scaled(8));

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(elevated_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(filled_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(tonal_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(outlined_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(text_, {.flex_grow = 0, .flex_shrink = 0});

    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider divider_;
  ButtonRow elevated_;
  ButtonRow filled_;
  ButtonRow tonal_;
  ButtonRow outlined_;
  ButtonRow text_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
ButtonScreen button_screen(env);
SingletonActivity activity(app, button_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}
