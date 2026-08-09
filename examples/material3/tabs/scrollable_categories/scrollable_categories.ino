// Learning goal: use scrollable tabs when useful categories cannot fit in one
// fixed row. Drag the tabs horizontally, then select an equipment category.

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
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"
#include "roo_windows/widgets/text_block.h"

namespace {

/// Scrollable category tabs that update the visible equipment selection.
class EquipmentTabs : public material3::ScrollableTabs {
 public:
  /// Creates scrollable secondary tabs that update `selection`.
  EquipmentTabs(ApplicationContext& context, TextLabel& selection)
      : material3::ScrollableTabs(context, material3::TabsVariant::kSecondary),
        selection_(selection) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    static constexpr const char* kSelections[] = {
        "All equipment", "Circulation pumps", "Heating valves",
        "Water-quality sensors", "Lighting circuits"};
    selection_.setText(kSelections[new_index]);
  }

 private:
  TextLabel& selection_;
};

/// Equipment screen whose categories intentionally overflow the tab row.
class EquipmentCategories : public FlexLayout {
 public:
  /// Creates the equipment view and its five scrollable categories.
  explicit EquipmentCategories(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Equipment", material3::text_style_title_large()),
        selection_(context, "All equipment",
                   material3::text_style_headline_small()),
        help_(context, "Drag the tab row to reveal more categories",
              material3::text_style_body_medium(), kTop | kLeft),
        tabs_(context, selection_),
        all_(context, "All"),
        pumps_(context, "Circulation pumps"),
        valves_(context, "Heating valves"),
        sensors_(context, "Water-quality sensors"),
        lights_(context, "Lighting circuits") {
    setPadding(Padding(Scaled(12), Scaled(14)));
    setGap(Scaled(10));

    // ScrollableTabs retains the normal tab selection API while paying for
    // horizontal scrolling only in this overflow-oriented subclass.
    tabs_.addTab(all_);
    tabs_.addTab(pumps_);
    tabs_.addTab(valves_);
    tabs_.addTab(sensors_);
    tabs_.addTab(lights_);

    add(title_);
    add(tabs_);
    add(selection_);
    add(help_);
  }

 private:
  TextLabel title_;
  TextLabel selection_;
  TextBlock help_;
  EquipmentTabs tabs_;
  material3::Tab all_;
  material3::Tab pumps_;
  material3::Tab valves_;
  material3::Tab sensors_;
  material3::Tab lights_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
EquipmentCategories equipment_categories(app.context());
Task& task = app.addTaskFullScreen(equipment_categories);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
