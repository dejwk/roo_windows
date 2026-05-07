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
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

int PercentFromPos(uint16_t pos) {
  return ((uint32_t)pos * 100u + 32767u) / 65535u;
}

uint16_t PosFromPercent(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;
  return ((uint32_t)percent * 65535u + 50u) / 100u;
}

class SliderRow : public FlexLayout {
 public:
  SliderRow(const Environment& env, const char* primary, const char* secondary,
            int initial_percent)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, PosFromPercent(initial_percent)) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    header_.setAlignItems(AlignItems::kCenter);
    header_.setGap(Scaled(12));

    labels_.setAlignItems(AlignItems::kFlexStart);
    labels_.add(primary_, {.flex_grow = 0, .flex_shrink = 0});
    labels_.add(secondary_, {.flex_grow = 0, .flex_shrink = 0});

    header_.add(labels_, {.flex_grow = 1, .flex_shrink = 1});
    header_.add(value_, {.flex_grow = 0, .flex_shrink = 0});

    add(header_, {.flex_grow = 0, .flex_shrink = 0});
    add(slider_, {.flex_grow = 0, .flex_shrink = 1});

    slider_.setOnInteractiveChange([this]() { handleInteractiveChange(); });
    updateValue();
  }

  void setOnInteractiveChange(std::function<void()> handler) {
    on_change_ = std::move(handler);
  }

  int percent() const { return PercentFromPos(slider_.getPos()); }

 private:
  void updateValue() { value_.setTextf("%d%%", percent()); }

  void handleInteractiveChange() {
    updateValue();
    if (on_change_ != nullptr) {
      on_change_();
    }
  }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  material3::Slider slider_;
  std::function<void()> on_change_;
};

class FullWidthColumn : public FlexLayout {
 public:
  explicit FullWidthColumn(const Environment& env)
      : FlexLayout(env, FlexDirection::kColumn) {}

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

class SliderScreen : public SimpleScrollablePanel {
 public:
  SliderScreen(const Environment& env)
      : SimpleScrollablePanel(env),
        content_(env),
        title_(env, "Material 3 sliders", font_body1()),
        subtitle_(env, "Drag the handles or tap the track to set a value.",
                  font_caption()),
        divider_(env),
        summary_(env, "", font_caption()),
        media_(env, "Media volume", "Speaker output for videos and games", 72),
        brightness_(env, "Screen brightness",
                    "Balances visibility and battery life", 58),
        warmth_(env, "Color warmth", "Moves the panel from cool to warm tones",
                36),
        footer_divider_(env),
        note_(env, "This demo uses the standard continuous Material 3 slider.",
              font_caption()) {
    content_.setPadding(Scaled(12));
    content_.setGap(Scaled(4));

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(summary_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(media_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(brightness_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(warmth_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(footer_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(note_, {.flex_grow = 0, .flex_shrink = 0});

    media_.setOnInteractiveChange([this]() { updateSummary(); });
    brightness_.setOnInteractiveChange([this]() { updateSummary(); });
    warmth_.setOnInteractiveChange([this]() { updateSummary(); });

    setContents(content_);
    updateSummary();
  }

 private:
  void updateSummary() {
    int average =
        (media_.percent() + brightness_.percent() + warmth_.percent()) / 3;
    summary_.setTextf("Average level: %d%%", average);
  }

  FullWidthColumn content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider divider_;
  TextLabel summary_;
  SliderRow media_;
  SliderRow brightness_;
  SliderRow warmth_;
  HorizontalDivider footer_divider_;
  TextLabel note_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
SliderScreen slider_screen(env);
SingletonActivity activity(app, slider_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}