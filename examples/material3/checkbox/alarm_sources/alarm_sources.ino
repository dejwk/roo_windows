// Learning goal: model independent alarm sources with checkboxes. Each source
// can be enabled or disabled without changing the other selections.

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
#include "roo_windows/material3/checkbox/checkbox.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

/// Checkbox that reports its independent state after the base toggle runs.
class AlarmCheckbox : public material3::Checkbox {
 public:
  /// Creates a checkbox for the named alarm source.
  AlarmCheckbox(ApplicationContext& context, OnOffState state,
                roo::string_view source, TextLabel& feedback)
      : material3::Checkbox(context, state),
        source_(source),
        feedback_(feedback) {}

  /// Lets the base widget toggle, then publishes the new independent state.
  void onClicked() override {
    material3::Checkbox::onClicked();
    feedback_.setTextf("%.*s alarm %s", static_cast<int>(source_.size()),
                       source_.data(), isOn() ? "enabled" : "disabled");
  }

 private:
  roo::string_view source_;
  TextLabel& feedback_;
};

/// One labeled alarm source with a trailing independent checkbox.
class AlarmRow : public FlexLayout {
 public:
  /// Creates an alarm row with its initial selection state.
  AlarmRow(ApplicationContext& context, roo::string_view label,
           material3::Checkbox::OnOffState state, TextLabel& feedback)
      : FlexLayout(context, FlexDirection::kRow),
        label_(context, std::string(label), material3::text_style_body_large()),
        checkbox_(context, state, label, feedback) {
    setAlignItems(AlignItems::kCenter);
    setGap(Scaled(12));
    add(label_, {.flex_grow = 1, .flex_shrink = 1});
    add(checkbox_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel label_;
  AlarmCheckbox checkbox_;
};

/// Alarm configuration screen with three independent sources.
class AlarmSources : public FlexLayout {
 public:
  /// Creates the alarm-source choices and visible state feedback.
  explicit AlarmSources(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Alarm sources", material3::text_style_title_large()),
        guidance_(context, "Select every condition that should raise an alarm",
                  material3::text_style_body_medium()),
        feedback_(context, "Freeze and flow alarms enabled",
                  material3::text_style_title_small()),
        freeze_(context, "Freeze risk", material3::Checkbox::OnOffState::kOn,
                feedback_),
        flow_(context, "Low flow", material3::Checkbox::OnOffState::kOn,
              feedback_),
        chemistry_(context, "Chemistry drift",
                   material3::Checkbox::OnOffState::kOff, feedback_) {
    setPadding(Padding(Scaled(16), Scaled(12)));
    setGap(Scaled(8));
    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(guidance_, {.flex_grow = 0, .flex_shrink = 0});
    add(freeze_, {.flex_grow = 0, .flex_shrink = 0});
    add(flow_, {.flex_grow = 0, .flex_shrink = 0});
    add(chemistry_, {.flex_grow = 0, .flex_shrink = 0});
    add(feedback_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel guidance_;
  TextLabel feedback_;
  AlarmRow freeze_;
  AlarmRow flow_;
  AlarmRow chemistry_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
AlarmSources alarm_sources(app.context());
SingletonActivity activity(app, alarm_sources);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
