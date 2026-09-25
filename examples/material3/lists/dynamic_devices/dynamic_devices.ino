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

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/dynamic_list.h"

namespace {

using DeviceRow = material3::ListRow<material3::RadioListItem>;

/// Application data and row bindings, independent of any list widget.
class DeviceModel : public material3::DynamicListModel<DeviceRow> {
 public:
  /// Creates a model that reports device activation through @p
  /// on_device_invoked.
  explicit DeviceModel(std::function<void(int)> on_device_invoked)
      : on_device_invoked_(std::move(on_device_invoked)) {}

  /// Returns the number of available devices.
  int elementCount() const override { return devices_.size(); }

  /// Prepares one fixed headline slot, also defining the prototype's row
  /// height.
  void prepare(DeviceRow& row) const override {
    row.item().setHeadline("Device");
  }

  /// Binds text, radio state, and the domain action for the current device.
  void bind(int index, DeviceRow& row) const override {
    const Device& device = devices_[index];
    row.item().setHeadline(device.name);
    row.item().setSelected(device.id == selected_device_id_);
    // The index belongs to this binding's model revision. Reset cancels old
    // interactions and unbind() releases this callback before indices can
    // change.
    row.item().setOnInvoked([this, index]() { on_device_invoked_(index); });
  }

  /// Releases borrowed text and the previous device action before reuse/reset.
  void unbind(DeviceRow& row) const override {
    row.item().setHeadline({});
    row.item().setOnInvoked({});
  }

  /// Directs keyboard navigation to each row's radio accessory.
  material3::DynamicListSectionState sectionState() const override {
    return {true, material3::DynamicListFocusTarget::kDescendant};
  }

  /// Returns the stable application ID at @p index.
  int deviceId(int index) const { return devices_[index].id; }

  /// Updates application state; the screen separately notifies affected rows.
  void setSelectedDeviceId(int device_id) { selected_device_id_ = device_id; }

  /// Adds a device. The screen must release old bindings before vector growth.
  void addDevice() {
    int device_id = next_device_id_++;
    devices_.push_back({device_id, "Device " + std::to_string(device_id)});
  }

 private:
  struct Device {
    int id;
    std::string name;
  };

  std::vector<Device> devices_ = {{1, "Kitchen"}, {2, "Living room"}};
  std::function<void(int)> on_device_invoked_;
  int selected_device_id_ = 0;
  int next_device_id_ = 3;
};

/// Full-width list that forwards its selection hook to the screen controller.
class DeviceList : public material3::List {
 public:
  /// Creates a list reporting logical selection through @p
  /// on_selection_changed.
  DeviceList(
      ApplicationContext& context,
      std::function<void(material3::ListRowLocation)> on_selection_changed)
      : List(context), on_selection_changed_(std::move(on_selection_changed)) {}

  /// Uses the viewport width while retaining scrollable content height.
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }

 protected:
  void onSingleSelectionChanged(material3::ListRowLocation location) override {
    on_selection_changed_(location);
  }

 private:
  std::function<void(material3::ListRowLocation)> on_selection_changed_;
};

/// Coordinates model updates, logical selection, and the two fixed action rows.
class DeviceScreen : public SimpleScrollablePanel {
 public:
  /// Creates a scrollable device chooser using @p context.
  explicit DeviceScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        model_([this](int index) { list_.select(devices_, index); }),
        devices_(context, model_),
        add_(context, "Add device"),
        advanced_(context, "Advanced"),
        list_(context, [this](material3::ListRowLocation location) {
          selectionChanged(location);
        }) {
    add_.item().setOnInvoked([this]() { addDevice(); });
    advanced_.item().setOnInvoked([this]() { list_.select(advanced_); });
    list_.setStyle(material3::ListStyle::kSegmented);

    // The screen owns selection coordination. Radio activation and row actions
    // both reach this screen with a known index in the current model revision.
    material3::ListSelectionPolicy policy;
    policy.mode = material3::SelectionMode::kSingle;
    // Automatic selection would select "Add device" before its callback and
    // clear the device selection we want to preserve. Select explicitly
    // instead.
    policy.selection_follows_press = false;
    list_.setSelectionPolicy(policy);
    list_.add(add_);
    list_.add(devices_);
    list_.add(advanced_);
    list_.select(devices_, 0);
    setContents(list_);
  }

  /// Detaches the borrowed list before member destruction begins.
  ~DeviceScreen() override { clearContents(); }

 private:
  // Row highlighting and radio accessory state are separate. Update the model,
  // then refresh only the known old/new indices; no selection-discovery scan.
  void selectionChanged(material3::ListRowLocation location) {
    int old_index = selected_index_;
    selected_index_ = location.section == &devices_ ? location.index : -1;
    model_.setSelectedDeviceId(
        selected_index_ >= 0 ? model_.deviceId(selected_index_) : 0);
    if (replacing_model_) return;
    if (old_index >= 0) devices_.modelItemChanged(old_index);
    if (selected_index_ >= 0 && selected_index_ != old_index) {
      devices_.modelItemChanged(selected_index_);
    }
  }

  // Vector growth can move strings referenced by pooled rows. Begin reset
  // releases those views before mutation; end reset publishes the new data.
  void addDevice() {
    // This mutation only appends, so every existing index keeps its identity.
    int previous_index = selected_index_;
    replacing_model_ = true;
    devices_.beginModelReset();
    model_.addDevice();
    devices_.endModelReset();
    replacing_model_ = false;
    if (previous_index >= 0) list_.select(devices_, previous_index);
  }

  // Declaration order matters: the model and borrowed rows outlive the list.
  DeviceModel model_;
  material3::DynamicList<DeviceRow> devices_;
  material3::ListRow<material3::InvokableListItemBase> add_;
  material3::ListRow<material3::InvokableListItemBase> advanced_;
  DeviceList list_;
  int selected_index_ = -1;
  bool replacing_model_ = false;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
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
