// Learning goal: mix static actions and recycled radio rows in one list.

// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"
#include "roo_windows/fake/fltk_key_source.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

/// Connects the emulated display and touch controller to the simulated pins.
struct Emulator {
  FltkViewport viewport;
  FlexViewport flex_viewport;
  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  /// Creates the display/touch devices with matching orientation and
  /// calibration.
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

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using roo_display::Display;
using roo_display::Ili9341spi;
using roo_display::Orientation;
using roo_display::TouchCalibration;
using roo_display::TouchXpt2046;
using roo_windows::Application;
using roo_windows::ApplicationContext;
using roo_windows::Environment;
using roo_windows::PreferredSize;
using roo_windows::SimpleScrollablePanel;
using roo_windows::Task;
namespace material3 = roo_windows::material3;

// Change these pins and the touch calibration for your display board.
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

/// Initializes the physical display bus; customize the pin constants above.
void InitDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

// *************** DYNAMIC LIST EXAMPLE BEGIN

#include <string>
#include <vector>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/dynamic_list.h"

namespace {

using DeviceRow = material3::ListRow<material3::RadioListItem>;

/// Device data and one selected index; the framework manages radio state.
class DeviceModel
    : public material3::DynamicSingleSelectionListModel<DeviceRow> {
 public:
  /// Returns the number of available devices.
  int elementCount() const override { return devices_.size(); }

  /// Prepares the fixed headline slot and representative prototype height.
  void prepare(DeviceRow& row) const override {
    row.item().setHeadline("Device");
  }

  /// Supplies content; the selection helper supplies radio state and routing.
  void bind(int index, DeviceRow& row) const override {
    row.item().setHeadline(devices_[index]);
  }

  /// Releases text before the vector may move its backing strings.
  void unbind(DeviceRow& row) const override { row.item().setHeadline({}); }

  /// Directs keyboard navigation to each row's radio accessory.
  material3::DynamicListSectionState sectionState() const override {
    return {true, material3::DynamicListFocusTarget::kDescendant};
  }

  /// Appends a device while the screen has released borrowed text bindings.
  void addDevice() {
    devices_.push_back("Device " + std::to_string(devices_.size() + 1));
  }

 private:
  std::vector<std::string> devices_ = {"Kitchen", "Living room"};
};

/// Uses the viewport width while retaining scrollable content height.
class DeviceList : public material3::List {
 public:
  /// Creates a full-width list using @p context.
  explicit DeviceList(ApplicationContext& context) : List(context) {}

  /// Fills the viewport horizontally and wraps all rows vertically.
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }
};

/// Coordinates safe data replacement and two non-selectable action rows.
class DeviceScreen : public SimpleScrollablePanel {
 public:
  /// Creates a scrollable device chooser using @p context.
  explicit DeviceScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        devices_(context, model_),
        add_(context, "Add device"),
        advanced_(context, "Advanced"),
        list_(context) {
    add_.item().setOnInvoked([this]() { addDevice(); });
    advanced_.item().setOnInvoked([this]() {
      advanced_.item().setHeadline("Advanced settings unavailable");
      advanced_.refreshFromItem();
    });
    // Actions invoke normally without becoming selection choices.
    add_.item().setSelectionParticipation(
        material3::SelectionParticipation::kAction);
    advanced_.item().setSelectionParticipation(
        material3::SelectionParticipation::kAction);
    list_.setStyle(material3::ListStyle::kSegmented);
    // Parent-wide selection stays at its default, kNone. The model owns the
    // device group; no callbacks or selection-policy override are needed.
    list_.add(add_);
    list_.add(devices_);
    list_.add(advanced_);
    model_.select(0);
    setContents(list_);
  }

  /// Detaches the borrowed list before member destruction begins.
  ~DeviceScreen() override { clearContents(); }

 private:
  // Reset releases borrowed strings before vector growth. Appending preserves
  // every existing index, so the model's selection remains valid automatically.
  // For removal/reordering, clear or remap selection while bindings are
  // released.
  void addDevice() {
    devices_.beginModelReset();
    model_.addDevice();
    devices_.endModelReset();
  }

  // Declaration order matters: the model and borrowed rows outlive the list.
  DeviceModel model_;
  material3::DynamicList<DeviceRow> devices_;
  material3::ListRow<material3::InvokableListItemBase> add_;
  material3::ListRow<material3::InvokableListItemBase> advanced_;
  DeviceList list_;
};

}  // namespace

roo_scheduler::SchedulingService scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
DeviceScreen devices(app.context());
Task& task = app.addTaskFullScreen(devices);

void setup() {
  InitDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
