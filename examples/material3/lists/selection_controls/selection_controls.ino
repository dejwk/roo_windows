// Learning goal: choose the list selection policy that matches independent,
// exclusive, or boolean settings and let each row invoke its control.

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
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "../list_example_layout.h"

namespace {

/// Scrollable preferences screen demonstrating the three selection models.
class SelectionControls : public SimpleScrollablePanel {
 public:
  /// Creates checkbox, radio, and switch lists with row-wide interaction.
  explicit SelectionControls(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context),
        title_(context, "Control preferences",
               material3::text_style_title_large()),
        alarms_label_(context, "Alarm sources",
                      material3::text_style_title_small()),
        alarms_(context),
        freeze_(context, "Freeze risk", "Independent alarm", true,
                material3::AffordancePlacement::kLeading),
        flow_(context, "Low flow", "Independent alarm", false,
              material3::AffordancePlacement::kLeading),
        density_label_(context, "Display density",
                       material3::text_style_title_small()),
        density_(context),
        compact_(context, "Compact", "More rows", false,
                 material3::AffordancePlacement::kLeading),
        comfortable_(context, "Comfortable", "Larger targets", true,
                     material3::AffordancePlacement::kLeading),
        automation_label_(context, "Automation",
                          material3::text_style_title_small()),
        automation_(context),
        thermal_lock_(context, "Thermal lock", "Protect setpoint", true),
        solar_boost_(context, "Solar boost", "Prefer roof heat", false) {
    content_.setPadding(Padding(Scaled(16), Scaled(12)));
    content_.setGap(Scaled(8));

    material3::ListSelectionPolicy multiple;
    multiple.mode = material3::SelectionMode::kMultiple;
    multiple.affordance = material3::SelectionAffordance::kCheckbox;
    multiple.placement = material3::AffordancePlacement::kLeading;
    alarms_.setSelectionPolicy(multiple);

    material3::ListSelectionPolicy single;
    single.mode = material3::SelectionMode::kSingle;
    single.affordance = material3::SelectionAffordance::kRadio;
    single.placement = material3::AffordancePlacement::kLeading;
    density_.setSelectionPolicy(single);

    material3::ListSelectionPolicy switches;
    switches.mode = material3::SelectionMode::kMultiple;
    switches.affordance = material3::SelectionAffordance::kSwitch;
    automation_.setSelectionPolicy(switches);

    // Radio rows need application state to enforce exactly one selection.
    compact_.item().setOnInvoked([this]() { selectDensity(false); });
    comfortable_.item().setOnInvoked([this]() { selectDensity(true); });

    alarms_.add(freeze_);
    alarms_.add(flow_);
    density_.add(compact_);
    density_.add(comfortable_);
    automation_.add(thermal_lock_);
    automation_.add(solar_boost_);

    content_.add(title_);
    content_.add(alarms_label_);
    content_.add(alarms_);
    content_.add(density_label_);
    content_.add(density_);
    content_.add(automation_label_);
    content_.add(automation_);
    setContents(content_);
  }

 private:
  // Updates both radio items so exactly one density remains selected.
  void selectDensity(bool comfortable) {
    compact_.item().setSelected(!comfortable);
    comfortable_.item().setSelected(comfortable);
    compact_.refreshFromItem();
    comfortable_.refreshFromItem();
  }

  material3_examples::FullWidthColumn content_;
  TextLabel title_;
  TextLabel alarms_label_;
  material3::List alarms_;
  material3::ListRow<material3::CheckboxListItem> freeze_;
  material3::ListRow<material3::CheckboxListItem> flow_;
  TextLabel density_label_;
  material3::List density_;
  material3::ListRow<material3::RadioListItem> compact_;
  material3::ListRow<material3::RadioListItem> comfortable_;
  TextLabel automation_label_;
  material3::List automation_;
  material3::ListRow<material3::SwitchListItem> thermal_lock_;
  material3::ListRow<material3::SwitchListItem> solar_boost_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
SelectionControls selection_controls(app.context());
SingletonActivity activity(app, selection_controls);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
