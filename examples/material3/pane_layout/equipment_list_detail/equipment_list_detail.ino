// Learning goal: build a responsive list-detail flow with PaneLayout. Compact
// screens show the caller-selected pane; expanded screens dock the equipment
// list beside the current detail without changing application routes.

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

#include <memory>
#include <string>

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

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows/widgets/text_label.h"

namespace {
/// Equipment browser that owns compact pane selection and detail state.
class EquipmentListDetail : public material3::PaneLayout {
 public:
  explicit EquipmentListDetail(ApplicationContext& context)
      : material3::PaneLayout(context), details_(nullptr) {
    auto equipment =
        std::make_unique<FlexLayout>(context, FlexDirection::kColumn);
    equipment->setPadding(Padding(Scaled(12), Scaled(12)));
    equipment->setGap(Scaled(8));
    equipment->add(WidgetRef(std::make_unique<TextLabel>(
        context, "Equipment", material3::text_style_title_large())));
    auto pump = std::make_unique<material3::Button>(
        context, "Circulation pump", material3::ButtonVariant::kFilledTonal);
    pump->setOnInteractiveChange([this]() { showDetail(0); });
    auto heater = std::make_unique<material3::Button>(
        context, "Solar heater", material3::ButtonVariant::kOutlined);
    heater->setOnInteractiveChange([this]() { showDetail(1); });
    equipment->add(WidgetRef(std::move(pump)));
    equipment->add(WidgetRef(std::move(heater)));

    auto details = std::make_unique<StackedLayout>(context);
    details_ = details.get();
    details_->add(makeDetail(context, "Circulation pump",
                             "Running · 1,850 rpm · 96 L/min"));
    details_->add(makeDetail(context, "Solar heater",
                             "Available · roof temperature 31.4 °C"));

    // The default PaneSpec docks leading beside main only when expanded.
    setLeadingPane(WidgetRef(std::move(equipment)));
    setMainPane(WidgetRef(std::move(details)));
    setActivePane(material3::PaneRole::kLeading);
    selectDetail(0);
  }

 private:
  WidgetRef makeDetail(ApplicationContext& context, roo::string_view title,
                       roo::string_view status) {
    auto detail = std::make_unique<FlexLayout>(context, FlexDirection::kColumn);
    detail->setPadding(Padding(Scaled(16), Scaled(16)));
    detail->setGap(Scaled(8));
    detail->add(WidgetRef(std::make_unique<TextLabel>(
        context, std::string(title), material3::text_style_title_large())));
    detail->add(WidgetRef(std::make_unique<TextLabel>(
        context, std::string(status), material3::text_style_body_large())));
    auto back = std::make_unique<material3::Button>(
        context, "Back to equipment", material3::ButtonVariant::kText);
    back->setOnInteractiveChange(
        [this]() { setActivePane(material3::PaneRole::kLeading); });
    detail->add(WidgetRef(std::move(back)));
    return WidgetRef(std::move(detail));
  }

  void selectDetail(size_t index) {
    for (size_t i = 0; i < details_->children().size(); ++i) {
      details_->children()[i]->setVisibility(i == index ? Visibility::kVisible
                                                        : Visibility::kGone);
    }
  }

  void showDetail(size_t index) {
    selectDetail(index);
    // Compact screens switch panes; expanded screens keep both visible.
    setActivePane(material3::PaneRole::kMain);
  }

  StackedLayout* details_;
};
}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
#ifdef ROO_TESTING
Application app(&env, display, emulator_keys, true);
#else
Application app(&env, display);
#endif
EquipmentListDetail equipment(app.context());
SingletonActivity activity(app, equipment);
void setup() {
  initDisplay();
  app.start();
  scheduler.run();
}
void loop() {}
