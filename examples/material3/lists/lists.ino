// *************** EMULATOR SETUP BEGIN

#ifdef ROO_TESTING

#include "roo_testing/devices/display/ili9341/ili9341spi.h"
#include "roo_testing/devices/touch/xpt2046/xpt2046spi.h"
#include "roo_testing/microcontrollers/esp32/fake_esp32.h"
#include "roo_testing/transducers/ui/viewport/flex_viewport.h"
#include "roo_testing/transducers/ui/viewport/fltk/fltk_viewport.h"

using roo_testing_transducers::FlexViewport;
using roo_testing_transducers::FltkViewport;

struct Emulator {
  FltkViewport viewport;
  FlexViewport flexViewport;

  FakeIli9341Spi display;
  FakeXpt2046Spi touch;

  Emulator()
      : viewport(),
        flexViewport(viewport, 1, FlexViewport::kRotationRight),
        display(flexViewport),
        touch(flexViewport, FakeXpt2046Spi::Calibration(269, 249, 3829, 3684,
                                                        true, false, false)) {
    FakeEsp32().attachSpiDevice(display, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(7, display.cs());
    FakeEsp32().gpio.attachOutput(2, display.dc());
    FakeEsp32().gpio.attachOutput(3, display.rst());
    FakeEsp32().attachSpiDevice(touch, 4, 5, 6);
    FakeEsp32().gpio.attachOutput(1, touch.cs());
  }
} emulator;

#endif

// *************** DISPLAY SETUP BEGIN

#include <memory>

#include "Arduino.h"
#include "roo_display.h"
#include "roo_scheduler.h"
#include "roo_windows.h"

using namespace roo_display;
using namespace roo_windows;

// Select the driver to match your display device.
#include "roo_display/driver/ili9341.h"
#include "roo_display/driver/touch_xpt2046.h"

// Set your configuration for the driver.
static constexpr int kCsPin = 7;
static constexpr int kDcPin = 2;
static constexpr int kRstPin = 3;
static constexpr int kBlPin = 20;

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

// *************** EXAMPLE STARTS HERE

#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_label.h"

namespace {

class OwnedStandardListRow : public material3::ListEntry {
 public:
  OwnedStandardListRow(ApplicationContext& context,
                       const material3::StandardListItemInit& init)
      : material3::ListEntry(context), item_(init) {
    setItem(item_);
  }

