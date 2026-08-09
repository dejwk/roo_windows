// Learning goal: use LayoutScaffold breakpoint slots to present bottom
// navigation on compact/medium screens and a navigation rail on expanded
// screens. Both controls update the same visible application state.

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

#include <memory>

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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {
class AdaptivePoolController;

class CompactNavigation : public material3::NavigationBar {
 public:
  CompactNavigation(ApplicationContext& context, AdaptivePoolController& owner)
      : material3::NavigationBar(context), owner_(owner) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  AdaptivePoolController& owner_;
};

class ExpandedNavigation : public material3::NavigationRail {
 public:
  ExpandedNavigation(ApplicationContext& context, AdaptivePoolController& owner)
      : material3::NavigationRail(context), owner_(owner) {}

 protected:
  void onSelectedIndexChanged(int old_index, int new_index) override;

 private:
  AdaptivePoolController& owner_;
};

/// Pool controller shell that changes navigation chrome at width breakpoints.
class AdaptivePoolController : public material3::LayoutScaffold {
 public:
  explicit AdaptivePoolController(ApplicationContext& context)
      : material3::LayoutScaffold(context),
        title_(nullptr),
        detail_(nullptr),
        compact_navigation_(nullptr),
        expanded_navigation_(nullptr) {
    auto top_bar = std::make_unique<material3::AppBar>(context);
    top_bar->setTitle("Pool controller");
    setTopBar(WidgetRef(std::move(top_bar)));
    auto body = std::make_unique<FlexLayout>(context, FlexDirection::kColumn);
    body->setPadding(Padding(Scaled(16), Scaled(16)));
    body->setGap(Scaled(8));
    auto title = std::make_unique<TextLabel>(
        context, "Pool status", material3::text_style_title_large());
    title_ = title.get();
    auto detail =
        std::make_unique<TextLabel>(context, "Water 27.4 °C · Pump running",
                                    material3::text_style_body_large());
    detail_ = detail.get();
    body->add(WidgetRef(std::move(title)));
    body->add(WidgetRef(std::move(detail)));
    setBody(WidgetRef(std::move(body)));

    auto bar = std::make_unique<CompactNavigation>(context, *this);
    compact_navigation_ = bar.get();
    bar->add(WidgetRef(std::make_unique<material3::NavigationBarDestination>(
        context, "Status", &ic_outlined_24_navigation_home_work())));
    bar->add(WidgetRef(std::make_unique<material3::NavigationBarDestination>(
        context, "Schedule", &ic_outlined_24_action_calendar_month())));
    setBottomBar(WidgetRef(std::move(bar)),
                 {material3::LayoutBreakpoint::kCompact,
                  material3::LayoutBreakpoint::kMedium});

    auto rail = std::make_unique<ExpandedNavigation>(context, *this);
    expanded_navigation_ = rail.get();
    rail->setLayout(material3::NavigationRailLayout::kExpanded);
    rail->add(WidgetRef(std::make_unique<material3::NavigationRailDestination>(
        context, "Status", &ic_outlined_24_navigation_home_work())));
    rail->add(WidgetRef(std::make_unique<material3::NavigationRailDestination>(
        context, "Schedule", &ic_outlined_24_action_calendar_month())));
    // The rail slot defaults to expanded through extra-large breakpoints.
    setLeadingRail(WidgetRef(std::move(rail)));
    selectDestination(0);
  }

  void selectDestination(int index) {
    static constexpr const char* kTitles[] = {"Pool status", "Schedule"};
    static constexpr const char* kDetails[] = {"Water 27.4 °C · Pump running",
                                               "Next pump cycle at 18:30"};
    title_->setText(kTitles[index]);
    detail_->setText(kDetails[index]);
    // Keep the hidden control synchronized so resizing preserves selection.
    if (compact_navigation_->selectedIndex() != index) {
      compact_navigation_->setSelectedIndex(index);
    }
    if (expanded_navigation_->selectedIndex() != index) {
      expanded_navigation_->setSelectedIndex(index);
    }
  }

 private:
  TextLabel* title_;
  TextLabel* detail_;
  CompactNavigation* compact_navigation_;
  ExpandedNavigation* expanded_navigation_;
};

void CompactNavigation::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  owner_.selectDestination(new_index);
}
void ExpandedNavigation::onSelectedIndexChanged(int old_index, int new_index) {
  (void)old_index;
  owner_.selectDestination(new_index);
}
}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
AdaptivePoolController pool_controller(app.context());
UiTask& task = app.addUiTaskFullScreen(pool_controller);
void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
