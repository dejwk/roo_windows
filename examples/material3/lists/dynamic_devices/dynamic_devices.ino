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

// *************** DISPLAY SETUP BEGIN

#include "Arduino.h"
#include "roo_display.h"
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

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

void initDisplay() {
  SPI.begin(kSpiSckPin, kSpiMisoPin, kSpiMosiPin);
  display.enableTurbo();
  display.init();
}

#include <string>
#include <vector>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/dynamic_list.h"

namespace {
using DeviceRow = material3::ListRow<material3::RadioListItem>;

// Model storage is independent of the viewport-sized row pool.
class DeviceModel : public material3::DynamicListModel<DeviceRow> {
 public:
  struct Device {
    int id;
    std::string name;
  };
  std::vector<Device> devices = {{1, "Kitchen"}, {2, "Living room"}};
  material3::List* list = nullptr;
  material3::DynamicList<DeviceRow>* section = nullptr;
  int selected_index = -1;
  bool replacing = false;

  int elementCount() const override { return devices.size(); }

  void prepare(DeviceRow& row) const override {
    row.item().setHeadline("Device");
  }

  void bind(int index, DeviceRow& row) const override {
    row.item().setHeadline(devices[index].name);
    row.item().setSelected(index == selected_index);
    // Capture domain identity from this binding, never a recycled widget
    // address.
    int id = devices[index].id;
    row.item().setOnInvoked([this, id]() {
      // IDs are sequential in this append-only example; applications with
      // sorting resolve their own domain ID to the current model index after
      // reset.
      list->select(*section, id - 1);
    });
  }

  void unbind(DeviceRow& row) const override {
    row.item().setHeadline({});
    row.item().setOnInvoked({});
  }

  material3::DynamicListSectionState sectionState() const override {
    return {true, material3::DynamicListFocusTarget::kDescendant};
  }
};

class DeviceList : public material3::List {
 public:
  DeviceList(ApplicationContext& context, DeviceModel& model)
      : List(context), model_(model) {}

 protected:
  void onSingleSelectionChanged(material3::ListRowLocation location) override {
    int old_index = model_.selected_index;
    int new_index = location.section == model_.section ? location.index : -1;
    model_.selected_index = new_index;
    // Radio accessory state belongs to the application, separate from the row
    // highlight. Refresh only the known old/new bindings without a model scan.
    if (model_.replacing) return;
    if (old_index >= 0) model_.section->modelItemChanged(old_index);
    if (new_index >= 0 && new_index != old_index) {
      model_.section->modelItemChanged(new_index);
    }
  }

 private:
  DeviceModel& model_;
};

class DeviceScreen : public SimpleScrollablePanel {
 public:
  explicit DeviceScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        devices_(context, model_),
        add_(context, "Add device"),
        advanced_(context, "Advanced"),
        list_(context, model_) {
    model_.list = &list_;
    model_.section = &devices_;
    add_.item().setOnInvoked([this]() { addDevice(); });
    advanced_.item().setOnInvoked([this]() { list_.select(advanced_); });
    list_.setStyle(material3::ListStyle::kSegmented);
    material3::ListSelectionPolicy policy;
    policy.mode = material3::SelectionMode::kSingle;
    policy.selection_follows_press = false;
    list_.setSelectionPolicy(policy);
    list_.add(add_);
    list_.add(devices_);
    list_.add(advanced_);
    list_.select(devices_, 0);
    setContents(list_);
  }

  ~DeviceScreen() override { clearContents(); }

 private:
  void addDevice() {
    int previous_selection = model_.selected_index;
    model_.replacing = true;
    devices_.beginModelReset();
    int id = model_.devices.size() + 1;
    model_.devices.push_back({id, "Device " + std::to_string(id)});
    devices_.endModelReset();
    model_.replacing = false;
    if (previous_selection >= 0) list_.select(devices_, previous_selection);
  }

  DeviceModel model_;
  material3::DynamicList<DeviceRow> devices_;
  material3::ListRow<material3::InvokableListItemBase> add_;
  material3::ListRow<material3::InvokableListItemBase> advanced_;
  DeviceList list_;
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
  initDisplay();
  app.start();
  scheduler.run();
}

void loop() {}
