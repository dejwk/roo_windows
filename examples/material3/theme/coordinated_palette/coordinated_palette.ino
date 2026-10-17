// Learning goal: replace a coordinated palette in a mixed M3/M2 application.

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
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/checkbox.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/palette.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

Ili9341spi<7, 2, 3> screen(Orientation().rotateLeft());
TouchXpt2046<1> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

// Installs values only; each application explicitly requests its next frame.
void ChangePalette();

class CoordinatedPaletteDemo : public FlexLayout {
 public:
  explicit CoordinatedPaletteDemo(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        row_(context, FlexDirection::kRow),
        checkbox_(context, material2::Checkbox::OnOffState::kOn),
        label_(context, "Enable reminders", material3::text_style_body_large()),
        apply_(context, "Apply reminders"),
        change_palette_(context, "Change palette") {
    setPadding(Padding(Scaled(20)));
    setGap(Scaled(16));
    checkbox_.setParentClipMode(ParentClipMode::kUnclipped);
    row_.setGap(Scaled(12));
    row_.add(checkbox_);
    row_.add(label_);
    add(row_);
    add(apply_);
    add(change_palette_);
    apply_.setOnInteractiveChange([this]() { checkbox_.setOn(); });
    change_palette_.setOnInteractiveChange(ChangePalette);
  }

  ~CoordinatedPaletteDemo() override {
    row_.removeAll();
    removeAll();
  }

 private:
  class Row : public FlexLayout {
   public:
    using FlexLayout::FlexLayout;
    using FlexLayout::removeAll;
  } row_;
  material2::Checkbox checkbox_;
  TextLabel label_;
  material2::SimpleButton apply_;
  material3::Button change_palette_;
};

roo_scheduler::SchedulingService scheduler;
Theme theme;
Environment environment(scheduler, theme);
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif
CoordinatedPaletteDemo demo(app.context());
Task& task = app.addTaskFullScreen(demo);

void ChangePalette() {
  static bool warm = false;
  warm = !warm;
  auto colors = material3::DefaultColorScheme();
  if (warm) {
    colors.primary = Color(0xFF865000);
    colors.onPrimary = Color(0xFFFFFFFF);
    colors.background = colors.surface = Color(0xFFFFF8F0);
    colors.onSurface = Color(0xFF302820);
  }
  // Copies M3, framework roots, and matching M2 colors. The checkbox inherits
  // framework emphasis; the contained M2 button uses its own copied palette.
  ApplyCoordinatedMaterial3Palette(theme, colors);
  app.invalidateTheme();
}

void setup() {
  SPI.begin(4, 5, 6);
  display.enableTurbo();
  display.init();
  ApplyCoordinatedMaterial3Palette(theme, material3::DefaultColorScheme());
  app.start();
  scheduler.run();
}

void loop() {}
