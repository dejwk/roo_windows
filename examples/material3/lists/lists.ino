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
#include "roo_icons/filled/24/device.h"
#include "roo_icons/outlined/24/notification.h"
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

material3::StandardListItemInit WrappedTwoLine(roo::string_view headline,
                                               roo::string_view supporting) {
  material3::StandardListItemInit init =
      material3::StandardListItemInit::TwoLine(headline, supporting);
  init.supporting_policy.overflow = material3::TextOverflowPolicy::kWrap;
  init.supporting_policy.max_lines = 2;
  return init;
}

material3::ListTextPolicy WrappedSupporting(uint8_t max_lines = 2) {
  material3::ListTextPolicy policy;
  policy.overflow = material3::TextOverflowPolicy::kWrap;
  policy.max_lines = max_lines;
  return policy;
}

class StaticSettingsSection : public FlexLayout {
 public:
  explicit StaticSettingsSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Expressive settings section", font_body1()),
        subtitle_(context,
                  "Inset dividers separate standard rows while selected "
                  "entries keep their expressive shape roles.",
                  font_caption()),
        list_(context),
        safety_lock_switch_(context, material3::Switch::OnOffState::kOff),
        pool_mode_row_(context, material3::StandardListItemInit::TwoLine(
                                    "Pool pump", "Auto")),
        solar_delta_row_(
            context,
            WrappedTwoLine("Solar delta",
                           "Starts when the roof loop exceeds the pool by 4 C "
                           "and pauses when freeze guard is active")),
        safety_lock_row_(context,
                         material3::StandardListItemInit::OneLine(
                             "Safety lock", nullptr, &safety_lock_switch_)) {
    setGap(Scaled(6));

    material3::ListSelectionPolicy selection_policy;
    selection_policy.mode = material3::SelectionMode::kMultiple;
    list_.setSelectionPolicy(selection_policy);

    material3::ListDividerPolicy divider_policy;
    divider_policy.mode = material3::DividerMode::kInset;
    divider_policy.start_inset = 16;
    divider_policy.end_inset = 24;
    list_.setDividerPolicy(divider_policy);

    material3::ListEntryVisualContext selected_context;
    selected_context.selected = true;

    solar_delta_row_.setVisualContext(selected_context);
    safety_lock_row_.setVisualContext(selected_context);

    list_.add(pool_mode_row_);
    list_.add(solar_delta_row_);
    list_.add(safety_lock_row_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List list_;
  material3::Switch safety_lock_switch_;
  material3::ListRow<material3::StandardListItem> pool_mode_row_;
  material3::ListRow<material3::StandardListItem> solar_delta_row_;
  material3::ListRow<material3::StandardListItem> safety_lock_row_;
};

class AdoptedRowsSection : public FlexLayout {
 public:
  explicit AdoptedRowsSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Expressive segmented rows", font_body1()),
        subtitle_(context,
                  "Segmented gaps now pair with expressive first, middle, and "
                  "last row shapes.",
                  font_caption()),
        list_(context) {
    setGap(Scaled(6));

    list_.setStyle(material3::ListStyle::kSegmented);

    material3::ListSelectionPolicy selection_policy;
    selection_policy.mode = material3::SelectionMode::kMultiple;
    list_.setSelectionPolicy(selection_policy);

    material3::ListEntryVisualContext selected_context;
    selected_context.selected = true;

    auto morning =
        std::make_unique<material3::ListRow<material3::StandardListItem>>(
            context, material3::StandardListItemInit::TwoLine("06:30 pool pump",
                                                              "Weekdays only"));
    auto solar =
        std::make_unique<material3::ListRow<material3::StandardListItem>>(
            context,
            material3::StandardListItemInit::TwoLine(
                "09:15 solar assist", "Starts when the roof loop is warmer"));
    auto spa =
        std::make_unique<material3::ListRow<material3::StandardListItem>>(
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
        subtitle_(context,
                  "Baseline rows stay square and use full-width dividers for "
                  "stronger separation.",
                  font_caption()),
        list_(context),
        refresh_shortcut_(context, "R", font_caption()),
        edit_shortcut_(context, "E", font_caption()),
        remove_shortcut_(context, "Del", font_caption()),
        refresh_row_(context, material3::StandardListItemInit::OneLine(
                                  "Refresh", nullptr, &refresh_shortcut_)),
        edit_row_(context, material3::StandardListItemInit::OneLine(
                               "Edit schedule", nullptr, &edit_shortcut_)),
        remove_row_(context, material3::StandardListItemInit::OneLine(
                                 "Remove", nullptr, &remove_shortcut_)) {
    setGap(Scaled(6));

    list_.setVariant(material3::ListVariant::kBaseline);

    material3::ListDividerPolicy divider_policy;
    divider_policy.mode = material3::DividerMode::kFullWidth;
    list_.setDividerPolicy(divider_policy);

    list_.add(refresh_row_);
    list_.add(edit_row_);
    list_.add(remove_row_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List list_;
  TextLabel refresh_shortcut_;
  TextLabel edit_shortcut_;
  TextLabel remove_shortcut_;
  material3::ListRow<material3::StandardListItem> refresh_row_;
  material3::ListRow<material3::StandardListItem> edit_row_;
  material3::ListRow<material3::StandardListItem> remove_row_;
};

class VisualConvenienceSection : public FlexLayout {
 public:
  explicit VisualConvenienceSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Phase 8 convenience items", font_body1()),
        subtitle_(context,
                  "Avatar, pictogram, and text-only items now bind through "
                  "the shared ListRow surface.",
                  font_caption()),
        contacts_(context),
        status_(context),
        owner_(context, "DW", "Dawid Wojcik", "Pool owner"),
        roof_loop_(context, ic_filled_24_device_wifi_tethering(),
                   "Roof loop network",
                   "Supporting text wraps on narrow screens without a custom "
                   "row subclass.", material3::ListTextPolicy{},
                   WrappedSupporting()),
        alerts_headline_(context, "Alert routing"),
        freeze_guard_(context, "Freeze guard",
                      "Inset dividers and convenience items share the same "
                      "text policy path.",
                      material3::ListTextPolicy{}, WrappedSupporting()),
        schedule_sync_(context, ic_outlined_24_notification_sync(),
                       "Schedule sync",
                       "Selected segmented rows still use the same row host.",
                       material3::ListTextPolicy{}, WrappedSupporting()) {
    setGap(Scaled(6));

    contacts_.setStyle(material3::ListStyle::kSegmented);

    material3::ListDividerPolicy inset_dividers;
    inset_dividers.mode = material3::DividerMode::kInset;
    inset_dividers.start_inset = 72;
    inset_dividers.end_inset = 24;
    status_.setDividerPolicy(inset_dividers);

    material3::ListEntryVisualContext selected_context;
    selected_context.selected = true;
    schedule_sync_.setVisualContext(selected_context);

    contacts_.add(owner_);
    contacts_.add(roof_loop_);

    status_.add(alerts_headline_);
    status_.add(freeze_guard_);
    status_.add(schedule_sync_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(contacts_, {.flex_grow = 0, .flex_shrink = 0});
    add(status_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List contacts_;
  material3::List status_;
  material3::ListRow<material3::AvatarSupportingTextItem> owner_;
  material3::ListRow<material3::PictogramSupportingTextItem> roof_loop_;
  material3::ListRow<material3::HeadlineListItem> alerts_headline_;
  material3::ListRow<material3::SupportingTextListItem> freeze_guard_;
  material3::ListRow<material3::PictogramSupportingTextItem> schedule_sync_;
};

class ListsScreen : public SimpleScrollablePanel {
 public:
  explicit ListsScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 lists", font_h6()),
        subtitle_(context,
                  "Phase 8 - convenience items for avatar, pictogram, and "
                  "text-only Material 3 rows",
                  font_caption()),
        top_divider_(context),
        settings_(context),
        middle_divider_(context),
        adopted_(context),
        bottom_divider_(context),
        convenience_(context),
        final_divider_(context),
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
    content_.add(convenience_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(final_divider_, {.flex_grow = 0, .flex_shrink = 0});
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
  VisualConvenienceSection convenience_;
  HorizontalDivider final_divider_;
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