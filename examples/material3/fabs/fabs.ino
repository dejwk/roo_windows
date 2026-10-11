// Learning goal: keep a compose action fixed while the reading area scrolls,
// and use an icon FAB in a navigation-rail header to collapse that action.

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
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/navigation.h"
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

#include <memory>

#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/fab/fab.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

// The host decides when to collapse the extended action. AlignedLayout then
// measures it again and keeps its square footprint at the lower-right corner.
class HeaderToggle : public material3::FloatingActionButton {
 public:
  HeaderToggle(ApplicationContext& context,
               material3::ExtendedFloatingActionButton& target)
      : FloatingActionButton(context, ic_outlined_24_action_add_task()),
        target_(target) {}

  void onClicked() override { target_.setExpanded(!target_.expanded()); }

 private:
  material3::ExtendedFloatingActionButton& target_;
};

class ComposeAction : public material3::ExtendedFloatingActionButton {
 public:
  ComposeAction(ApplicationContext& context, TextLabel& status)
      : ExtendedFloatingActionButton(context, "Compose",
                                     ic_outlined_24_action_note_add()),
        status_(status) {}

  void onClicked() override { status_.setText("Draft created"); }

 private:
  TextLabel& status_;
};

class ReadingArea : public FlexLayout {
 public:
  ReadingArea(ApplicationContext& context, std::unique_ptr<TextLabel> status)
      : FlexLayout(context, FlexDirection::kColumn) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(14));
    add(std::move(status));
    static constexpr const char* kLines[] = {
        "Pump inspection",    "Filter replacement", "Water test",
        "Valve adjustment",   "Heater check",       "Pressure reading",
        "Chemical inventory", "Service notes",      "Next visit"};
    for (const char* line : kLines) {
      add(std::make_unique<TextLabel>(context, line,
                                      material3::text_style_body_large()));
    }
  }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

// The rail occupies its own column; the FAB is outside the scrolling subtree.
// The overlay layer can relayout the extended FAB without moving scroll
// content.
class ServiceScreen : public Panel {
 public:
  explicit ServiceScreen(ApplicationContext& context) : Panel(context) {
    auto status =
        std::make_unique<TextLabel>(context, "Tap Compose to start a note",
                                    material3::text_style_title_medium());
    TextLabel& status_ref = *status;
    auto content = std::make_unique<ReadingArea>(context, std::move(status));

    auto overlay = std::make_unique<AlignedLayout>(context);
    overlay->add(
        std::make_unique<SimpleScrollablePanel>(context, std::move(content)));
    auto compose = std::make_unique<ComposeAction>(context, status_ref);
    ComposeAction& compose_ref = *compose;
    overlay->add(std::move(compose),
                 (kRight | kBottom).shiftBy(-Scaled(16), -Scaled(16)));

    auto rail = std::make_unique<material3::NavigationRail>(context);
    rail->setHeader(std::make_unique<HeaderToggle>(context, compose_ref));
    rail->add(std::make_unique<material3::NavigationRailDestination>(
        context, "Notes", &ic_outlined_24_navigation_home_work()));
    add(std::move(rail), Rect(0, 0, 79, 239));
    add(std::move(overlay), Rect(80, 0, 319, 239));
  }
};

}  // namespace

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
ServiceScreen screen_content(app.context());
Task& task = app.addTaskFullScreen(screen_content);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
