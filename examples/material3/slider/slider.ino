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
#include "roo_windows/material3/slider/range_slider.h"
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

class LegacySliderRow : public FlexLayout {
 public:
  LegacySliderRow(const Environment& env, const char* primary,
                  const char* secondary, int initial_percent)
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

using SliderValueFormatter =
    std::function<void(TextLabel&, const material3::Slider&)>;

class SemanticSliderRow : public FlexLayout {
 public:
  SemanticSliderRow(const Environment& env, const char* primary,
                    const char* secondary, material3::SliderRange range,
                    float initial_value, SliderValueFormatter formatter,
                    material3::SliderVariant variant =
                        material3::SliderVariant::kStandard)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, range, initial_value, variant),
        formatter_(std::move(formatter)) {
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

 private:
  void updateValue() { formatter_(value_, slider_); }

  void handleInteractiveChange() { updateValue(); }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  material3::Slider slider_;
  SliderValueFormatter formatter_;
};

class DemoRangeSlider : public material3::RangeSlider {
 public:
  using UpdateHandler = std::function<void()>;

  DemoRangeSlider(const Environment& env, material3::SliderRange range,
                  float start_value, float end_value)
      : material3::RangeSlider(env, range, start_value, end_value) {}

  void setOnStateChange(UpdateHandler handler) {
    on_state_change_ = std::move(handler);
  }

  void onValueChange(float start, float end, int active_thumb,
                     bool from_user) override {
    (void)start;
    (void)end;
    (void)active_thumb;
    (void)from_user;
    notifyStateChange();
  }

  void onInteractionStart(int active_thumb) override {
    (void)active_thumb;
    notifyStateChange();
  }

  void onInteractionEnd(float start, float end) override {
    (void)start;
    (void)end;
    notifyStateChange();
  }

 private:
  void notifyStateChange() {
    if (on_state_change_ != nullptr) {
      on_state_change_();
    }
  }

  UpdateHandler on_state_change_;
};

class RangeSliderRow : public FlexLayout {
 public:
  RangeSliderRow(const Environment& env, const char* primary,
                 const char* secondary, material3::SliderRange range,
                 float start_value, float end_value, float min_separation)
      : FlexLayout(env, FlexDirection::kColumn),
        header_(env, FlexDirection::kRow),
        labels_(env, FlexDirection::kColumn),
        primary_(env, primary, font_body1()),
        secondary_(env, secondary, font_caption()),
        value_(env, "", font_body1()),
        slider_(env, range, start_value, end_value),
        status_(env, "", font_caption()),
        min_separation_(min_separation) {
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
    add(status_, {.flex_grow = 0, .flex_shrink = 0});

    slider_.setMinSeparation(min_separation);
    slider_.setOnStateChange([this]() { updateLabels(); });
    slider_.setOnInteractiveChange([this]() { updateLabels(); });
    updateLabels();
  }

 private:
  const char* ActiveThumbLabel() const {
    int active_thumb = slider_.activeThumbIndex();
    if (active_thumb == 0) return "adjusting start thumb";
    if (active_thumb == 1) return "adjusting end thumb";
    return "idle";
  }

  void updateLabels() {
    value_.setTextf("%02d:00-%02d:00", (int)slider_.startValue(),
                    (int)slider_.endValue());
    status_.setTextf("%s · minimum gap %dh", ActiveThumbLabel(),
                     (int)min_separation_);
  }

  FlexLayout header_;
  FlexLayout labels_;
  TextLabel primary_;
  TextLabel secondary_;
  TextLabel value_;
  DemoRangeSlider slider_;
  TextLabel status_;
  float min_separation_;
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
        subtitle_(env,
                  "Compatibility, semantic, discrete, centered, and range "
                  "slider behavior in one demo.",
                  font_caption()),
        divider_(env),
        migration_(env,
                   "The first row still uses Slider(env, pos). The rest use "
                   "semantic ranges.",
                   font_caption()),
        legacy_(env, "Legacy compatibility",
                "Normalized position constructor mapped to a percentage", 72),
        fan_speed_(env, "Discrete fan speed",
                   "Whole-step snapping on taps and drags",
                   material3::SliderRange{0.0f, 5.0f, 1.0f}, 2.0f,
                   [](TextLabel& label, const material3::Slider& slider) {
                     label.setTextf("%.0fx", slider.value());
                   }),
        balance_(env, "Centered balance",
                 "Active track grows away from the midpoint anchor",
                 material3::SliderRange{-100.0f, 100.0f, 5.0f}, -20.0f,
                 [](TextLabel& label, const material3::Slider& slider) {
                   label.setTextf("%+.0f", slider.value());
                 },
                 material3::SliderVariant::kCentered),
        quiet_hours_(env, "Quiet hours",
                     "Range slider with active thumb state and 2h minimum "
                     "separation",
                     material3::SliderRange{0.0f, 24.0f, 1.0f}, 8.0f, 18.0f,
                     2.0f),
        footer_divider_(env),
        note_(env,
              "Value indicators and richer styling are intentionally left for "
              "later steps. This screen is for visually confirming the "
              "behavior implemented so far.",
              font_caption()) {
    content_.setPadding(Scaled(12));
    content_.setGap(Scaled(4));

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(migration_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(legacy_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(fan_speed_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(balance_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(quiet_hours_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(footer_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(note_, {.flex_grow = 0, .flex_shrink = 0});

    setContents(content_);
  }

  FullWidthColumn content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider divider_;
  TextLabel migration_;
  LegacySliderRow legacy_;
  SemanticSliderRow fan_speed_;
  SemanticSliderRow balance_;
  RangeSliderRow quiet_hours_;
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