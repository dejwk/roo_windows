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
                   "row subclass.",
                   material3::ListTextPolicy{}, WrappedSupporting()),
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

class NavigationSection : public FlexLayout {
 public:
  explicit NavigationSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Phase 9 navigation items", font_body1()),
        subtitle_(context,
                  "Rows become clickable only for invokable items, and row "
                  "presses share the same invoke path as item callbacks.",
                  font_caption()),
        navigation_(context),
        next_task_(context, ic_outlined_24_notification_sync(), "Next task",
                   "Open task details"),
        owner_(context, "DW", "Assigned owner", "Open schedule") {
    setGap(Scaled(6));

    next_task_.item().setOnInvoked([this]() {
      next_task_.item().setSupportingText("Task detail opened");
      next_task_.refreshFromItem();
    });
    owner_.item().setOnInvoked([this]() {
      owner_.item().setSupportingText("Schedule opened");
      owner_.refreshFromItem();
    });

    navigation_.add(next_task_);
    navigation_.add(owner_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(navigation_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List navigation_;
  material3::ListRow<material3::NavigationListItem> next_task_;
  material3::ListRow<material3::AvatarNavigationListItem> owner_;
};

class SelectionSection : public FlexLayout {
 public:
  explicit SelectionSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Phase 10 selection items", font_body1()),
        subtitle_(context,
                  "Row presses and control presses share one invoke path for "
                  "checkbox, radio, and switch rows.",
                  font_caption()),
        multi_select_(context),
        single_select_(context),
        toggle_list_(context),
        notifications_(context, "Notifications", "Include push updates", true,
                       material3::AffordancePlacement::kLeading),
        freeze_guard_(context, "Freeze guard", "Protect outdoor loop", false,
                      material3::AffordancePlacement::kLeading),
        compact_(context, "Compact", "Dense spacing", false,
                 material3::AffordancePlacement::kLeading),
        balanced_(context, "Balanced", "Default spacing", true,
                  material3::AffordancePlacement::kLeading),
        comfortable_(context, "Comfortable", "Extra breathing room", false,
                     material3::AffordancePlacement::kLeading),
        thermal_lock_(context, "Thermal lock", "Prevent accidental toggles",
                      true),
        heater_boost_(context, "Heater boost", "Temporarily raise target",
                      false) {
    setGap(Scaled(6));

    material3::ListSelectionPolicy multi_policy;
    multi_policy.mode = material3::SelectionMode::kMultiple;
    multi_policy.affordance = material3::SelectionAffordance::kCheckbox;
    multi_policy.placement = material3::AffordancePlacement::kLeading;
    multi_select_.setSelectionPolicy(multi_policy);

    material3::ListSelectionPolicy single_policy;
    single_policy.mode = material3::SelectionMode::kSingle;
    single_policy.affordance = material3::SelectionAffordance::kRadio;
    single_policy.placement = material3::AffordancePlacement::kLeading;
    single_select_.setSelectionPolicy(single_policy);

    material3::ListSelectionPolicy switch_policy;
    switch_policy.mode = material3::SelectionMode::kMultiple;
    switch_policy.affordance = material3::SelectionAffordance::kSwitch;
    toggle_list_.setSelectionPolicy(switch_policy);

    compact_.item().setOnInvoked([this]() { selectDensity(0); });
    balanced_.item().setOnInvoked([this]() { selectDensity(1); });
    comfortable_.item().setOnInvoked([this]() { selectDensity(2); });

    multi_select_.add(notifications_);
    multi_select_.add(freeze_guard_);

    single_select_.add(compact_);
    single_select_.add(balanced_);
    single_select_.add(comfortable_);

    toggle_list_.add(thermal_lock_);
    toggle_list_.add(heater_boost_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(multi_select_, {.flex_grow = 0, .flex_shrink = 0});
    add(single_select_, {.flex_grow = 0, .flex_shrink = 0});
    add(toggle_list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  void selectDensity(int idx) {
    compact_.item().setSelected(idx == 0);
    balanced_.item().setSelected(idx == 1);
    comfortable_.item().setSelected(idx == 2);
    compact_.refreshFromItem();
    balanced_.refreshFromItem();
    comfortable_.refreshFromItem();
  }

  TextLabel title_;
  TextLabel subtitle_;
  material3::List multi_select_;
  material3::List single_select_;
  material3::List toggle_list_;
  material3::ListRow<material3::CheckboxListItem> notifications_;
  material3::ListRow<material3::CheckboxListItem> freeze_guard_;
  material3::ListRow<material3::RadioListItem> compact_;
  material3::ListRow<material3::RadioListItem> balanced_;
  material3::ListRow<material3::RadioListItem> comfortable_;
  material3::ListRow<material3::SwitchListItem> thermal_lock_;
  material3::ListRow<material3::SwitchListItem> heater_boost_;
};

class ExpandableRowItem : public material3::InvokableListItemBase {
 public:
  ExpandableRowItem(ApplicationContext& context, roo::string_view headline,
                    roo::string_view supporting,
                    roo::string_view expanded_details)
      : material3::InvokableListItemBase(headline, supporting, {}, {}, true),
        details_(context, std::string(expanded_details), font_caption()),
        details_panel_(context) {
    details_panel_.setAnimationDuration(180);
    details_panel_.setExpanded(false, false);
    details_panel_.setContent(WidgetRef(details_));
  }

  Widget* body() override { return &details_panel_; }

 protected:
  void handleInvoke() override {
    details_panel_.setExpanded(!details_panel_.isExpanded());
  }

 private:
  TextLabel details_;
  material3::ExpandablePanel details_panel_;
};

class ExpandableSection : public FlexLayout {
 public:
  explicit ExpandableSection(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn),
        title_(context, "Phase 11 expandable rows", font_body1()),
        subtitle_(context,
                  "ExpandablePanel is reusable body content; expansion state "
                  "stays inside the item, not on ListEntry.",
                  font_caption()),
        expandable_list_(context),
        filter_schedule_(context, "Filter schedule",
                         "Tap to expand maintenance details",
                         "Runs at 06:30 and 18:45 on weekdays.\n"
                         "Skips automatically when freeze guard is active."),
        chemistry_notes_(context, "Chemistry notes",
                         "Tap to expand current plan",
                         "pH target: 7.4\n"
                         "Chlorine target: 2.0 ppm\n"
                         "Retest after evening circulation.") {
    setGap(Scaled(6));

    expandable_list_.add(filter_schedule_);
    expandable_list_.add(chemistry_notes_);

    add(title_, {.flex_grow = 0, .flex_shrink = 0});
    add(subtitle_, {.flex_grow = 0, .flex_shrink = 0});
    add(expandable_list_, {.flex_grow = 0, .flex_shrink = 0});
  }

 private:
  TextLabel title_;
  TextLabel subtitle_;
  material3::List expandable_list_;
  material3::ListRow<ExpandableRowItem> filter_schedule_;
  material3::ListRow<ExpandableRowItem> chemistry_notes_;
};

class ListsScreen : public SimpleScrollablePanel {
 public:
  explicit ListsScreen(ApplicationContext& context)
      : SimpleScrollablePanel(context),
        content_(context, FlexDirection::kColumn),
        title_(context, "Material 3 lists", font_h6()),
        subtitle_(context,
                  "Phase 11 - expandable rows now use reusable body panels "
                  "with animated measured-height reveal",
                  font_caption()),
        top_divider_(context),
        settings_(context),
        middle_divider_(context),
        adopted_(context),
        bottom_divider_(context),
        convenience_(context),
        final_divider_(context),
        navigation_(context),
        selection_divider_(context),
        selection_(context),
        expandable_divider_(context),
        expandable_(context),
        menu_divider_(context),
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
    content_.add(navigation_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(selection_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(selection_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(expandable_divider_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(expandable_, {.flex_grow = 0, .flex_shrink = 0});
    content_.add(menu_divider_, {.flex_grow = 0, .flex_shrink = 0});
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
  NavigationSection navigation_;
  HorizontalDivider selection_divider_;
  SelectionSection selection_;
  HorizontalDivider expandable_divider_;
  ExpandableSection expandable_;
  HorizontalDivider menu_divider_;
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