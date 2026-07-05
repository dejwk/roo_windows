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
  FlexViewport flexViewport;

  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flexViewport(viewport, 1, FlexViewport::kRotationRight),
        display(flexViewport),
        touch(flexViewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
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
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

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
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class DemoPageHost;

class DemoTabs : public material3::Tabs {
 public:
  explicit DemoTabs(ApplicationContext& context)
      : material3::Tabs(context, material3::TabsVariant::kPrimary),
        pages_(nullptr) {}

  void bind(DemoPageHost& pages) { pages_ = &pages; }

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  DemoPageHost* pages_;
};

class DemoPageHost : public HorizontalPageHost {
 public:
  explicit DemoPageHost(ApplicationContext& context)
      : HorizontalPageHost(context), tabs_(nullptr) {}

  void bind(DemoTabs& tabs) { tabs_ = &tabs; }

 protected:
  void onSettledIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    if (tabs_ != nullptr && tabs_->selectedIndex() != new_index) {
      tabs_->setSelectedIndex(new_index, true);
    }
  }

 private:
  DemoTabs* tabs_;
};

void DemoTabs::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  if (pages_ != nullptr && pages_->currentIndex() != new_index) {
    pages_->setCurrentIndex(new_index, true);
  }
}

class TabsScreen : public FlexLayout {
 public:
  explicit TabsScreen(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        pages_(context),
        tabs_(context),
        secondary_tabs_(context, material3::TabsVariant::kSecondary),
        overview_tab_(context, "Overview"),
        heating_tab_(context, "Heating"),
        history_tab_(context, "History"),
        today_tab_(context, "Today"),
        week_tab_(context, "Week"),
        month_tab_(context, "Month"),
        scroll_overview_tab_(context, "Overview"),
        scroll_pump_tab_(context, "Pump schedule"),
        scroll_chemistry_tab_(context, "Chemistry"),
        scroll_weather_tab_(context, "Weather forecast"),
        scroll_history_tab_(context, "Temperature history"),
        scrollable_tabs_(context, material3::TabsVariant::kSecondary),
        overview_(context, "Pool status overview", font_h6()),
        heating_(context, "Solar heating controls", font_h6()),
        history_(context, "Recent temperature history", font_h6()) {
    setPadding(Padding(Scaled(12), Scaled(8)));
    setGap(Scaled(8));

    tabs_.bind(pages_);
    pages_.bind(tabs_);

    heating_tab_.setIcon(&ic_outlined_24_action_done());

    tabs_.addTab(overview_tab_);
    tabs_.addTab(heating_tab_);
    tabs_.addTab(history_tab_);

    secondary_tabs_.addTab(today_tab_);
    secondary_tabs_.addTab(week_tab_);
    secondary_tabs_.addTab(month_tab_);
    secondary_tabs_.setSelectedIndex(1, false);

    scrollable_tabs_.addTab(scroll_overview_tab_);
    scrollable_tabs_.addTab(scroll_pump_tab_);
    scrollable_tabs_.addTab(scroll_chemistry_tab_);
    scrollable_tabs_.addTab(scroll_weather_tab_);
    scrollable_tabs_.addTab(scroll_history_tab_);

    pages_.addPage(overview_);
    pages_.addPage(heating_);
    pages_.addPage(history_);

    add(tabs_, {.flex_grow = 0, .flex_shrink = 0});
    add(pages_, {.flex_grow = 1, .flex_shrink = 1});
    add(secondary_tabs_, {.flex_grow = 0, .flex_shrink = 0});
    add(scrollable_tabs_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  DemoPageHost pages_;
  DemoTabs tabs_;
  material3::Tabs secondary_tabs_;
  material3::Tab overview_tab_;
  material3::Tab heating_tab_;
  material3::Tab history_tab_;
  material3::Tab today_tab_;
  material3::Tab week_tab_;
  material3::Tab month_tab_;
  material3::Tab scroll_overview_tab_;
  material3::Tab scroll_pump_tab_;
  material3::Tab scroll_chemistry_tab_;
  material3::Tab scroll_weather_tab_;
  material3::Tab scroll_history_tab_;
  material3::ScrollableTabs scrollable_tabs_;
  TextLabel overview_;
  TextLabel heating_;
  TextLabel history_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
TabsScreen tabs_screen(app.context());
SingletonActivity activity(app, tabs_screen);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
