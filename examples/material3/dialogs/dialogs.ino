// Material 3 alert and generic basic-dialog examples. The dialog objects and
// their borrowed action labels live for the application lifetime.

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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/dialog/basic_dialog.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_block.h"
#include "roo_windows/widgets/text_label.h"

namespace {

using material3::DialogActionRole;
using material3::DialogActionSpec;

constexpr DialogActionSpec kAlertActions[] = {
    {1, "Later", DialogActionRole::kDismiss},
    {2, "Restart", DialogActionRole::kConfirm},
};
constexpr DialogActionSpec kChoiceActions[] = {
    {3, "Cancel", DialogActionRole::kDismiss},
    {4, "Apply", DialogActionRole::kConfirm},
};

class StatusAlert final : public material3::AlertDialog {
 public:
  StatusAlert(ApplicationContext& context, TextLabel& status)
      : AlertDialog(context, "Restart controller?",
                    "Active schedules pause briefly while the controller "
                    "restarts.",
                    kAlertActions, 2),
        status_(status) {}

 protected:
  void onActionInvoked(uint8_t id, DialogActionRole) override {
    status_.setText(id == 2 ? "Restart requested" : "Restart postponed");
  }

 private:
  TextLabel& status_;
};

class ChoiceDialog final : public material3::BasicDialog {
 public:
  ChoiceDialog(ApplicationContext& context, Widget& body, TextLabel& status)
      : BasicDialog(context, WidgetRef(body), kChoiceActions, 2),
        status_(status) {
    setHeadline("Circulation profile");
  }

 protected:
  void onActionInvoked(uint8_t id, DialogActionRole) override {
    status_.setText(id == 4 ? "Profile applied" : "No profile change");
  }

 private:
  TextLabel& status_;
};

template <typename DialogT>
class OpenDialogButton final : public material3::Button {
 public:
  OpenDialogButton(ApplicationContext& context, roo::string_view label,
                   DialogT& dialog)
      : Button(context, label, material3::ButtonVariant::kFilledTonal),
        dialog_(dialog) {}

  void bind(Task& owner) { owner_ = &owner; }

  void onClicked() override {
    if (owner_ != nullptr) dialog_.show(*owner_);
  }

 private:
  Task* owner_ = nullptr;
  DialogT& dialog_;
};

class DialogCatalog final : public FlexLayout {
 public:
  explicit DialogCatalog(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Material 3 dialogs",
               material3::text_style_title_large()),
        status_(context, "Choose a dialog",
                material3::text_style_body_medium()),
        choice_body_(context,
                     "Eco: quiet filtration\nBoost: maximum circulation",
                     material3::text_style_body_medium()),
        alert_(context, status_),
        choice_(context, choice_body_, status_),
        open_alert_(context, "Open alert", alert_),
        open_choice_(context, "Choose profile", choice_) {
    setPadding(Padding(Scaled(16)));
    setGap(Scaled(12));
    add(title_);
    add(open_alert_);
    add(open_choice_);
    add(status_);
  }

  void bind(Task& owner) {
    open_alert_.bind(owner);
    open_choice_.bind(owner);
  }

 private:
  TextLabel title_;
  TextLabel status_;
  TextBlock choice_body_;
  StatusAlert alert_;
  ChoiceDialog choice_;
  OpenDialogButton<StatusAlert> open_alert_;
  OpenDialogButton<ChoiceDialog> open_choice_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
DialogCatalog catalog(app.context());
Task& task = app.addTaskFullScreen(catalog);

void setup() {
  catalog.bind(task);
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
