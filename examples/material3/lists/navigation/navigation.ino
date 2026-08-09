// Learning goal: make list rows invokable and publish the selected equipment
// destination as visible application state.

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
#include "roo_icons/outlined/24/notification.h"
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

class FullWidthColumn : public FlexLayout {
 public:
  explicit FullWidthColumn(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {}

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

/// Equipment list whose invokable rows update adjacent screen state.
class EquipmentNavigation : public SimpleScrollablePanel {
 public:
  /// Creates two destinations and binds each item's invocation callback.
  explicit EquipmentNavigation(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context),
        title_(context, "Equipment", material3::text_style_title_large()),
        feedback_(context, "Select an item to open its details",
                  material3::text_style_body_medium()),
        equipment_(context),
        next_task_(context, ic_outlined_24_notification_sync(), "Next task",
                   "Backwash filter · tomorrow at 08:00"),
        owner_(context, "DW", "Service contact", "Dawid · pool technician") {
    content_.setPadding(Padding(Scaled(16), Scaled(12)));
    content_.setGap(Scaled(10));

    // Invokable item types add the click target and trailing affordance. The
    // row forwards both touch and keyboard invocation to the same callback.
    next_task_.item().setOnInvoked(
        [this]() { feedback_.setText("Opened tomorrow's maintenance task"); });
    owner_.item().setOnInvoked(
        [this]() { feedback_.setText("Opened service contact details"); });

    equipment_.add(next_task_);
    equipment_.add(owner_);

    content_.add(title_);
    content_.add(feedback_);
    content_.add(equipment_);
    setContents(content_);
  }

 private:
  FullWidthColumn content_;
  TextLabel title_;
  TextLabel feedback_;
  material3::List equipment_;
  material3::ListRow<material3::NavigationListItem> next_task_;
  material3::ListRow<material3::AvatarNavigationListItem> owner_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
EquipmentNavigation equipment_navigation(app.context());
UiTask& task = app.addUiTaskFullScreen(equipment_navigation);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
