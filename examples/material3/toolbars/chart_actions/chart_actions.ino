// Learning goal: put chart actions in a docked toolbar and move extra actions
// into an explicit anchored menu. Keep the action set fitting the display.

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
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

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

#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/toolbar/toolbar.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class ChartMenuItem : public material3::StandardMenuItem {
 public:
  ChartMenuItem(roo_display::StringView label, TextLabel& status)
      : StandardMenuItem(material3::StandardMenuItemInit{label, {}}),
        status_(status) {}

  void onInvoked() override { status_.setText("Chart range reset"); }

 private:
  TextLabel& status_;
};

class ChartPage : public material3::LayoutScaffold {
 public:
  explicit ChartPage(ApplicationContext& context)
      : LayoutScaffold(context),
        status_(context, "Live chart: last 10 minutes",
                material3::text_style_body_large()),
        reset_item_("Reset zoom", status_),
        reset_row_(context),
        menu_group_(context),
        menu_(context) {
    setBody(status_);
    reset_row_.setMenuItem(reset_item_);
    menu_group_.add(reset_row_);
    menu_.addGroup(menu_group_);

    auto toolbar = std::make_unique<material3::DockedToolbar>(context);
    toolbar->setArrangement(material3::DockedToolbarArrangement::kEdgeAnchored);
    toolbar->setShowGroupDividers(true);

    auto pause = std::make_unique<material3::Button>(
        context, "Pause", material3::ButtonVariant::kFilledTonal);
    pause->setOnInteractiveChange([this] { status_.setText("Chart paused"); });
    toolbar->add(std::move(pause), material3::ToolbarSlot::kLeading);

    auto save = std::make_unique<material3::Button>(
        context, "Save", material3::ButtonVariant::kFilledTonal);
    save->setOnInteractiveChange([this] { status_.setText("Snapshot saved"); });
    toolbar->add(std::move(save), material3::ToolbarSlot::kCenter);

    auto more = std::make_unique<material3::Button>(
        context, "More", material3::ButtonVariant::kFilledTonal);
    material3::Button* anchor = more.get();
    more->setOnInteractiveChange([this, anchor] {
      if (getTask() != nullptr) {
        menu_.show(*getTask(), *anchor, material3::MenuPlacement::kAboveEnd);
      }
    });
    toolbar->add(std::move(more), material3::ToolbarSlot::kTrailing);

    // The scaffold reserves a bottom band. The menu is kept on this page so
    // it outlives repeated presentations from the toolbar's stable anchor.
    setBottomBar(std::move(toolbar));
  }

  ~ChartPage() override {
    clearBottomBar();
    setBody(WidgetRef());
  }

 private:
  TextLabel status_;
  ChartMenuItem reset_item_;
  material3::MenuEntry reset_row_;
  material3::MenuGroup menu_group_;
  material3::Menu menu_;
};

}  // namespace

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
Application app(&env, display);
ChartPage chart_page(app.context());
Task& task = app.addTaskFullScreen(chart_page);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
