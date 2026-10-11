// Learning goal: put chart actions in a docked toolbar and move extra actions
// into an explicit anchored menu. The sample trace stands in for sensor data.

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

#include "roo_icons/outlined/24/av.h"
#include "roo_icons/outlined/24/content.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_windows/containers/static_layout.h"
#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/toolbar/toolbar.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {

// Replace these fixed samples with a rolling sensor buffer in an application.
class ChartPreview : public SurfaceWidget {
 public:
  explicit ChartPreview(ApplicationContext& context) : SurfaceWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(288, 104);
  }

  Color background() const override {
    return theme().material3Theme().color.surfaceContainerLow;
  }

  void markSnapshot() {
    snapshot_marked_ = true;
    invalidateInterior();
  }

  void paint(PaintContext& ctx) const override {
    static constexpr uint8_t kSamples[] = {30, 36, 33, 42, 48, 44, 51, 57,
                                           52, 60, 66, 61, 69, 72, 65, 75,
                                           79, 73, 82, 78, 84, 88, 82, 91};
    const Color& primary = theme().material3Theme().color.primary;
    const Color& grid = theme().material3Theme().color.outlineVariant;
    ctx.clear();
    for (int y : {height() / 3, 2 * height() / 3}) {
      ctx.fillRect(Rect(8, y, width() - 9, y), grid);
    }
    const int step = (width() - 24) / 24;
    for (int i = 0; i < 24; ++i) {
      const int x = 12 + i * step;
      const int bar_height = kSamples[i] * (height() - 16) / 100;
      ctx.fillRect(Rect(x, height() - 8 - bar_height, x + 3, height() - 8),
                   primary);
    }
    if (snapshot_marked_) {
      const int marker_x = 12 + 23 * step + 1;
      ctx.fillRect(Rect(marker_x, 8, marker_x + 1, height() - 8),
                   theme().material3Theme().color.tertiary);
    }
  }

 private:
  bool snapshot_marked_ = false;
};

class ChartMenuItem : public material3::StandardMenuItem {
 public:
  ChartMenuItem(roo_display::StringView label, TextLabel& status)
      : StandardMenuItem(material3::StandardMenuItemInit{label, {}}),
        status_(status) {}

  void onInvoked() override { status_.setText("Demo trace · 24 samples"); }

 private:
  TextLabel& status_;
};

class ChartPage : public material3::LayoutScaffold {
 public:
  explicit ChartPage(ApplicationContext& context)
      : LayoutScaffold(context),
        status_(context, "Live readings · last 10 min",
                material3::text_style_body_large()),
        chart_(context),
        body_(context),
        details_item_("Sensor details", status_),
        details_row_(context),
        menu_group_(context),
        menu_(context) {
    body_.add(status_, 16, 12);
    body_.add(chart_, 16, 48);
    setBody(body_);
    details_row_.setMenuItem(details_item_);
    menu_group_.add(details_row_);
    menu_.addGroup(menu_group_);

    auto toolbar = std::make_unique<material3::DockedToolbar>(context);
    toolbar->setArrangement(material3::DockedToolbarArrangement::kEdgeAnchored);

    // The selected state is the paused state, so the icon changes to Play.
    auto pause = std::make_unique<material3::ToggleIconButton>(
        context, ic_outlined_24_av_pause(), &ic_outlined_24_av_play_arrow(),
        material3::IconButtonStyle::kFilledTonal);
    material3::ToggleIconButton* pause_button = pause.get();
    pause->setOnInteractiveChange([this, pause_button] {
      status_.setText(pause_button->isSelected()
                          ? "Readings paused"
                          : "Live readings · last 10 min");
    });
    toolbar->add(std::move(pause), material3::ToolbarSlot::kLeading);

    // The demo marks the newest reading; connect this action to storage when
    // adapting the sketch to a device.
    auto save = std::make_unique<material3::IconButton>(
        context, ic_outlined_24_content_save(),
        material3::IconButtonStyle::kFilledTonal);
    save->setOnInteractiveChange([this] {
      chart_.markSnapshot();
      status_.setText("Snapshot marked");
    });
    toolbar->add(std::move(save), material3::ToolbarSlot::kCenter);

    auto more = std::make_unique<material3::IconButton>(
        context, ic_outlined_24_navigation_more_vert(),
        material3::IconButtonStyle::kFilledTonal);
    material3::IconButton* anchor = more.get();
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
  ChartPreview chart_;
  StaticLayout body_;
  ChartMenuItem details_item_;
  material3::MenuEntry details_row_;
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