 private:
  material3::StandardListItem item_;
};

class StaticSettingsSection : public FlexLayout {
 public:
  explicit StaticSettingsSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Expressive settings section", font_body1()),
        subtitle_(context,
                  "Borrowed rows with stable bound switches and expressive "
                  "selected states.",
                  font_caption()),
        list_(context),
        pool_mode_entry_(context),
        pool_mode_item_(
            material3::StandardListItemInit::TwoLine("Pool pump", "Auto")),
        solar_delta_entry_(context),
        solar_delta_item_(material3::StandardListItemInit::TwoLine(
            "Solar delta",
            "Starts when the roof loop exceeds the pool by 4 C")),
        safety_lock_entry_(context),
        safety_lock_switch_(context, material3::Switch::OnOffState::kOff),
        safety_lock_item_(material3::StandardListItemInit::OneLine(
            "Safety lock", nullptr, &safety_lock_switch_)) {
    setGap(Scaled(6));

    material3::ListSelectionPolicy selection_policy;
    selection_policy.mode = material3::SelectionMode::kMultiple;
    list_.setSelectionPolicy(selection_policy);

    material3::ListEntryVisualContext selected_context;
    selected_context.selected = true;

    pool_mode_entry_.setItem(pool_mode_item_);
    solar_delta_entry_.setItem(solar_delta_item_);
    solar_delta_entry_.setVisualContext(selected_context);
    safety_lock_entry_.setItem(safety_lock_item_);
    safety_lock_entry_.setVisualContext(selected_context);

    list_.add(pool_mode_entry_);
    list_.add(solar_delta_entry_);
    list_.add(safety_lock_entry_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List list_;
  material3::ListEntry pool_mode_entry_;
  material3::StandardListItem pool_mode_item_;
  material3::ListEntry solar_delta_entry_;
  material3::StandardListItem solar_delta_item_;
  material3::ListEntry safety_lock_entry_;
  material3::Switch safety_lock_switch_;
  material3::StandardListItem safety_lock_item_;
};

class AdoptedRowsSection : public FlexLayout {
 public:
  explicit AdoptedRowsSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Expressive segmented rows", font_body1()),
        subtitle_(context,
                  "The baseline owning-row pattern still reads clearly without "
                  "extra wrappers.",
                  font_caption()),
        list_(context) {
    setGap(Scaled(6));

    list_.setStyle(material3::ListStyle::kSegmented);

    material3::ListSelectionPolicy selection_policy;
    selection_policy.mode = material3::SelectionMode::kMultiple;
    list_.setSelectionPolicy(selection_policy);

    material3::ListEntryVisualContext selected_context;
    selected_context.selected = true;

    auto morning = std::make_unique<OwnedStandardListRow>(
        context, material3::StandardListItemInit::TwoLine("06:30 pool pump",
                                                          "Weekdays only"));
    auto solar = std::make_unique<OwnedStandardListRow>(
        context,
        material3::StandardListItemInit::TwoLine(
            "09:15 solar assist", "Starts when the roof loop is warmer"));
    auto spa = std::make_unique<OwnedStandardListRow>(
        context,
        material3::StandardListItemInit::TwoLine(
            "18:45 spa filter", "Skips automatically during freeze guard"));

    solar->setVisualContext(selected_context);

    list_.add(std::move(morning));
    list_.add(std::move(solar));
    list_.add(std::move(spa));

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List list_;
};

class MenuPrototypeSection : public FlexLayout {
 public:
  explicit MenuPrototypeSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Baseline menu prototype", font_body1()),
        subtitle_(
            context,
            "A compact baseline list reusing the same row and item primitives.",
            font_caption()),
        list_(context),
        refresh_entry_(context),
        refresh_shortcut_(context, "R", font_caption()),
        refresh_item_(material3::StandardListItemInit::OneLine(
            "Refresh", nullptr, &refresh_shortcut_)),
        edit_entry_(context),
        edit_shortcut_(context, "E", font_caption()),
        edit_item_(material3::StandardListItemInit::OneLine(
            "Edit schedule", nullptr, &edit_shortcut_)),
        remove_entry_(context),
        remove_shortcut_(context, "Del", font_caption()),
        remove_item_(material3::StandardListItemInit::OneLine(
            "Remove", nullptr, &remove_shortcut_)) {
    setGap(Scaled(6));

    list_.setVariant(material3::ListVariant::kBaseline);

    refresh_entry_.setItem(refresh_item_);
    edit_entry_.setItem(edit_item_);
    remove_entry_.setItem(remove_item_);

    list_.add(refresh_entry_);
    list_.add(edit_entry_);
    list_.add(remove_entry_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List list_;
  material3::ListEntry refresh_entry_;
  TextLabel refresh_shortcut_;
  material3::StandardListItem refresh_item_;
  material3::ListEntry edit_entry_;
  TextLabel edit_shortcut_;
  material3::StandardListItem edit_item_;
  material3::ListEntry remove_entry_;
  TextLabel remove_shortcut_;
  material3::StandardListItem remove_item_;
};

class ListsScreen : public SimpleScrollablePanel {
 public:
  explicit ListsScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 lists", font_h6()),
        subtitle_(context,
                  "Phase 4 - static settings, adopted rows, and a menu-like "
                  "prototype",
                  font_caption()),
        top_divider_(context),
        settings_(context),
        middle_divider_(context),
        adopted_(context),
        bottom_divider_(context),
        menu_(context) {
    content_.setPadding(Padding(Scaled(12), Scaled(8)));
    content_.setGap(Scaled(8));

    content_.add(title_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(top_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(settings_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(middle_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(adopted_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(bottom_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(menu_, {.flex_grow = 0, .flex_shrink = 0});

    setContents(content_);
  }

 private:
  FlexLayout content_;
  TextLabel title_;
  TextLabel subtitle_;
  HorizontalDivider top_divider_;
  StaticSettingsSection settings_;
  HorizontalDivider middle_divider_;
  AdoptedRowsSection adopted_;
  HorizontalDivider bottom_divider_;
  MenuPrototypeSection menu_;
};

}  // namespace

roo_scheduler::Scheduler scheduler;
Environment env(scheduler);
Application app(&env, display);
ListsScreen lists_screen(app.context());
SingletonActivity activity(app, lists_screen);

void setup() {
  initDisplay();
  app.start();

  // Never exits.
  scheduler.run();
}

void loop() {}