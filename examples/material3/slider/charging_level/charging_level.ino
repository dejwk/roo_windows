// Learning goal: present a level with a vertical, bottom-to-top slider.
// Tap or drag the tank control and observe its stepped percentage update.

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
#include "roo_windows/containers/holder.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"

namespace {

/// Makes low values appear at the bottom and exposes each ten-percent step.
constexpr material3::SliderStyle ChargingLevelStyle() {
  material3::SliderStyle style;
  style.orientation = material3::SliderOrientation::kVertical;
  style.direction = material3::SliderDirection::kInverted;
  style.tick_mode = material3::SliderTickMode::kShowTicks;
  style.value_indicator =
      material3::SliderValueIndicatorBehavior::kShowOnInteraction;
  return style;
}

/// Holder that reserves the height required to demonstrate a vertical slider.
class VerticalSliderSlot : public Holder {
 public:
  /// Creates an empty fixed-size slot.
  explicit VerticalSliderSlot(ApplicationContext& context) : Holder(context) {}

 protected:
  /// Gives the child a tall, narrow layout area.
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(Scaled(100)),
                         PreferredSize::ExactHeight(Scaled(168)));
  }
};

/// Vertical slider that binds the tank percentage to status text.
class ChargingLevelSlider : public material3::Slider {
 public:
  /// Creates a zero-to-100-percent control with ten-percent steps.
  ChargingLevelSlider(ApplicationContext& context, TextLabel& value_label)
      : material3::Slider(context, {0.0f, 100.0f, 10.0f}, 60.0f,
                          material3::SliderVariant::kStandard,
                          ChargingLevelStyle()),
        value_label_(value_label) {
    updateLabel(value());
  }

  /// Reflects each snapped level in the application state label.
  void onValueChange(float value, bool from_user) override {
    (void)from_user;
    updateLabel(value);
  }

  /// Formats the value bubble as a percentage.
  roo::string_view formatLabel(float value, char* scratch,
                               size_t scratch_size) const override {
    if (scratch_size == 0) return {};
    int length = snprintf(scratch, scratch_size, "%.0f%%", value);
    if (length < 0) length = 0;
    if (static_cast<size_t>(length) >= scratch_size) {
      length = static_cast<int>(scratch_size - 1);
    }
    return roo::string_view(scratch, static_cast<size_t>(length));
  }

 private:
  void updateLabel(float value) { value_label_.setTextf("%.0f%% full", value); }

  TextLabel& value_label_;
};

/// Charging screen demonstrating orientation independently from value semantics.
class ChargingLevel : public FlexLayout {
 public:
  /// Creates the vertical charging-level lesson.
  explicit ChargingLevel(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kRow),
        details_(context, FlexDirection::kColumn),
        title_(context, "Target level", material3::text_style_title_large()),
        guidance_(context, "Ten-percent steps keep sparse ticks readable",
                  material3::text_style_body_medium(), kTop | kLeft),
        value_(context, "", material3::text_style_display_small()),
        slider_slot_(context),
        slider_(context, value_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    setAlignItems(AlignItems::kCenter);
    details_.setGap(Scaled(12));
    details_.add(title_);
    details_.add(guidance_);
    details_.add(value_);
    slider_slot_.setContents(slider_);
    add(details_, {.flex_grow = 1});
    add(slider_slot_);
  }

 private:
  FlexLayout details_;
  TextLabel title_;
  TextBlock guidance_;
  TextLabel value_;
  VerticalSliderSlot slider_slot_;
  ChargingLevelSlider slider_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
ChargingLevel charging_level(app.context());
UiTask& task = app.addUiTaskFullScreen(charging_level);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
