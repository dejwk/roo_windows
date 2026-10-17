// Learning goal: borrow a Material 2 checkbox inside a Material 3 surface.
// Generic layouts forward foreground and readable accent without theme copies.

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

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

Ili9341spi<7, 2, 3> screen(Orientation().rotateLeft());
TouchXpt2046<1> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

class BorrowedCheckboxDemo : public FlexLayout {
 public:
  explicit BorrowedCheckboxDemo(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        row_(context, FlexDirection::kRow),
        checkbox_(context, material2::Checkbox::OnOffState::kOn),
        label_(context, "Enable reminders", material3::text_style_body_large()),
        change_surface_(context, "Change surface") {
    setPadding(Padding(Scaled(20)));
    setGap(Scaled(16));
    // Let the point overlay extend beyond the row without changing alignment.
    checkbox_.setParentClipMode(ParentClipMode::kUnclipped);
    row_.setGap(Scaled(12));
    row_.add(checkbox_);
    row_.add(label_);
    add(row_);
    add(change_surface_);
    change_surface_.setOnInteractiveChange([this]() {
      colored_ = !colored_;
      // Color queries are live. Invalidate descendants after changing the
      // boundary so their displayed colors catch up with the new pair.
      invalidateDescending();
      invalidateInterior();
    });
  }

  ~BorrowedCheckboxDemo() override {
    row_.removeAll();
    removeAll();
  }

  material3::ColorToken containerRole() const override {
    return colored_ ? material3::ColorToken::kSecondaryContainer
                    : material3::ColorToken::kSurfaceContainerLow;
  }

  Color background() const override {
    return theme().material3Theme().color.resolve(containerRole());
  }

 private:
  // The row has no surface role: both its label and borrowed checkbox inherit
  // our pair. The contained M3 button resolves its own surface independently.
  class Row : public FlexLayout {
   public:
    using FlexLayout::FlexLayout;
    using FlexLayout::removeAll;
  } row_;
  material2::Checkbox checkbox_;
  TextLabel label_;
  material3::Button change_surface_;
  bool colored_ = false;
};

roo_scheduler::SchedulingService scheduler;
Theme theme;
Environment environment(scheduler, theme);
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif
BorrowedCheckboxDemo demo(app.context());
Task& task = app.addTaskFullScreen(demo);

void setup() {
  SPI.begin(4, 5, 6);
  display.enableTurbo();
  display.init();
  // On the neutral surface, normal text and checked emphasis are distinct.
  // On the colored surface both resolve to readable on-container ink. The
  // Material 2 palette is independent and is not copied or modified here.
  theme.material3Theme().color.onSurface = Color(0xFF213547);
  theme.material3Theme().color.primary = Color(0xFF865000);
  app.start();
  scheduler.run();
}

void loop() {}
