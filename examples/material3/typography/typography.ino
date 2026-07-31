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

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

static constexpr int kCsPin = 7, kDcPin = 2, kRstPin = 3;
static constexpr int kSpiSckPin = 4, kSpiMisoPin = 5, kSpiMosiPin = 6;
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

namespace {
class TypographyCatalog : public SimpleScrollablePanel {
 public:
  explicit TypographyCatalog(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 typography",
               material3::text_style_title_large()),
        badge_(context, "Badge and value indicator — Label small",
               material3::text_style_label_small()),
        button_(context, "Button — Label large",
                material3::text_style_label_large()),
        destination_(context, "Navigation destination — Label medium",
                     material3::text_style_label_medium()),
        tab_(context, "Tabs — Title small",
             material3::text_style_title_small()),
        headline_(context, "List headline — Body large",
                  material3::text_style_body_large()),
        supporting_(context, "List supporting — Body medium",
                    material3::text_style_body_medium()),
        small_(context, "Small app bar — Title large",
               material3::text_style_title_large()),
        medium_(context, "Medium app bar — Headline small",
                material3::text_style_headline_small()),
        large_(context, "Large app bar — Headline medium",
               material3::text_style_headline_medium()) {
    content_.setPadding(Padding(Scaled(16), Scaled(12)));
    content_.setGap(Scaled(8));
    content_.add(title_);
    content_.add(badge_);
    content_.add(button_);
    content_.add(destination_);
    content_.add(tab_);
    content_.add(headline_);
    content_.add(supporting_);
    content_.add(small_);
    content_.add(medium_);
    content_.add(large_);
    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel title_, badge_, button_, destination_, tab_, headline_, supporting_,
      small_, medium_, large_;
};
}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
TypographyCatalog catalog(app.context());
SingletonActivity activity(app, catalog);
void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
