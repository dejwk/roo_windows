// Edit account settings in place, validate required values, and reveal a
// password without leaving the form. Fields share the owning task editor.

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
        flex_viewport(viewport, 1, FlexViewport::kRotationNone),
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

using namespace roo_display;
using namespace roo_windows;

static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;
static constexpr int kTouchCsPin = 1;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen{Orientation()};
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

// Adjust the SPI pins and touch calibration above for your display.
void InitDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/text_field/secure_text_field.h"

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
// The scrollable column fills its viewport horizontally and wraps vertically.
class AccountForm : public FlexLayout {
 public:
  explicit AccountForm(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {}
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};
AccountForm form(app.context());
material3::TextField account(app.context(), "Account");
material3::SecureTextField password(app.context(), "Password",
                                    material3::TextFieldVariant::kOutlined);
material3::TextField amount(app.context(), "Daily limit");
material3::TextField region(app.context(), "Region",
                            material3::TextFieldVariant::kOutlined);
material3::Button validate(app.context(), "Validate");
// Keep the form usable when text scaling or the keyboard reduces its viewport.
SimpleScrollablePanel scroller(app.context(), form);

void setup() {
  account.setSupportingText("Enter your account name");
  password.setSupportingText("Tap the eye to reveal");
  amount.setPrefixText("$ ");
  amount.setSuffixText(" / day");
  amount.setText("25");
  region.setText("Europe");
  // A read-only field remains an action target, useful for picker-style values.
  region.setReadOnly(true);
  region.setOnInteractiveChange(
      [] { region.setText(region.text() == "Europe" ? "Asia" : "Europe"); });
  // Error text replaces help; clearing it restores the original help message.
  validate.setOnInteractiveChange([] {
    if (account.text().empty())
      account.setErrorText("Account is required");
    else
      account.clearError();
    if (password.text().empty())
      password.setErrorText("Password is required");
    else
      password.clearError();
  });
  form.setGap(Scaled(8));
  form.setPadding(Padding(Scaled(8)));
  form.add(account);
  form.add(password);
  form.add(amount);
  form.add(region);
  form.add(validate);
  app.addTaskFullScreen(scroller);
  InitDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
