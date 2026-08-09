// Learning goal: use a range slider for a bounded time window.
// Move either thumb and observe the selected quiet period and active endpoint.

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

#include <stdio.h>

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
#include "roo_windows/material3/slider/range_slider.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Shows hourly stops and labels the thumb being adjusted.
constexpr material3::SliderStyle QuietHoursStyle() {
  material3::SliderStyle style;
  style.size = material3::SliderSize::kLarge;
  style.tick_mode = material3::SliderTickMode::kShowTicks;
  // No need for the bubble since we're showing the selected range in a separate label.
  style.value_indicator = material3::SliderValueIndicatorBehavior::kHidden;
  return style;
}

/// Range slider that publishes both selected endpoints to application UI.
class QuietHoursSlider : public material3::RangeSlider {
 public:
  /// Creates an hourly time range and enforces a two-hour minimum window.
  QuietHoursSlider(ApplicationContext& context, TextLabel& value_label,
                   TextLabel& interaction_label)
      : material3::RangeSlider(context, {0.0f, 24.0f, 1.0f}, 21.0f, 24.0f,
                               QuietHoursStyle()),
        value_label_(value_label),
        interaction_label_(interaction_label) {
    setMinSeparation(2.0f);
    updateLabels(startValue(), endValue(), -1);
  }

  /// Publishes both endpoints and identifies the active thumb.
  void onValueChange(float start, float end, int active_thumb,
                     bool from_user) override {
    (void)from_user;
    updateLabels(start, end, active_thumb);
  }

  /// Marks which endpoint owns a newly started interaction.
  void onInteractionStart(int active_thumb) override {
    updateInteraction(active_thumb);
  }

  /// Clears the transient interaction status after the gesture finishes.
  void onInteractionEnd(float start, float end) override {
    updateLabels(start, end, -1);
  }

  /// Formats each endpoint bubble as an hour on a 24-hour clock.
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (scratch_size == 0) return {};
    int length =
        snprintf(scratch, scratch_size, "%02d:00", static_cast<int>(value));
    if (length < 0) length = 0;
    if (static_cast<size_t>(length) >= scratch_size) {
      length = static_cast<int>(scratch_size - 1);
    }
    return roo::string_view(scratch, static_cast<size_t>(length));
  }

 private:
  void updateLabels(float start, float end, int active_thumb) {
    value_label_.setTextf("%02d:00–%02d:00", static_cast<int>(start),
                          static_cast<int>(end));
    updateInteraction(active_thumb);
  }

  void updateInteraction(int active_thumb) {
    if (active_thumb == 0) {
      interaction_label_.setText("Adjusting start time");
    } else if (active_thumb == 1) {
      interaction_label_.setText("Adjusting end time");
    } else {
      interaction_label_.setText("Minimum quiet period: 2 hours");
    }
  }

  TextLabel& value_label_;
  TextLabel& interaction_label_;
};

/// Schedule screen demonstrating a two-value domain and separation rule.
class QuietHours : public FlexLayout {
 public:
  /// Creates the pump quiet-hours lesson.
  explicit QuietHours(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Pump quiet hours",
               material3::text_style_title_large()),
        guidance_(context, "Each thumb owns one endpoint of the time window",
                  material3::text_style_body_medium(), kTop | kLeft),
        value_(context, "", material3::text_style_display_small()),
        interaction_(context, "", material3::text_style_body_medium()),
        slider_(context, value_, interaction_) {
    // Keep the large slider at its intrinsic 68 px height. This compact
    // spacing leaves room for the two-line guidance and status text within
    // the 240 px example display.
    setPadding(Padding(Scaled(8)));
    setGap(Scaled(4));
    add(title_);
    add(guidance_);
    add(value_);
    add(slider_);
    add(interaction_);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  TextLabel value_;
  TextLabel interaction_;
  QuietHoursSlider slider_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
QuietHours quiet_hours(app.context());
UiTask& task = app.addUiTaskFullScreen(quiet_hours);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
