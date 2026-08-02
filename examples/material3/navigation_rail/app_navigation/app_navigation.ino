// Learning goal: use a persistent navigation rail on a wider device and bind
// its selection to visible application content. Tap a route or the menu icon.

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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class ActionIcon : public Icon {
 public:
  using Icon::Icon;
  bool isClickable() const override { return true; }

  // A point interaction layer is 40 dp wide. Reserve that full footprint so
  // the header's press overlay is not clipped against the rail's top edge.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(Scaled(40), Scaled(40));
  }
};

class PoolNavigationRail : public material3::NavigationRail {
 public:
  PoolNavigationRail(ApplicationContext& context, TextLabel& title,
                     TextBlock& detail)
      : material3::NavigationRail(context), title_(title), detail_(detail) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    static constexpr const char* kTitles[] = {"Pool status", "Alerts",
                                              "Energy"};
    static constexpr const char* kDetails[] = {"All systems operating normally",
                                               "2 maintenance reminders",
                                               "Solar supplied 68% today"};
    title_.setText(kTitles[new_index]);
    detail_.setText(kDetails[new_index]);
  }

 private:
  TextLabel& title_;
  TextBlock& detail_;
};

class EquipmentConsole : public FlexLayout {
 public:
  explicit EquipmentConsole(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kRow),
        content_(context, FlexDirection::kColumn),
        title_(context, "Pool status", material3::text_style_title_large()),
        detail_(context, "All systems operating normally",
                material3::text_style_body_large()),
        menu_feedback_(context, "Navigation remains visible beside content",
                       material3::text_style_body_medium()),
        menu_(context, ic_outlined_24_navigation_menu()),
        status_(context, "Status", &ic_outlined_24_navigation_home_work()),
        alerts_(context, "Alerts",
                &ic_outlined_24_action_circle_notifications()),
        energy_(context, "Energy", &ic_outlined_24_action_eco()),
        navigation_(context, title_, detail_) {
    // detail_.setWrapMode(TextWrapMode::kWordWrap);
    // menu_feedback_.setWrapMode(TextWrapMode::kWordWrap);
    content_.setPadding(Padding(Scaled(16), Scaled(20)));
    content_.setGap(Scaled(8));
    content_.add(title_);
    content_.add(detail_);
    content_.add(menu_feedback_);

    menu_.setOnInteractiveChange(
        [this]() { menu_feedback_.setText("Navigation menu requested"); });
    navigation_.setHeader(menu_);
    navigation_.add(status_);
    navigation_.add(alerts_);
    navigation_.add(energy_);
    alerts_.setBadgeValue(2);

    // Collapsed rails preserve content width on this 320 px example display.
    // Wider products can call setLayout(kExpanded) to show labels inline.
    // Keep the persistent rail at its token width when the changing page
    // labels make the content column's natural width larger than the display.
    add(navigation_, {.flex_shrink = 0});
    add(content_, {.flex_grow = 1});
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextBlock detail_;
  TextBlock menu_feedback_;
  ActionIcon menu_;
  material3::NavigationRailDestination status_;
  material3::BadgedNavigationRailDestination alerts_;
  material3::NavigationRailDestination energy_;
  PoolNavigationRail navigation_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
EquipmentConsole equipment_console(app.context());
SingletonActivity activity(app, equipment_console);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
