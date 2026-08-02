// Learning goal: model one exclusive display-density choice with radio
// buttons. Selecting one option always clears the other two.

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
#include "roo_windows/material3/radio_button/radio_button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"

namespace {

/// Minimal interface used by each radio button to select its group index.
class DensityGroup {
 public:
  /// Allows safe destruction through the group interface.
  virtual ~DensityGroup() = default;

  /// Selects exactly one density by its group index.
  virtual void selectDensity(int index) = 0;
};

/// Radio button that delegates exclusive selection to its owning group.
class DensityRadio : public material3::RadioButton {
 public:
  /// Creates a radio button associated with one group index.
  DensityRadio(ApplicationContext& context, DensityGroup& group, int index)
      : material3::RadioButton(context), group_(group), index_(index) {}

  /// Delegates selection so the group can clear every other option.
  void onClicked() override { group_.selectDensity(index_); }

 private:
  DensityGroup& group_;
  int8_t index_;
};

/// One labeled density option with a leading radio button.
class DensityRow : public FlexLayout {
 public:
  /// Creates a row associated with one exclusive group index.
  DensityRow(ApplicationContext& context, roo::string_view label,
             DensityGroup& group, int index)
      : FlexLayout(context, FlexDirection::kRow),
        radio_(context, group, index),
        label_(context, std::string(label),
               material3::text_style_body_large()) {
    setAlignItems(AlignItems::kCenter);
    setGap(Scaled(12));
    add(radio_, {.flex_grow = 0, .flex_shrink = 0});
    add(label_, {.flex_grow = 1, .flex_shrink = 1});
  }

  /// Returns the row's group-owned radio control.
  DensityRadio& radio() { return radio_; }

 private:
  DensityRadio radio_;
  TextLabel label_;
};

/// Display-density screen that owns the exclusive selection model.
class ControlDensity : public FlexLayout, public DensityGroup {
 public:
  /// Creates three density choices with Balanced initially selected.
  explicit ControlDensity(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Control density", material3::text_style_title_large()),
        guidance_(context, "Choose one spacing policy for controller screens",
                  material3::text_style_body_medium(), kTop | kLeft),
        feedback_(context, "Balanced spacing selected",
                  material3::text_style_title_small()),
        compact_(context, "Compact", *this, 0),
        balanced_(context, "Balanced", *this, 1),
        comfortable_(context, "Comfortable", *this, 2) {
    setPadding(Padding(Scaled(16), Scaled(12)));
    setGap(Scaled(8));
    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(guidance_, {.flex_grow = 0, .flex_shrink = 0});
    add(compact_, {.flex_grow = 0, .flex_shrink = 0});
    add(balanced_, {.flex_grow = 0, .flex_shrink = 0});
    add(comfortable_, {.flex_grow = 0, .flex_shrink = 0});
    add(feedback_, {.flex_grow = 0, .flex_shrink = 0});
    selectDensity(1);
  }

  /// Applies the exclusive state and publishes the selected density.
  void selectDensity(int index) override {
    compact_.radio().setOnOffState(
        index == 0 ? material3::RadioButton::OnOffState::kOn
                   : material3::RadioButton::OnOffState::kOff);
    balanced_.radio().setOnOffState(
        index == 1 ? material3::RadioButton::OnOffState::kOn
                   : material3::RadioButton::OnOffState::kOff);
    comfortable_.radio().setOnOffState(
        index == 2 ? material3::RadioButton::OnOffState::kOn
                   : material3::RadioButton::OnOffState::kOff);
    static constexpr const char* kMessages[] = {"Compact spacing selected",
                                                "Balanced spacing selected",
                                                "Comfortable spacing selected"};
    feedback_.setText(kMessages[index]);
  }

 private:
  TextLabel title_;
  TextBlock guidance_;
  TextLabel feedback_;
  DensityRow compact_;
  DensityRow balanced_;
  DensityRow comfortable_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
ControlDensity control_density(app.context());
SingletonActivity activity(app, control_density);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
