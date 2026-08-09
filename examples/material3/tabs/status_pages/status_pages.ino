// Learning goal: bind primary tabs to swipeable application pages. Tap a tab
// or swipe the page area; both interactions keep selection synchronized.

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
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class StatusPages;

/// Primary tabs that send selected indices to a bound page host.
class StatusTabs : public material3::Tabs {
 public:
  /// Creates an initially unbound primary tab row.
  explicit StatusTabs(ApplicationContext& context)
      : material3::Tabs(context), pages_(nullptr) {}

  /// Binds the page host that follows tab selection.
  void bind(StatusPages& pages) { pages_ = &pages; }

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  StatusPages* pages_;
};

/// Page host that sends swipe targets back to its bound primary tabs.
class StatusPages : public HorizontalPageHost {
 public:
  /// Creates an initially unbound horizontal page host.
  explicit StatusPages(ApplicationContext& context)
      : HorizontalPageHost(context), tabs_(nullptr) {}

  /// Binds the tab row that follows page swipe targets.
  void bind(StatusTabs& tabs) { tabs_ = &tabs; }

 protected:
  // targetIndex changes as soon as a swipe commits to a likely page, so the
  // tab indicator can respond before the page's settle animation completes.
  void onTargetIndexChanged(int old_index, int new_index) override {
    (void)old_index;
    if (tabs_ != nullptr && tabs_->selectedIndex() != new_index) {
      tabs_->setSelectedIndex(new_index, true);
    }
  }

 private:
  StatusTabs* tabs_;
};

void StatusTabs::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  if (pages_ != nullptr && pages_->targetIndex() != new_index) {
    pages_->setCurrentIndex(new_index, true);
  }
}

/// Pool status screen with synchronized tabs and swipeable pages.
class PoolStatus : public FlexLayout {
 public:
  /// Creates the three tabs, matching pages, and bidirectional binding.
  explicit PoolStatus(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        tabs_(context),
        overview_tab_(context, "Overview"),
        heating_tab_(context, "Heating"),
        history_tab_(context, "History"),
        pages_(context),
        overview_(context, "Water 27.4°C · Pump on",
                  material3::text_style_title_large()),
        heating_(context, "Solar collector 41.8°C",
                 material3::text_style_title_large()),
        history_(context, "Today: 25.1°C to 27.6°C",
                 material3::text_style_title_large()) {
    tabs_.bind(pages_);
    pages_.bind(tabs_);

    overview_tab_.setIcon(&ic_outlined_24_action_dashboard());
    heating_tab_.setIcon(&ic_outlined_24_action_eco());
    history_tab_.setIcon(&ic_outlined_24_action_history());
    tabs_.addTab(overview_tab_);
    tabs_.addTab(heating_tab_);
    tabs_.addTab(history_tab_);

    // Page order must match tab order because the callbacks share indices.
    pages_.addPage(overview_);
    pages_.addPage(heating_);
    pages_.addPage(history_);

    // Keep page content away from the screen and tab-row edges.
    overview_.setPadding(PaddingSize::kLarge);
    heating_.setPadding(PaddingSize::kLarge);
    history_.setPadding(PaddingSize::kLarge);

    add(tabs_);
    add(pages_, {.flex_grow = 1});
  }

 private:
  StatusTabs tabs_;
  material3::Tab overview_tab_;
  material3::Tab heating_tab_;
  material3::Tab history_tab_;
  StatusPages pages_;
  TextLabel overview_;
  TextLabel heating_;
  TextLabel history_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
PoolStatus pool_status(app.context());
Task& task = app.addTaskFullScreen(pool_status);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
