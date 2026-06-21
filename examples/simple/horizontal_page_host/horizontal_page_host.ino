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
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Select the driver to match your display device.
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

// Set your configuration for the driver.
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
  display.init();
}

// *************** EXAMPLE STARTS HERE

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/widgets/button.h"

namespace {

class ColorPage : public BasicSurfaceWidget {
 public:
  ColorPage(ApplicationContext& context, Color color)
      : BasicSurfaceWidget(context), color_(color) {}

  Color background() const override { return color_; }

  void paint(PaintContext& ctx) const override { ctx.clear(); }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(120, 80);
  }

 private:
  Color color_;
};

class HorizontalPageHostDemo : public FlexLayout {
 public:
  explicit HorizontalPageHostDemo(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        host_(context),
        controls_(context, FlexDirection::kRow),
        prev_(context, "Prev", Button::TEXT),
        next_(context, "Next", Button::TEXT) {
    setGap(Scaled(8));
    controls_.setGap(Scaled(8));

    host_.addPage(std::make_unique<ColorPage>(context, color::SkyBlue));
    host_.addPage(std::make_unique<ColorPage>(context, color::Pink));
    host_.addPage(std::make_unique<ColorPage>(context, color::Teal));

    prev_.setOnInteractiveChange([this]() {
      int current = host_.currentIndex();
      if (current > 0) {
        host_.setCurrentIndex(current - 1);
      }
    });

    next_.setOnInteractiveChange([this]() {
      int current = host_.currentIndex();
      if (current + 1 < host_.pageCount()) {
        host_.setCurrentIndex(current + 1);
      }
    });

    controls_.add(prev_, {.flex_grow = 1, .flex_shrink = 1});
    controls_.add(next_, {.flex_grow = 1, .flex_shrink = 1});

    add(host_, {.flex_grow = 1, .flex_shrink = 1});
    add(controls_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  HorizontalPageHost host_;
  FlexLayout controls_;
  SimpleButton prev_;
  SimpleButton next_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
HorizontalPageHostDemo demo(app.context());
SingletonActivity activity(app, demo);

void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
