// Choose a maintenance date using calendar, numeric input, or a docked field.

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
#include "roo_windows/material3/date_picker/docked_date_picker_field.h"

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif

using material3::DatePickerEntryMode;
using roo_time::CivilDay;

class MaintenanceField : public material3::DockedDatePickerField {
 public:
  explicit MaintenanceField(ApplicationContext& context)
      : DockedDatePickerField(context, "Next maintenance") {}

 protected:
  void onAccepted(CivilDay) override {
    setSupportingText("Maintenance date saved");
  }
};
MaintenanceField date_field(app.context());

class MaintenancePicker : public material3::ModalDatePicker {
 public:
  using ModalDatePicker::ModalDatePicker;

 protected:
  void onAccepted(CivilDay day) override {
    date_field.setDate(day);
    date_field.setSupportingText("Maintenance date saved");
  }
};
MaintenancePicker picker(app.context());
material3::Button calendar(app.context(), "Choose on calendar");
material3::Button numeric(app.context(), "Enter a date");
FlexLayout form(app.context(), FlexDirection::kColumn);
SimpleScrollablePanel scroller(app.context(), form);

void OpenDate(DatePickerEntryMode mode) {
  picker.setValue(date_field.date());
  picker.setEntryMode(mode);
  if (picker.open(*calendar.getTask()) != PresentationStartResult::kStarted) {
    date_field.setSupportingText("Date picker is unavailable");
  }
}

void setup() {
  // Civil dates are explicit: applications decide how to obtain local today.
  CivilDay today = CivilDay::FromYmd(2026, 9, 19);
  material3::DatePickerBounds bounds{today, CivilDay::FromYmd(2028, 12, 31)};
  date_field.setDate(CivilDay::FromYmd(2026, 10, 1));
  date_field.setToday(today);
  date_field.setBounds(bounds);
  date_field.setSupportingText("Tap to choose a maintenance date");
  picker.setToday(today);
  picker.setBounds(bounds);
  calendar.setOnInteractiveChange(
      [] { OpenDate(DatePickerEntryMode::kCalendar); });
  numeric.setOnInteractiveChange([] { OpenDate(DatePickerEntryMode::kInput); });
  form.setGap(Scaled(8));
  form.setPadding(Padding(Scaled(8)));
  form.add(calendar);
  form.add(numeric);
  form.add(date_field);
  // On this compact display the docked calendar promotes to full-screen.
  // On a larger display it anchors to the field. Both capture calendar focus.
  app.addTaskFullScreen(scroller);
  InitDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
