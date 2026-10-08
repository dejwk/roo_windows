// Learning goal: compact a settings screen at runtime without changing its
// fonts, icons, or firmware. Tap 0 through -5, then scroll through the fields
// and device choices. The device list has a local override you can clear.
// Application spacing stays under your control.
// Compact bounds are exact hit targets; keep 0 or add spacing for touch use.

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
#include "density_settings.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_icons/filled/navigation.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

using namespace roo_display;
using namespace roo_windows;

static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kTouchCsPin = 1;
static constexpr int kSpiSckPin = 4;
static constexpr int kSpiMisoPin = 5;
static constexpr int kSpiMosiPin = 6;

Ili9341spi<kCsPin, kDcPin, kRstPin> screen(Orientation().rotateLeft());
TouchXpt2046<kTouchCsPin> touch;
Display display(screen, touch,
                TouchCalibration(269, 249, 3829, 3684,
                                 Orientation::LeftDown()));

// This mutable storage outlives the Theme, Application, and all widgets.
// Copying a framework Theme alone still borrows its original Material object.
material3::Material3Theme material = DefaultTheme().material3Theme();
Theme theme{material3::MakeFrameworkTheme(material), &material};
roo_scheduler::SchedulingService scheduler;
Environment environment(scheduler, theme);
#ifdef ROO_TESTING
Application app(&environment, display, emulator_keys, true);
#else
Application app(&environment, display);
#endif

using DeviceRow = material3::ListRow<material3::RadioListItem>;

/// Owns one device choice across recycled rows; borrowed labels are literals.
class DeviceModel
    : public material3::DynamicSingleSelectionListModel<DeviceRow> {
 public:
  int elementCount() const override { return 8; }
  void prepare(DeviceRow& row) const override {
    row.item().setHeadline("Device");
  }
  void bind(int index, DeviceRow& row) const override {
    static const char* labels[] = {"Kitchen",  "Hall",       "Bedroom",
                                   "Office",   "Garage",     "Garden",
                                   "Workshop", "Living room"};
    row.item().setHeadline(labels[index]);
  }
  material3::DynamicListSectionState sectionState() const override {
    return {true, material3::DynamicListFocusTarget::kDescendant};
  }
};

/// Wraps row content vertically while using the viewport width.
class DeviceList : public material3::List {
 public:
  using List::List;
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }
};

/// Presents controls sharing the owned theme; only the device model is
/// retained.
class SettingsScreen : public SimpleScrollablePanel {
 public:
  explicit SettingsScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context) {
    auto form = std::make_unique<VerticalLayout>(context);
    // Explicit application padding stays fixed at every density.
    form->setPadding(Padding(Scaled(8)));
    form->add(std::make_unique<TextLabel>(
        context, "Display density", material3::text_style_title_medium()));
    auto choices = std::make_unique<HorizontalLayout>(context);
    static constexpr material3::Density levels[] = {
        material3::Density::kDefault, material3::Density::kMinus1,
        material3::Density::kMinus2,  material3::Density::kMinus3,
        material3::Density::kMinus4,  material3::Density::kMinus5};
    static const char* labels[] = {"0", "-1", "-2", "-3", "-4", "-5"};
    for (int i = 0; i < 6; ++i) {
      auto button = std::make_unique<material3::Button>(context, labels[i]);
      button->setSize(material3::ButtonSize::kExtraSmall);
      material3::Density level = levels[i];
      // Click settlement runs after painting closes. The two requests schedule
      // the next frame; they never measure or paint synchronously here.
      button->setOnInteractiveChange([level]() {
        density_example::ApplySetting(static_cast<int>(level), material,
                                      app.root());
        // Refresh retained detached roots too when they cache shared geometry.
        // The shared setting also reaches eligible controls inside dialogs.
      });
      choices->add(std::move(button));
    }
    form->add(std::move(choices));
    auto save = std::make_unique<material3::Button>(context, "Save settings");
    // Choose the icon asset for display zoom. Density removes whitespace,
    // including transparent icon margins, while keeping artwork unchanged.
    save->setIcon(&SCALED_ROO_ICON(filled, navigation_check));
    form->add(std::move(save));
    form->add(std::make_unique<material3::TextField>(context, "Device name"));
    form->add(std::make_unique<material3::TextField>(
        context, "Location", material3::TextFieldVariant::kOutlined));
    auto list = std::make_unique<DeviceList>(context);
    // Compact device rows independently of buttons and fields. The list owns
    // this policy for both its static rows and its recycled dynamic section.
    list->setDensity(material3::Density::kMinus3);
    DeviceList* device_list = list.get();
    auto inherit = std::make_unique<material3::Button>(
        context, "Use display density for devices");
    inherit->setOnInteractiveChange(
        [device_list]() { device_list->clearDensityOverride(); });
    form->add(std::move(inherit));
    list->add(std::make_unique<material3::ListRow<material3::CheckboxListItem>>(
        context, "Notifications"));
    list->add(std::make_unique<material3::ListRow<material3::RadioListItem>>(
        context, "Automatic connection"));
    list->add(
        std::make_unique<material3::DynamicList<DeviceRow>>(context, devices_));
    devices_.select(0);
    form->add(std::move(list));
    setContents(std::move(form));
  }

  // Destroy owned sections while their borrowed model still exists.
  ~SettingsScreen() override { clearContents(); }

 private:
  DeviceModel devices_;
};

SettingsScreen settings(app.context());
Task& task = app.addTaskFullScreen(settings);

void setup() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
  app.start();
  scheduler.run();
}

void loop() {}
