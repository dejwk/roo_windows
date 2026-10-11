// Learning goal: select drawing tools in a floating toolbar beside a separate
// clear-canvas FAB. Tap the open canvas below the toolbar.

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

#include <memory>

#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/content.h"
#include "roo_icons/outlined/24/editor.h"
#include "roo_windows/containers/static_layout.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/fab/fab.h"
#include "roo_windows/material3/toolbar/toolbar.h"

namespace {

// The fixed point buffer avoids heap work while drawing on a small board.
class DrawingCanvas : public SurfaceWidget {
 public:
  explicit DrawingCanvas(ApplicationContext& context)
      : SurfaceWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(320, 240);
  }
  Color background() const override {
    return theme().material3Theme().color.surface;
  }
  bool isClickable() const override { return true; }
  void setErase(bool erase) { erase_ = erase; }
  void clear() {
    count_ = 0;
    invalidateInterior();
  }

  void onSingleTapUp(XDim x, YDim y) override {
    SurfaceWidget::onSingleTapUp(x, y);
    if (erase_) {
      for (int i = count_ - 1; i >= 0; --i) {
        if (abs(points_[i].x - x) > Scaled(12) ||
            abs(points_[i].y - y) > Scaled(12))
          continue;
        for (int j = i; j + 1 < count_; ++j) points_[j] = points_[j + 1];
        --count_;
        invalidateInterior();
        return;
      }
    } else if (count_ < kMaxPoints) {
      points_[count_++] = {x, y};
      invalidateInterior();
    }
  }

  void paint(PaintContext& ctx) const override {
    for (int i = 0; i < count_; ++i) {
      Rect mark(points_[i].x - 3, points_[i].y - 3, points_[i].x + 3,
                points_[i].y + 3);
      ctx.fillRect(mark, theme().material3Theme().color.primary);
      ctx.addExclusion(mark);
    }
    ctx.clear();
  }

 private:
  static constexpr int kMaxPoints = 24;
  struct Point {
    XDim x;
    YDim y;
  };
  Point points_[kMaxPoints] = {};
  uint8_t count_ = 0;
  bool erase_ = false;
};

}  // namespace

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
Application app(&env, display);
StaticLayout drawing_page(app.context());
Task& task = app.addTaskFullScreen(drawing_page);

void setup() {
  initDisplay();
  auto canvas = std::make_unique<DrawingCanvas>(app.context());
  DrawingCanvas* drawing = canvas.get();
  drawing_page.add(std::move(canvas), Rect(0, 0, 319, 239));

  auto tools = std::make_unique<material3::FloatingToolbar>(app.context());
  auto pen = std::make_unique<material3::ToggleIconButton>(
      app.context(), ic_outlined_24_editor_mode_edit(), nullptr,
      material3::IconButtonStyle::kFilledTonal, true);
  material3::ToggleIconButton* pen_button = pen.get();
  auto erase = std::make_unique<material3::ToggleIconButton>(
      app.context(), ic_outlined_24_content_backspace(), nullptr,
      material3::IconButtonStyle::kFilledTonal);
  material3::ToggleIconButton* erase_button = erase.get();

  // Each toggle owns its appearance; the application keeps exactly one tool
  // selected and updates the canvas only after the button's own click.
  pen->setOnInteractiveChange([drawing, pen_button, erase_button] {
    pen_button->setSelected(true);
    erase_button->setSelected(false);
    drawing->setErase(false);
  });
  erase->setOnInteractiveChange([drawing, pen_button, erase_button] {
    erase_button->setSelected(true);
    pen_button->setSelected(false);
    drawing->setErase(true);
  });
  tools->add(std::move(pen));
  tools->add(std::move(erase));
  // The application owns placement and reserves sixteen pixels around the
  // toolbar, leaving the drawing region below its shadow unobstructed.
  drawing_page.add(std::move(tools), 16, 16);

  auto clear = std::make_unique<material3::FloatingActionButton>(
      app.context(), ic_outlined_24_action_delete());
  clear->setOnInteractiveChange([drawing] { drawing->clear(); });
  // A FAB is a sibling. It does not occupy a toolbar slot.
  drawing_page.add(std::move(clear), 248, 168);
  app.start();
  scheduler.run();
}

void loop() {}
