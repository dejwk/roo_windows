// Learning goal: keep expansion state inside an invokable list item and use
// ExpandablePanel as measured, animated body content below its row.

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

#include <string>

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

namespace {

/// Invokable maintenance item that owns its expandable details panel.
class MaintenanceItem : public material3::InvokableListItemBase {
 public:
  /// Creates a collapsed item with explanatory body text.
  MaintenanceItem(ApplicationContext& context, roo::string_view headline,
                  roo::string_view supporting, roo::string_view details)
      : material3::InvokableListItemBase(headline, supporting, {}, {}, true),
        details_(context, std::string(details),
                 material3::text_style_body_medium()),
        details_panel_(context) {
    details_panel_.setAnimationDuration(180);
    details_panel_.setExpanded(false, false);
    details_panel_.setContent(WidgetRef(details_));
  }

  /// Returns the optional content rendered below the main list row.
  Widget* body() override { return &details_panel_; }

 protected:
  /// Toggles the item's owned body through the shared invocation path.
  void handleInvoke() override {
    details_panel_.setExpanded(!details_panel_.isExpanded());
  }

 private:
  TextLabel details_;
  material3::ExpandablePanel details_panel_;
};

/// Maintenance plan with independently expandable tasks.
class MaintenancePlan : public SimpleScrollablePanel {
 public:
  /// Creates collapsed filter and chemistry tasks.
  explicit MaintenancePlan(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Maintenance plan",
               material3::text_style_title_large()),
        guidance_(context, "Select a task to reveal its instructions",
                  material3::text_style_body_medium()),
        tasks_(context),
        filter_(context, "Backwash filter", "Due tomorrow",
                "Run the pump for 2 minutes.\n"
                "Rinse for 30 seconds, then restore Filter mode."),
        chemistry_(context, "Balance chemistry", "Retest this evening",
                   "pH target: 7.4\nChlorine target: 2.0 ppm") {
    content_.setPadding(Padding(Scaled(16), Scaled(12)));
    content_.setGap(Scaled(10));
    tasks_.add(filter_);
    tasks_.add(chemistry_);
    content_.add(title_);
    content_.add(guidance_);
    content_.add(tasks_);
    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextLabel guidance_;
  material3::List tasks_;
  material3::ListRow<MaintenanceItem> filter_;
  material3::ListRow<MaintenanceItem> chemistry_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
MaintenancePlan maintenance_plan(app.context());
SingletonActivity activity(app, maintenance_plan);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
