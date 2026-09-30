// Learning goal: use a flexible top app bar to establish hierarchy on a
// detail screen and change its surface as equipment details scroll underneath.
// Scroll the body; return to the top to restore the flat surface.

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

#endif

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/button/navigation.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class ActionIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }
};

class HeatingDetails : public FlexLayout {
 public:
  explicit HeatingDetails(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        app_bar_(context, material3::AppBarVariant::kMediumFlexible),
        back_(context),
        more_(context, ic_outlined_24_navigation_more_vert()),
        scroller_(context),
        content_(context, FlexDirection::kColumn),
        status_(context, "Collector 41.8 °C",
                material3::text_style_title_medium()),
        feedback_(context, "Heating is adding 3.2 °C per hour",
                  material3::text_style_body_medium()),
        pump_(context, "Circulation pump: running",
              material3::text_style_body_large()),
        target_(context, "Target water temperature: 28 °C",
                material3::text_style_body_large()),
        schedule_(context, "Heating schedule: 08:00–18:00",
                  material3::text_style_body_large()) {
    // Flexible variants make room for both a hierarchical title and useful
    // supporting context. Use the large variant when even more emphasis is
    // appropriate; medium fits this landscape device comfortably.
    app_bar_.setTitle("Solar heating");
    app_bar_.setSubtitle("Equipment details");
    app_bar_.setLeading(back_);
    app_bar_.setTrailing(0, more_);

    more_.setOnInteractiveChange(
        [this]() { feedback_.setText("More equipment actions requested"); });

    content_.setPadding(Padding(Scaled(16), Scaled(10)));
    content_.setGap(Scaled(6));
    content_.add(status_);
    content_.add(feedback_);
    content_.add(pump_);
    content_.add(target_);
    content_.add(schedule_);
    scroller_.setContents(content_);

    // The bar stays outside the scrolling viewport. Content origins become
    // negative when scrolling down; positive top overscroll stays flat.
    // The initial origin is zero, matching the bar's default flat surface.
    scroller_.setOnScrollPositionChanged([this](ScrollPosition,
                                                ScrollPosition current) {
      app_bar_.setSurfaceState(current.y < 0
                                   ? material3::AppBarSurfaceState::kScrolled
                                   : material3::AppBarSurfaceState::kFlat);
    });

    add(app_bar_);
    add(scroller_, {.flex_grow = 1});
  }

  // The task handles Back here because this standalone demo has one screen.
  BackResult onBackRequested(BackSource) {
    feedback_.setText("Back to pool overview");
    return BackResult::kHandled;
  }

 private:
  material3::AppBar app_bar_;
  material3::BackButton back_;
  ActionIcon more_;
  ScrollablePanel scroller_;
  FlexLayout content_;
  TextLabel status_;
  TextLabel feedback_;
  TextLabel pump_;
  TextLabel target_;
  TextLabel schedule_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
HeatingDetails heating_details(app.context());
Task& task = app.addTaskFullScreen(heating_details);

void setup() {
  initDisplay();
  // BackButton and hardware Back share the same semantic request handler.
  task.setBackCallback([](BackSource source) {
    return heating_details.onBackRequested(source);
  });
  app.start();
  scheduler.run();
}

void loop() {}
