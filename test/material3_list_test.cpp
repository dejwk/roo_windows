#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "gtest/gtest.h"
#include "roo_icons/filled/24/device.h"
#include "roo_icons/outlined/24/notification.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace material3 {
namespace {

static_assert(std::is_base_of<Container, ListEntry>::value,
              "ListEntry must keep the fixed-child Container boundary");
static_assert(std::is_base_of<ListEntry, ListRow<StandardListItem>>::value,
              "ListRow must remain a thin ListEntry adapter");
static_assert(std::is_base_of<Container, List>::value,
              "List must remain a public widget container");
static_assert(!std::is_base_of<FlexLayout, List>::value,
              "List must not leak FlexLayout in its public type");
static_assert(std::is_convertible<List*, Widget*>::value,
              "List must remain composable as a Widget");
static_assert(!std::is_convertible<List*, FlexLayout*>::value,
              "List callers must not be able to use FlexLayout APIs");
static_assert(std::is_base_of<ListItem, StandardListItem>::value,
              "StandardListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, HeadlineListItem>::value,
              "HeadlineListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, SupportingTextListItem>::value,
              "SupportingTextListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, PictogramSupportingTextItem>::value,
              "PictogramSupportingTextItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, AvatarSupportingTextItem>::value,
              "AvatarSupportingTextItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, NavigationListItem>::value,
              "NavigationListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, AvatarNavigationListItem>::value,
              "AvatarNavigationListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, CheckboxListItem>::value,
              "CheckboxListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, RadioListItem>::value,
              "RadioListItem must remain a ListItem descriptor");
static_assert(std::is_base_of<ListItem, SwitchListItem>::value,
              "SwitchListItem must remain a ListItem descriptor");

class TestWidget : public BasicWidget {
 public:
  explicit TestWidget(ApplicationContext& context, Dimensions dimensions = {})
      : BasicWidget(context), dimensions_(dimensions) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return dimensions_;
  }

 private:
  Dimensions dimensions_;
};

class CountingMeasureWidget : public BasicWidget {
 public:
  CountingMeasureWidget(ApplicationContext& context, Dimensions suggested,
                        Dimensions measured)
      : BasicWidget(context),
        suggested_(suggested),
        measured_(measured),
        measure_count_(0) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return suggested_;
  }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    ++measure_count_;
    return Dimensions(width.resolveSize(measured_.width()),
                      height.resolveSize(measured_.height()));
  }

  int measureCount() const { return measure_count_; }

 private:
  Dimensions suggested_;
  Dimensions measured_;
  int measure_count_;
};

class TestListEntry : public ListEntry {
 public:
  explicit TestListEntry(ApplicationContext& context) : ListEntry(context) {}

  using ListEntry::getChild;
  using ListEntry::getChildrenCount;
};

class TestStandardListRow : public ListRow<StandardListItem> {
 public:
  template <typename... Args>
  explicit TestStandardListRow(ApplicationContext& context, Args&&... args)
      : ListRow<StandardListItem>(context, std::forward<Args>(args)...) {}

  using ListEntry::getChild;
  using ListEntry::getChildrenCount;
};

template <typename Item>
class TestListRow : public ListRow<Item> {
 public:
  template <typename... Args>
  explicit TestListRow(ApplicationContext& context, Args&&... args)
      : ListRow<Item>(context, std::forward<Args>(args)...) {}

  using ListEntry::getChild;
  using ListEntry::getChildrenCount;
};

class TestList : public List {
 public:
  explicit TestList(ApplicationContext& context) : List(context) {}

  using List::getChild;
  using List::getChildrenCount;
};

class TestExpandablePanel : public ExpandablePanel {
 public:
  explicit TestExpandablePanel(ApplicationContext& context)
      : ExpandablePanel(context) {}

  using ExpandablePanel::getChild;
  using ExpandablePanel::getChildrenCount;
};

class TrackingListEntry : public ListEntry {
 public:
  TrackingListEntry(ApplicationContext& context, bool& destroyed)
      : ListEntry(context), destroyed_(destroyed) {}

  ~TrackingListEntry() override { destroyed_ = true; }

 private:
  bool& destroyed_;
};

class MutableListItem : public ListItem {
 public:
  explicit MutableListItem(roo_display::StringView headline)
      : headline_(headline) {}

  roo_display::StringView headlineText() const override { return headline_; }

  void setHeadline(roo_display::StringView headline) { headline_ = headline; }

 private:
  roo_display::StringView headline_;
};

class MutablePolicyListItem : public ListItem {
 public:
  MutablePolicyListItem(roo_display::StringView headline,
                        roo_display::StringView supporting)
      : headline_(headline),
        supporting_(supporting),
        headline_policy_(),
        supporting_policy_() {}

  roo_display::StringView headlineText() const override { return headline_; }

  roo_display::StringView supportingText() const override {
    return supporting_;
  }

  ListTextPolicy headlinePolicy() const override { return headline_policy_; }

  ListTextPolicy supportingPolicy() const override {
    return supporting_policy_;
  }

  void setHeadlineText(roo_display::StringView text) { headline_ = text; }

  void setSupportingText(roo_display::StringView text) { supporting_ = text; }

  void setHeadlinePolicy(ListTextPolicy policy) { headline_policy_ = policy; }

  void setSupportingPolicy(ListTextPolicy policy) {
    supporting_policy_ = policy;
  }

 private:
  roo_display::StringView headline_;
  roo_display::StringView supporting_;
  ListTextPolicy headline_policy_;
  ListTextPolicy supporting_policy_;
};

class ExpandableBodyListItem : public InvokableListItemBase {
 public:
  ExpandableBodyListItem(ApplicationContext& context, roo::string_view headline,
                         roo::string_view supporting)
      : InvokableListItemBase(headline, supporting, {}, {}, true),
        body_content_(context, Dimensions(96, 24)),
        body_panel_(context) {
    body_panel_.setAnimationDuration(120);
    body_panel_.setExpanded(false, false);
    body_panel_.setContent(WidgetRef(body_content_));
  }

  Widget* body() override { return &body_panel_; }

  ExpandablePanel& bodyPanel() { return body_panel_; }

 protected:
  void handleInvoke() override {
    body_panel_.setExpanded(!body_panel_.isExpanded());
  }

 private:
  TestWidget body_content_;
  ExpandablePanel body_panel_;
};

class Material3ListRenderTest
    : public test_support::RooWindowsRenderTestSized<180, 140> {};

// Verifies that the public policy structs start from the Phase 1 design
// defaults and stay compact enough to be stored inline.
TEST(Material3List, PolicyDefaultsMatchDesign) {
  ListTextPolicy text_policy;
  EXPECT_EQ(TextOverflowPolicy::kTruncate, text_policy.overflow);
  EXPECT_EQ(1, text_policy.max_lines);

  ListSelectionPolicy selection_policy;
  EXPECT_EQ(SelectionMode::kNone, selection_policy.mode);
  EXPECT_EQ(SelectionAffordance::kNone, selection_policy.affordance);
  EXPECT_EQ(AffordancePlacement::kTrailing, selection_policy.placement);
  EXPECT_TRUE(selection_policy.selection_follows_press);

  ListDividerPolicy divider_policy;
  EXPECT_EQ(DividerMode::kNone, divider_policy.mode);
  EXPECT_EQ(0, divider_policy.start_inset);
  EXPECT_EQ(0, divider_policy.end_inset);
  EXPECT_TRUE(divider_policy.suppress_between_selected);

  ListEntryVisualContext visual_context;
  EXPECT_EQ(ListVariant::kExpressive, visual_context.variant);
  EXPECT_EQ(ListStyle::kStandard, visual_context.style);
  EXPECT_EQ(ListItemPosition::kSingle, visual_context.position);
  EXPECT_FALSE(visual_context.selected);
  EXPECT_TRUE(visual_context.enabled);
  EXPECT_FALSE(visual_context.pressed);
  EXPECT_FALSE(visual_context.focused);
  EXPECT_FALSE(visual_context.hovered);
  EXPECT_FALSE(visual_context.show_divider);
  EXPECT_EQ(DividerMode::kNone, visual_context.divider_mode);
  EXPECT_EQ(0, visual_context.divider_start_inset);
  EXPECT_EQ(0, visual_context.divider_end_inset);

  EXPECT_LE(sizeof(ListTextPolicy), 2U);
  EXPECT_LE(sizeof(ListSelectionPolicy), 4U);
  EXPECT_LE(sizeof(ListDividerPolicy), 4U);
  EXPECT_LE(sizeof(DividerInsetHint), 2U);
  EXPECT_LE(sizeof(ListEntryVisualContext), 12U);
}

// Verifies that the convenience init builders populate only descriptor data,
// without requiring any row widget behavior.
TEST(Material3List, StandardListItemInitPresetsPopulateDescriptorData) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestWidget leading(context);
  TestWidget trailing(context);
  TestWidget body(context);

  StandardListItemInit one_line =
      StandardListItemInit::OneLine("WiFi", &leading, &trailing);
  EXPECT_EQ("WiFi", std::string(one_line.headline));
  EXPECT_TRUE(one_line.supporting.empty());
  EXPECT_EQ(&leading, one_line.leading);
  EXPECT_EQ(&trailing, one_line.trailing);
  EXPECT_EQ(nullptr, one_line.body);
  EXPECT_EQ(1, one_line.headline_policy.max_lines);
  EXPECT_EQ(VerticalVisualAlignment::kMiddle, one_line.leading_alignment);

  StandardListItemInit two_line =
      StandardListItemInit::TwoLine("Pump", "Auto", &leading, &trailing);
  EXPECT_EQ("Pump", std::string(two_line.headline));
  EXPECT_EQ("Auto", std::string(two_line.supporting));
  EXPECT_EQ(1, two_line.supporting_policy.max_lines);
  EXPECT_FALSE(two_line.prefer_top_text_alignment);

  StandardListItemInit three_line = StandardListItemInit::ThreeLine(
      "Filter", "Last cleaned today", "Status", &leading, &trailing, &body);
  EXPECT_EQ("Status", std::string(three_line.overline));
  EXPECT_EQ(&body, three_line.body);
  EXPECT_EQ(1, three_line.supporting_policy.max_lines);
  EXPECT_EQ(VerticalVisualAlignment::kTop, three_line.leading_alignment);
  EXPECT_EQ(VerticalVisualAlignment::kTop, three_line.trailing_alignment);
  EXPECT_TRUE(three_line.prefer_top_text_alignment);
}

// Verifies that StandardListItem is construction-time configured and mirrors
// the stable borrowed widgets from its init descriptor.
TEST(Material3List, StandardListItemMirrorsInitDescriptor) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestWidget leading(context);
  TestWidget trailing(context);

  StandardListItemInit init =
      StandardListItemInit::TwoLine("Pool pump", "Auto", &leading, &trailing);
  init.divider_inset_hint.start_inset = 12;
  init.divider_inset_hint.end_inset = 4;
  StandardListItem item(init);

  EXPECT_EQ("Pool pump", std::string(item.headlineText()));
  EXPECT_EQ("Auto", std::string(item.supportingText()));
  EXPECT_TRUE(item.overlineText().empty());
  EXPECT_EQ(1, item.headlinePolicy().max_lines);
  EXPECT_EQ(1, item.supportingPolicy().max_lines);
  EXPECT_EQ(&leading, item.leading());
  EXPECT_EQ(&trailing, item.trailing());
  EXPECT_EQ(nullptr, item.body());
  EXPECT_EQ(&leading, item.leadingWidget());
  EXPECT_EQ(&trailing, item.trailingWidget());
  EXPECT_EQ(12, item.dividerInsetHint().start_inset);
  EXPECT_EQ(4, item.dividerInsetHint().end_inset);
  EXPECT_FALSE(item.preferTopTextAlignment());
}

// Verifies that phase 8 convenience items keep text data lightweight while
// exposing only the stable leading visual they own.
TEST(Material3List, ConvenienceItemsExposeCompactListItemState) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  ListTextPolicy wrapped_supporting;
  wrapped_supporting.overflow = TextOverflowPolicy::kWrap;
  wrapped_supporting.max_lines = 2;

  HeadlineListItem headline("Owner");
  SupportingTextListItem supporting("Pool pump", "Automatic", {},
                                    wrapped_supporting);
  PictogramSupportingTextItem pictogram(
      context, ic_filled_24_device_wifi_tethering(), "Roof loop",
      "Connected over the shed bridge", {}, wrapped_supporting);
  AvatarSupportingTextItem avatar(context, "DW", "Dawid Wojcik", "Pool owner",
                                  {}, wrapped_supporting);

  EXPECT_EQ("Owner", std::string(headline.headlineText()));
  EXPECT_EQ("Pool pump", std::string(supporting.headlineText()));
  EXPECT_EQ("Automatic", std::string(supporting.supportingText()));
  EXPECT_EQ(2, supporting.supportingPolicy().max_lines);
  EXPECT_EQ(&pictogram.leadingIcon(), pictogram.leading());
  EXPECT_EQ("DW", std::string(avatar.initials()));
  ASSERT_NE(nullptr, avatar.leading());
  Dimensions avatar_size = avatar.leading()->measure(
      WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(Scaled(40), avatar_size.width());
  EXPECT_EQ(Scaled(40), avatar_size.height());
}

// Verifies that convenience items bind through the existing ListEntry slot
// contract without introducing any new row abstraction.
TEST(Material3List, ConvenienceItemsBindThroughListEntry) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestListEntry entry(context);

  PictogramSupportingTextItem pictogram(
      context, ic_outlined_24_notification_sync(), "Schedule sync",
      "Last successful refresh this morning");
  entry.setItem(pictogram);

  ASSERT_EQ(3, entry.getChildrenCount());
  EXPECT_EQ(&entry, pictogram.leading()->parent());

  entry.clearItem();
  EXPECT_EQ(nullptr, pictogram.leading()->parent());

  AvatarSupportingTextItem avatar(context, "DW", "Dawid Wojcik", "Pool owner");
  entry.setItem(avatar);

  ASSERT_EQ(3, entry.getChildrenCount());
  EXPECT_EQ(&entry, avatar.leading()->parent());

  entry.clearItem();
  EXPECT_EQ(nullptr, avatar.leading()->parent());
}

// Verifies that ListRow still bridges inline-owned items when the item type
// needs the row's ApplicationContext to build its owned leading visual.
TEST(Material3List, ListRowConstructsContextAwareConvenienceItems) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  TestListRow<AvatarSupportingTextItem> avatar_row(
      context, "DW", "Dawid Wojcik", "Pool owner");
  TestListRow<PictogramSupportingTextItem> pictogram_row(
      context, ic_outlined_24_notification_sync(), "Schedule sync",
      "Refreshed from the controller");

  EXPECT_TRUE(avatar_row.hasItem());
  EXPECT_TRUE(pictogram_row.hasItem());
  EXPECT_EQ("DW", std::string(avatar_row.item().initials()));
  EXPECT_EQ("Schedule sync", std::string(pictogram_row.item().headlineText()));
  EXPECT_EQ(3, avatar_row.getChildrenCount());
  EXPECT_EQ(3, pictogram_row.getChildrenCount());
}

// Verifies that rows become clickable only when the bound item exposes
// invocation behavior.
TEST(Material3List, ListEntryClickabilityDependsOnBoundItemInvocation) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  TestListRow<NavigationListItem> navigation_row(
      context, ic_filled_24_device_wifi_tethering(), "Device details",
      "Inspect current connectivity state");
  EXPECT_FALSE(navigation_row.isClickable());

  int invocation_count = 0;
  navigation_row.item().setOnInvoked([&]() { ++invocation_count; });
  EXPECT_TRUE(navigation_row.isClickable());

  navigation_row.onClicked();
  EXPECT_EQ(1, invocation_count);

  TestStandardListRow standard_row(
      context, StandardListItemInit::OneLine("Standard row"));
  EXPECT_FALSE(standard_row.isClickable());
}

// Verifies that Phase 9 navigation convenience items keep callback storage in
// opt-in item types while exposing stable leading/trailing affordances.
TEST(Material3List, NavigationConvenienceItemsExposeInvokeSurface) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  NavigationListItem navigation(context, ic_outlined_24_notification_sync(),
                                "Schedule sync", "Open sync details");
  EXPECT_EQ("Schedule sync", std::string(navigation.headlineText()));
  EXPECT_EQ("Open sync details", std::string(navigation.supportingText()));
  ASSERT_NE(nullptr, navigation.leading());
  ASSERT_NE(nullptr, navigation.trailing());
  EXPECT_EQ(&navigation.leadingIcon(), navigation.leading());
  EXPECT_EQ(&navigation.trailingAffordance(), navigation.trailing());

  int navigation_invocations = 0;
  navigation.setOnInvoked([&]() { ++navigation_invocations; });
  navigation.invoke();
  EXPECT_EQ(1, navigation_invocations);

  AvatarNavigationListItem avatar_navigation(context, "DW", "Assigned owner",
                                             "Open schedule details");
  EXPECT_EQ("DW", std::string(avatar_navigation.initials()));
  ASSERT_NE(nullptr, avatar_navigation.leading());
  ASSERT_NE(nullptr, avatar_navigation.trailing());
  EXPECT_EQ(&avatar_navigation.trailingAffordance(),
            avatar_navigation.trailing());

  int avatar_invocations = 0;
  avatar_navigation.setOnInvoked([&]() { ++avatar_invocations; });
  avatar_navigation.invoke();
  EXPECT_EQ(1, avatar_invocations);
}

// Verifies that row click delegation and direct item invocation follow the
// same callback path for navigation convenience rows.
TEST(Material3List, NavigationRowsDelegateRowClickToItemInvokePath) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestListRow<AvatarNavigationListItem> row(context, "DW", "Assigned owner",
                                            "Open schedule details");

  int invocation_count = 0;
  row.item().setOnInvoked([&]() { ++invocation_count; });

  EXPECT_TRUE(row.isClickable());
  row.onClicked();
  row.item().invoke();

  EXPECT_EQ(2, invocation_count);
}

// Verifies that row invocation starts at tap-up confirmation so state changes
// can run concurrently with click animation, and deferred onClicked does not
// invoke the item a second time.
TEST(Material3List, NavigationRowsInvokeOnTapUpWithoutDoubleInvokeOnClick) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestListRow<AvatarNavigationListItem> row(context, "DW", "Assigned owner",
                                            "Open schedule details");

  int invocation_count = 0;
  row.item().setOnInvoked([&]() { ++invocation_count; });

  EXPECT_TRUE(row.isClickable());
  EXPECT_EQ(0, invocation_count);

  EXPECT_TRUE(row.onSingleTapUp(0, 0));
  EXPECT_EQ(1, invocation_count);

  row.onClicked();
  EXPECT_EQ(1, invocation_count);
}

// Verifies that Phase 10 selection convenience items expose semantic state
// APIs and support configurable leading/trailing control placement.
TEST(Material3List, SelectionConvenienceItemsExposeSemanticStateAndPlacement) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  CheckboxListItem checkbox_item(context, "Notifications", "Row toggles",
                                 Checkbox::OnOffState::kOff,
                                 AffordancePlacement::kTrailing);
  EXPECT_EQ(nullptr, checkbox_item.leading());
  EXPECT_EQ(&checkbox_item.checkbox(), checkbox_item.trailing());
  EXPECT_FALSE(checkbox_item.isChecked());
  checkbox_item.setChecked(true);
  EXPECT_TRUE(checkbox_item.isChecked());
  checkbox_item.setCheckedState(Checkbox::OnOffState::kIndeterminate);
  EXPECT_EQ(Checkbox::OnOffState::kIndeterminate, checkbox_item.checkedState());
  checkbox_item.setAffordancePlacement(AffordancePlacement::kLeading);
  EXPECT_EQ(&checkbox_item.checkbox(), checkbox_item.leading());
  EXPECT_EQ(nullptr, checkbox_item.trailing());

  RadioListItem radio_item(context, "Balanced", "Single-select choice", true,
                           AffordancePlacement::kLeading);
  EXPECT_EQ(&radio_item.radioButton(), radio_item.leading());
  EXPECT_EQ(nullptr, radio_item.trailing());
  EXPECT_TRUE(radio_item.isSelected());
  radio_item.setSelected(false);
  EXPECT_FALSE(radio_item.isSelected());
  radio_item.setAffordancePlacement(AffordancePlacement::kTrailing);
  EXPECT_EQ(nullptr, radio_item.leading());
  EXPECT_EQ(&radio_item.radioButton(), radio_item.trailing());

  SwitchListItem switch_item(context, "Thermal lock", "Toggle row", false,
                             AffordancePlacement::kTrailing);
  EXPECT_FALSE(switch_item.isOn());
  switch_item.setOn(true);
  EXPECT_TRUE(switch_item.isOn());
  switch_item.toggle();
  EXPECT_FALSE(switch_item.isOn());
}

// Verifies that selection convenience rows are clickable without requiring a
// callback and that row press toggles/selects the embedded affordance state.
TEST(Material3List, SelectionRowsDelegateRowClickToAffordanceState) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  TestListRow<CheckboxListItem> checkbox_row(context, "Notifications",
                                             "Row toggles checkbox");
  EXPECT_TRUE(checkbox_row.isClickable());
  EXPECT_FALSE(checkbox_row.item().isChecked());
  checkbox_row.onClicked();
  EXPECT_TRUE(checkbox_row.item().isChecked());

  TestListRow<RadioListItem> radio_row(context, "Balanced", "Row selects radio",
                                       false);
  EXPECT_TRUE(radio_row.isClickable());
  EXPECT_FALSE(radio_row.item().isSelected());
  radio_row.onClicked();
  EXPECT_TRUE(radio_row.item().isSelected());

  TestListRow<SwitchListItem> switch_row(context, "Thermal lock",
                                         "Row toggles switch", false);
  EXPECT_TRUE(switch_row.isClickable());
  EXPECT_FALSE(switch_row.item().isOn());
  switch_row.onClicked();
  EXPECT_TRUE(switch_row.item().isOn());
}

// Verifies that checkbox and radio affordance taps trigger the same
// invoke callback path used by row clicks.
TEST(Material3List, SelectionAffordanceTapUsesSameInvokeCallbackPath) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  CheckboxListItem checkbox_item(context, "Notifications", "Track invokes");
  int checkbox_invocations = 0;
  checkbox_item.setOnInvoked([&]() { ++checkbox_invocations; });
  checkbox_item.invoke();
  checkbox_item.checkbox().onClicked();
  EXPECT_EQ(2, checkbox_invocations);

  RadioListItem radio_item(context, "Balanced", "Track invokes");
  int radio_invocations = 0;
  radio_item.setOnInvoked([&]() { ++radio_invocations; });
  radio_item.invoke();
  radio_item.radioButton().onClicked();
  EXPECT_EQ(2, radio_invocations);
}

// Verifies that the Phase 1 widgets can be constructed with safe default
// state before row layout, child binding, and expansion behavior are added.
TEST(Material3List, BaselineClassesConstructWithSafeDefaults) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());

  ListEntry entry(context);
  EXPECT_FALSE(entry.hasItem());
  EXPECT_EQ(nullptr, entry.item());
  EXPECT_EQ(ListVariant::kExpressive, entry.visualContext().variant);
  EXPECT_EQ(ListItemPosition::kSingle, entry.visualContext().position);

  ListEntryVisualContext selected_context;
  selected_context.selected = true;
  selected_context.position = ListItemPosition::kFirst;
  entry.setVisualContext(selected_context);
  EXPECT_TRUE(entry.visualContext().selected);
  EXPECT_EQ(ListItemPosition::kFirst, entry.visualContext().position);

  ExpandablePanel panel(context);
  EXPECT_FALSE(panel.isExpanded());
  EXPECT_FALSE(panel.isAnimating());
  panel.clearContent();
  panel.setExpanded(false);
  panel.setAnimationDuration(180);

  List list(context);
  WidgetRef borrowed_list(list);
  EXPECT_EQ(&list, borrowed_list.get());
  list.clear();
}

// Verifies that ExpandablePanel owns a single optional body child, reports
// collapsed vs expanded measured height, and allows replacing content.
TEST(Material3List, ExpandablePanelAttachesContentAndResolvesHeight) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestExpandablePanel panel(context);
  TestWidget first_content(context, Dimensions(90, 28));
  TestWidget second_content(context, Dimensions(84, 16));

  panel.setAnimationDuration(120);
  panel.setContent(WidgetRef(first_content));
  ASSERT_EQ(1, panel.getChildrenCount());
  EXPECT_EQ(&first_content, &panel.getChild(0));

  panel.setExpanded(false, false);
  Dimensions collapsed =
      panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(0, collapsed.height());

  panel.setExpanded(true, false);
  Dimensions expanded =
      panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(28, expanded.height());

  panel.setContent(WidgetRef(second_content));
  ASSERT_EQ(1, panel.getChildrenCount());
  EXPECT_EQ(&second_content, &panel.getChild(0));
  EXPECT_EQ(nullptr, first_content.parent());

  panel.clearContent();
  EXPECT_EQ(0, panel.getChildrenCount());
}

// Verifies that ExpandablePanel progresses through clipped intermediate
// measured heights while animating between collapsed and expanded states.
TEST(Material3List, ExpandablePanelAnimationProducesIntermediateHeights) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  ExpandablePanel panel(context);
  TestWidget content(context, Dimensions(80, 30));

  panel.setAnimationDuration(100);
  panel.setContent(WidgetRef(content));
  panel.setExpanded(false, false);
  panel.setExpanded(true, true);

  Dimensions step1 =
      panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  Dimensions step2 =
      panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  Dimensions step3 =
      panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_TRUE(panel.isAnimating());
  EXPECT_GT(step1.height(), 0);
  EXPECT_GT(step2.height(), step1.height());
  EXPECT_GT(step3.height(), step2.height());
  EXPECT_LT(step3.height(), 30);

  for (int i = 0; i < 6; ++i) {
    panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  }
  EXPECT_FALSE(panel.isAnimating());
  Dimensions expanded =
      panel.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(30, expanded.height());
}

// Verifies that ExpandablePanel keeps body child layout clipped to the
// currently visible animated height, preventing child spill beyond panel
// bounds while expanding.
TEST(Material3List, ExpandablePanelClipsChildLayoutToVisibleHeight) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestExpandablePanel panel(context);
  TestWidget content(context, Dimensions(90, 36));

  panel.setAnimationDuration(120);
  panel.setContent(WidgetRef(content));
  panel.setExpanded(false, false);
  panel.setExpanded(true, true);

  Dimensions first =
      panel.measure(WidthSpec::Exactly(100), HeightSpec::Unspecified(0));
  panel.layout(Rect(0, 0, first.width() - 1, first.height() - 1));

  ASSERT_EQ(1, panel.getChildrenCount());
  EXPECT_EQ(first.height(), panel.getChild(0).height());
}

// Verifies that expandable-row usage keeps expansion state in item-owned body
// content while ListEntry remains a reusable row surface.
TEST(Material3List, ExpandableBodyItemTogglesPanelThroughRowInvocation) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestListRow<ExpandableBodyListItem> row(context, "Filter schedule",
                                          "Tap to expand details");

  EXPECT_TRUE(row.isClickable());
  Dimensions collapsed =
      row.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));

  row.onClicked();
  for (int i = 0; i < 8; ++i) {
    row.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  }
  EXPECT_TRUE(row.item().bodyPanel().isExpanded());
  EXPECT_FALSE(row.item().bodyPanel().isAnimating());

  Dimensions expanded =
      row.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  EXPECT_GT(expanded.height(), collapsed.height());
}

// Verifies that tapping an expandable rounded row and releasing before the
// click animation completes never leaves residual ripple pixels on the parent
// background once both the click animation and the concurrent expansion have
// fully settled. The settled framebuffer must match a forced clean repaint of
// the same final state (which models the "remains until scroll" symptom: a
// full repaint clears the stray pixels).
TEST_F(Material3ListRenderTest, ExpandableRowRippleLeavesNoResidueAfterSettle) {
  using test_support::ColorBoxWidget;

  // Mirror the example scene: a scrollable panel whose content is taller than
  // the viewport, with the expandable list sitting at a non-zero scroll
  // offset (so the row paints at a translated device origin).
  auto scroll = std::make_unique<SimpleScrollablePanel>(context());
  SimpleScrollablePanel* scroll_ptr = scroll.get();

  auto content = std::make_unique<FlexLayout>(context(), FlexDirection::kColumn);
  FlexLayout* content_ptr = content.get();
  // Match the example screen: horizontal padding leaves a parent-background
  // margin to the left/right of the (full width) list rows, so any ripple that
  // overshoots the row's rounded silhouette lands on a visible parent surface.
  content_ptr->setPadding(Padding(Scaled(12), Scaled(8)));
  content_ptr->setGap(Scaled(8));

  auto top_filler = std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::Blue, Dimensions(kWidth, 100));
  auto list = std::make_unique<List>(context());
  List* list_ptr = list.get();
  auto row1 = std::make_unique<ListRow<ExpandableBodyListItem>>(
      context(), "Filter schedule", "Tap to expand details");
  auto row2 = std::make_unique<ListRow<ExpandableBodyListItem>>(
      context(), "Chemistry notes", "Tap to expand details");
  ListRow<ExpandableBodyListItem>* row1_ptr = row1.get();
  list_ptr->add(std::move(row1));
  list_ptr->add(std::move(row2));
  auto bottom_filler = std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::Green, Dimensions(kWidth, 100));

  content_ptr->add(WidgetRef(std::move(top_filler)),
                   {.flex_grow = 0, .flex_shrink = 0});
  content_ptr->add(WidgetRef(std::move(list)),
                   {.flex_grow = 0, .flex_shrink = 0});
  content_ptr->add(WidgetRef(std::move(bottom_filler)),
                   {.flex_grow = 0, .flex_shrink = 0});

  scroll_ptr->setContents(WidgetRef(std::move(content)));
  app_.add(std::move(scroll), roo_display::Box(0, 0, kWidth - 1, kHeight - 1));

  ASSERT_TRUE(refresh());

  // Scroll so the expandable list sits near the top of the viewport.
  scroll_ptr->scrollTo(0, 0);
  ASSERT_TRUE(refresh());

  XDim row_lx;
  YDim row_ly;
  row1_ptr->getAbsoluteOffset(row_lx, row_ly);
  // Bring row1's top near the top of the viewport (content origin shifts so
  // the row paints at a non-zero scroll translation).
  scroll_ptr->scrollTo(0, 16 - row_ly);
  ASSERT_TRUE(refresh());

  // Capture a PRISTINE reference of the final (expanded) layout produced with
  // no click animation. Expanding directly drives the normal layout
  // invalidation path on a residue-free framebuffer, so this image cannot
  // contain any click-overlay leftovers. Comparing against this (rather than
  // against a post-gesture invalidate) catches residue that lands outside the
  // reach of invalidateInterior().
  row1_ptr->item().bodyPanel().setExpanded(true, false);
  scroll_ptr->invalidateInterior();
  content_ptr->invalidateInterior();
  list_ptr->invalidateInterior();
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(row1_ptr->item().bodyPanel().isExpanded());
  ASSERT_FALSE(row1_ptr->item().bodyPanel().isAnimating());
  std::vector<roo_display::Color> expanded_clean(kWidth * kHeight);
  for (int16_t y = 0; y < kHeight; ++y)
    for (int16_t x = 0; x < kWidth; ++x)
      expanded_clean[y * kWidth + x] = pixelAt(x, y);

  // Collapse again (no animation) to set up the real gesture.
  row1_ptr->item().bodyPanel().setExpanded(false, false);
  scroll_ptr->invalidateInterior();
  content_ptr->invalidateInterior();
  list_ptr->invalidateInterior();
  ASSERT_TRUE(refresh());

  // Quick release: a genuinely fast tap where the gesture detector never
  // emits onShowPress (the press timeout has not elapsed). onSingleTapUp then
  // takes the quick-release branch (setClicking + start animation) without the
  // widget ever entering the pressed state. The click animation runs to
  // completion concurrently with the expansion it triggers. (A longer press
  // fires onShowPress and paints several ripple frames at the steady collapsed
  // size first, and does not reproduce.)
  const XDim press_x = Scaled(10);
  const YDim press_y = Scaled(10);
  row1_ptr->onSingleTapUp(press_x, press_y);

  // Drive the click animation and expansion to completion.
  for (int frame = 0; frame < 28; ++frame) {
    delay(16);
    app_.root().refreshClickAnimation();
    ASSERT_TRUE(refresh());
  }
  ASSERT_TRUE(row1_ptr->item().bodyPanel().isExpanded());
  ASSERT_FALSE(row1_ptr->item().bodyPanel().isAnimating());

  // Capture the settled framebuffer (may contain stale ripple pixels).
  std::vector<roo_display::Color> settled(kWidth * kHeight);
  for (int16_t y = 0; y < kHeight; ++y)
    for (int16_t x = 0; x < kWidth; ++x)
      settled[y * kWidth + x] = pixelAt(x, y);

  int mismatches = 0;
  int first_x = -1;
  int first_y = -1;
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      if (settled[y * kWidth + x] != expanded_clean[y * kWidth + x]) {
        if (mismatches == 0) {
          first_x = x;
          first_y = y;
        }
        ++mismatches;
      }
    }
  }

  EXPECT_EQ(0, mismatches)
      << "Settled framebuffer differs from pristine expanded image by "
      << mismatches << " pixels; first mismatch at (" << first_x << ","
      << first_y << ") settled=0x" << std::hex
      << (first_x < 0 ? 0 : settled[first_y * kWidth + first_x].asArgb())
      << " clean=0x"
      << (first_x < 0 ? 0 : expanded_clean[first_y * kWidth + first_x].asArgb())
      << std::dec;
}

// Verifies that binding a standard item attaches the stable borrowed slot
// widgets plus row-owned text labels, and clearing the item detaches them.
TEST(Material3List, ListEntryBindsAndClearsStableSlotChildren) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestWidget leading(context, Dimensions(20, 20));
  TestWidget trailing(context, Dimensions(12, 16));
  TestWidget body(context, Dimensions(80, 10));
  StandardListItem item(StandardListItemInit::ThreeLine(
      "Headline", "Supporting", "Overline", &leading, &trailing, &body));
  TestListEntry entry(context);

  entry.setItem(item);

  EXPECT_TRUE(entry.hasItem());
  EXPECT_EQ(&item, entry.item());
  EXPECT_EQ(6, entry.getChildrenCount());
  EXPECT_EQ(&entry, leading.parent());
  EXPECT_EQ(&entry, trailing.parent());
  EXPECT_EQ(&entry, body.parent());

  entry.clearItem();

  EXPECT_FALSE(entry.hasItem());
  EXPECT_EQ(nullptr, entry.item());
  EXPECT_EQ(0, entry.getChildrenCount());
  EXPECT_EQ(nullptr, leading.parent());
  EXPECT_EQ(nullptr, trailing.parent());
  EXPECT_EQ(nullptr, body.parent());
}

// Verifies that a measured and laid-out row positions leading and trailing
// widgets according to the default Material 3 row padding and alignment.
TEST(Material3List, ListEntryMeasuresAndLaysOutSlots) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestWidget leading(context, Dimensions(20, 20));
  TestWidget trailing(context, Dimensions(12, 16));
  StandardListItem item(
      StandardListItemInit::OneLine("Headline", &leading, &trailing));
  TestListEntry entry(context);
  entry.setItem(item);

  Dimensions measured =
      entry.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  EXPECT_EQ(180, measured.width());
  EXPECT_GE(measured.height(), Scaled(56));

  entry.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));

  EXPECT_EQ(Scaled(16), leading.offsetLeft());
  EXPECT_EQ((measured.height() - leading.height()) / 2, leading.offsetTop());
  EXPECT_EQ(180 - Scaled(16) - trailing.width(), trailing.offsetLeft());
  EXPECT_EQ((measured.height() - trailing.height()) / 2, trailing.offsetTop());
}

// Verifies that the const suggested-minimum query stays on the cheap,
// non-measuring path and does not trigger child measure side effects.
TEST(Material3List, ListEntrySuggestedMinimumDoesNotMeasureBoundSlots) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  CountingMeasureWidget leading(context, Dimensions(10, 8), Dimensions(40, 30));
  CountingMeasureWidget trailing(context, Dimensions(7, 6), Dimensions(35, 20));
  CountingMeasureWidget body(context, Dimensions(9, 5), Dimensions(60, 18));
  StandardListItemInit init;
  init.leading = &leading;
  init.trailing = &trailing;
  init.body = &body;
  StandardListItem item(init);
  TestListEntry entry(context);
  entry.setItem(item);

  const ListEntry& const_entry = entry;
  Dimensions suggested = const_entry.getSuggestedMinimumDimensions();

  EXPECT_EQ(0, leading.measureCount());
  EXPECT_EQ(0, trailing.measureCount());
  EXPECT_EQ(0, body.measureCount());
  EXPECT_GT(suggested.width(), 0);
  EXPECT_GE(suggested.height(), Scaled(56));
}

// Verifies that refreshFromItem rereads lightweight descriptor state while
// preserving the already attached slot widget identity and row-owned labels.
TEST(Material3List, ListEntryRefreshesMutableTextWithoutReplacingSlots) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  MutableListItem item("A");
  TestListEntry entry(context);
  entry.setItem(item);

  Dimensions short_measure =
      entry.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  item.setHeadline("A much longer headline");
  entry.refreshFromItem();
  Dimensions long_measure =
      entry.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_GT(long_measure.width(), short_measure.width());
  EXPECT_EQ(1, entry.getChildrenCount());
}

// Verifies that one-line truncate policies keep the cheap label slot and do
// not replace the text child when only text content changes.
TEST(Material3List, OneLineTruncateRefreshKeepsExistingTextWidget) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  MutablePolicyListItem item("Headline", "");
  ListTextPolicy headline_policy;
  headline_policy.overflow = TextOverflowPolicy::kTruncate;
  headline_policy.max_lines = 1;
  item.setHeadlinePolicy(headline_policy);
  TestListEntry entry(context);

  entry.setItem(item);
  ASSERT_EQ(1, entry.getChildrenCount());
  const Widget* first_child = &entry.getChild(0);

  item.setHeadlineText("Updated headline value");
  entry.refreshFromItem();

  ASSERT_EQ(1, entry.getChildrenCount());
  const Widget* refreshed_child = &entry.getChild(0);
  EXPECT_EQ(first_child, refreshed_child);
}

// Verifies that changing text policy class through refresh swaps the slot
// widget from one-line label mode to block-text mode.
TEST(Material3List, RefreshSwitchesTextWidgetClassWhenPolicyClassChanges) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  MutablePolicyListItem item("Headline", "");
  ListTextPolicy headline_policy;
  headline_policy.overflow = TextOverflowPolicy::kTruncate;
  headline_policy.max_lines = 1;
  item.setHeadlinePolicy(headline_policy);
  TestListEntry entry(context);

  entry.setItem(item);
  ASSERT_EQ(1, entry.getChildrenCount());
  const Widget* one_line_child = &entry.getChild(0);

  headline_policy.overflow = TextOverflowPolicy::kWrap;
  headline_policy.max_lines = 2;
  item.setHeadlinePolicy(headline_policy);
  entry.refreshFromItem();

  ASSERT_EQ(1, entry.getChildrenCount());
  const Widget* wrapped_child = &entry.getChild(0);
  EXPECT_NE(one_line_child, wrapped_child);
}

// Verifies that max-lines policy affects measured row height for standard
// supporting text without requiring custom caller-provided text widgets.
TEST(Material3List, TextPolicyMaxLinesChangesMeasuredRowHeight) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  MutablePolicyListItem item(
      "Headline",
      "Supporting text that is intentionally long enough to wrap across "
      "multiple lines in a narrow row.");
  ListTextPolicy supporting_policy;
  supporting_policy.overflow = TextOverflowPolicy::kWrap;
  supporting_policy.max_lines = 1;
  item.setSupportingPolicy(supporting_policy);
  TestListEntry entry(context);
  entry.setItem(item);

  Dimensions one_line =
      entry.measure(WidthSpec::Exactly(120), HeightSpec::Unspecified(0));

  supporting_policy.max_lines = 3;
  item.setSupportingPolicy(supporting_policy);
  entry.refreshFromItem();
  Dimensions three_line =
      entry.measure(WidthSpec::Exactly(120), HeightSpec::Unspecified(0));

  EXPECT_GT(three_line.height(), one_line.height());
}

// Verifies that expressive rows derive their rounded-corner geometry from the
// list-provided row position and selection state, while baseline rows stay
// square.
TEST(Material3List, ListEntryResolvesShapeFromVariantPositionAndSelection) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestListEntry entry(context);

  ListEntryVisualContext visual_context;
  visual_context.variant = ListVariant::kExpressive;

  entry.setVisualContext(visual_context);
  BorderStyle single = entry.getBorderStyle();
  EXPECT_EQ(Scaled(16), single.top_left_corner_radius());
  EXPECT_EQ(Scaled(16), single.top_right_corner_radius());
  EXPECT_EQ(Scaled(16), single.bottom_right_corner_radius());
  EXPECT_EQ(Scaled(16), single.bottom_left_corner_radius());

  visual_context.position = ListItemPosition::kFirst;
  entry.setVisualContext(visual_context);
  BorderStyle first = entry.getBorderStyle();
  EXPECT_EQ(Scaled(16), first.top_left_corner_radius());
  EXPECT_EQ(Scaled(16), first.top_right_corner_radius());
  EXPECT_EQ(Scaled(4), first.bottom_right_corner_radius());
  EXPECT_EQ(Scaled(4), first.bottom_left_corner_radius());

  visual_context.position = ListItemPosition::kMiddle;
  entry.setVisualContext(visual_context);
  BorderStyle middle = entry.getBorderStyle();
  EXPECT_EQ(Scaled(4), middle.top_left_corner_radius());
  EXPECT_EQ(Scaled(4), middle.top_right_corner_radius());
  EXPECT_EQ(Scaled(4), middle.bottom_right_corner_radius());
  EXPECT_EQ(Scaled(4), middle.bottom_left_corner_radius());

  visual_context.position = ListItemPosition::kLast;
  entry.setVisualContext(visual_context);
  BorderStyle last = entry.getBorderStyle();
  EXPECT_EQ(Scaled(4), last.top_left_corner_radius());
  EXPECT_EQ(Scaled(4), last.top_right_corner_radius());
  EXPECT_EQ(Scaled(16), last.bottom_right_corner_radius());
  EXPECT_EQ(Scaled(16), last.bottom_left_corner_radius());

  visual_context.position = ListItemPosition::kMiddle;
  visual_context.selected = true;
  entry.setVisualContext(visual_context);
  BorderStyle selected_middle = entry.getBorderStyle();
  EXPECT_EQ(Scaled(16), selected_middle.top_left_corner_radius());
  EXPECT_EQ(Scaled(16), selected_middle.top_right_corner_radius());
  EXPECT_EQ(Scaled(16), selected_middle.bottom_right_corner_radius());
  EXPECT_EQ(Scaled(16), selected_middle.bottom_left_corner_radius());

  visual_context.variant = ListVariant::kBaseline;
  visual_context.position = ListItemPosition::kFirst;
  entry.setVisualContext(visual_context);
  BorderStyle baseline = entry.getBorderStyle();
  EXPECT_EQ(0, baseline.top_left_corner_radius());
  EXPECT_EQ(0, baseline.top_right_corner_radius());
  EXPECT_EQ(0, baseline.bottom_right_corner_radius());
  EXPECT_EQ(0, baseline.bottom_left_corner_radius());
}

// Verifies that ListRow owns exactly one inline item, binds it immediately,
// and exposes the same row contracts through the inherited ListEntry surface.
TEST(Material3List, ListRowOwnsAndBindsItsInlineItem) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestStandardListRow row(
      context, StandardListItemInit::TwoLine("Schedule", "Weekdays only"));

  EXPECT_TRUE(row.hasItem());
  EXPECT_EQ(&row.item(), static_cast<ListEntry&>(row).item());
  EXPECT_EQ("Schedule", std::string(row.item().headlineText()));
  EXPECT_EQ("Weekdays only", std::string(row.item().supportingText()));
  EXPECT_EQ(2, row.getChildrenCount());

  Dimensions measured =
      row.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  EXPECT_EQ(180, measured.width());
  EXPECT_GE(measured.height(), Scaled(72));
}

// Verifies that List continues to accept borrowed and adopted ListRow
// instances because the bridge adds ownership glue, not a second row model.
TEST(Material3List, ListAcceptsBorrowedAndAdoptedListRows) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestList list(context);
  ListRow<StandardListItem> borrowed(context,
                                     StandardListItemInit::OneLine("Borrowed"));
  auto adopted = std::make_unique<ListRow<StandardListItem>>(
      context, StandardListItemInit::OneLine("Adopted"));
  ListRow<StandardListItem>* adopted_raw = adopted.get();

  list.add(borrowed);
  list.add(std::move(adopted));

  EXPECT_EQ(2, list.getChildrenCount());
  EXPECT_EQ(&borrowed, &list.getChild(0));
  EXPECT_EQ(adopted_raw, &list.getChild(1));
  EXPECT_EQ(&list, borrowed.parent());
  EXPECT_EQ(&list, adopted_raw->parent());
}

// Verifies that inset divider painting uses the list policy floor together
// with the bound item's content hint, while pixels outside the resolved inset
// remain row background.
TEST_F(Material3ListRenderTest, InsetDividerPaintUsesResolvedInsets) {
  auto list = std::make_unique<List>(context());
  List* list_ptr = list.get();
  list_ptr->setVariant(ListVariant::kBaseline);

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kInset;
  divider_policy.start_inset = 18;
  divider_policy.end_inset = 26;
  list_ptr->setDividerPolicy(divider_policy);

  StandardListItemInit first_init = StandardListItemInit::OneLine("First");
  first_init.divider_inset_hint.start_inset = 40;
  first_init.divider_inset_hint.end_inset = 8;

  auto first =
      std::make_unique<ListRow<StandardListItem>>(context(), first_init);
  auto second = std::make_unique<ListRow<StandardListItem>>(
      context(), StandardListItemInit::OneLine("Second"));
  ListEntry* first_row = first.get();
  ListEntry* second_row = second.get();

  list_ptr->add(std::move(first));
  list_ptr->add(std::move(second));

  app_.add(WidgetRef(std::move(list)), roo_display::Box(10, 12, 149, 123));

  ASSERT_TRUE(refresh());

  roo_display::Color divider_color = test_support::QuantizeToArgb4444(
      context().theme().color.role(ColorRole::kOutlineVariant));
  int16_t divider_thickness = Scaled(1);

  EXPECT_EQ(first_row->height() + divider_thickness, second_row->offsetTop());

  int16_t divider_y =
      list_ptr->offsetTop() + first_row->offsetTop() + first_row->height();
  int16_t row_left = list_ptr->offsetLeft() + first_row->offsetLeft();
  int16_t row_right = row_left + first_row->width() - 1;

  EXPECT_NE(divider_color, pixelAt(row_left + 39, divider_y));
  EXPECT_EQ(divider_color, pixelAt(row_left + 40, divider_y));
  EXPECT_EQ(divider_color, pixelAt(row_right - 26, divider_y));
  EXPECT_NE(divider_color, pixelAt(row_right - 25, divider_y));
}

// Verifies that expressive standard lists keep divider strokes in shared
// inter-row separator space instead of drawing them inside the previous row.
TEST_F(Material3ListRenderTest, ExpressiveInsetDividerPaintUsesGapSpace) {
  auto list = std::make_unique<List>(context());
  List* list_ptr = list.get();

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kInset;
  divider_policy.start_inset = 18;
  divider_policy.end_inset = 26;
  list_ptr->setDividerPolicy(divider_policy);

  StandardListItemInit first_init = StandardListItemInit::OneLine("First");
  first_init.divider_inset_hint.start_inset = 40;
  first_init.divider_inset_hint.end_inset = 8;

  auto first =
      std::make_unique<ListRow<StandardListItem>>(context(), first_init);
  auto second = std::make_unique<ListRow<StandardListItem>>(
      context(), StandardListItemInit::OneLine("Second"));
  ListEntry* first_row = first.get();
  ListEntry* second_row = second.get();

  list_ptr->add(std::move(first));
  list_ptr->add(std::move(second));

  app_.add(WidgetRef(std::move(list)), roo_display::Box(10, 12, 149, 123));

  ASSERT_TRUE(refresh());

  roo_display::Color divider_color = test_support::QuantizeToArgb4444(
      context().theme().color.role(ColorRole::kOutlineVariant));
  roo_display::Color surface_color = test_support::QuantizeToArgb4444(
      context().theme().color.role(ColorRole::kSurface));
  int16_t expressive_gap = Scaled(2);
  int16_t divider_thickness = Scaled(1);

  EXPECT_EQ(first_row->height() + expressive_gap + divider_thickness,
            second_row->offsetTop());

  int16_t divider_y =
      list_ptr->offsetTop() + first_row->offsetTop() + first_row->height() +
      (expressive_gap + divider_thickness - divider_thickness) / 2;
  int16_t row_bottom_y =
      list_ptr->offsetTop() + first_row->offsetTop() + first_row->height() - 1;
  int16_t row_left = list_ptr->offsetLeft() + first_row->offsetLeft();
  int16_t row_right = row_left + first_row->width() - 1;

  EXPECT_EQ(surface_color, pixelAt(row_left + 40, row_bottom_y));
  EXPECT_EQ(divider_color, pixelAt(row_left + 40, divider_y));
  EXPECT_EQ(divider_color, pixelAt(row_right - 26, divider_y));
}

// Verifies that List keeps borrowed entries borrowed, adopts unique_ptr rows,
// and detaches or destroys them correctly on clear().
TEST(Material3List, ListClearsBorrowedAndAdoptedEntriesWithCorrectOwnership) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestList list(context);
  TestListEntry borrowed(context);
  bool adopted_destroyed = false;
  auto adopted =
      std::make_unique<TrackingListEntry>(context, adopted_destroyed);
  TrackingListEntry* adopted_raw = adopted.get();

  list.add(borrowed);
  list.add(std::move(adopted));

  EXPECT_EQ(2, list.getChildrenCount());
  EXPECT_EQ(&borrowed, &list.getChild(0));
  EXPECT_EQ(adopted_raw, &list.getChild(1));
  EXPECT_EQ(&list, borrowed.parent());
  EXPECT_EQ(&list, adopted_raw->parent());

  list.clear();

  EXPECT_EQ(0, list.getChildrenCount());
  EXPECT_EQ(nullptr, borrowed.parent());
  EXPECT_TRUE(adopted_destroyed);
}

// Verifies that List recomputes first, middle, last, and single row position
// roles as rows are added and list-level policies change.
TEST(Material3List, ListPropagatesPositionVariantStyleAndSegmentedGap) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestList list(context);
  TestListEntry first(context);
  TestListEntry second(context);
  TestListEntry third(context);
  StandardListItem first_item(StandardListItemInit::OneLine("First"));
  StandardListItem second_item(StandardListItemInit::OneLine("Second"));
  StandardListItem third_item(StandardListItemInit::OneLine("Third"));
  first.setItem(first_item);
  second.setItem(second_item);
  third.setItem(third_item);

  list.add(first);
  EXPECT_EQ(ListItemPosition::kSingle, first.visualContext().position);

  list.add(second);
  EXPECT_EQ(ListItemPosition::kFirst, first.visualContext().position);
  EXPECT_EQ(ListItemPosition::kLast, second.visualContext().position);

  list.setVariant(ListVariant::kBaseline);
  list.setStyle(ListStyle::kSegmented);
  list.add(third);

  EXPECT_EQ(ListVariant::kBaseline, first.visualContext().variant);
  EXPECT_EQ(ListStyle::kSegmented, first.visualContext().style);
  EXPECT_EQ(ListItemPosition::kFirst, first.visualContext().position);
  EXPECT_EQ(ListItemPosition::kMiddle, second.visualContext().position);
  EXPECT_EQ(ListItemPosition::kLast, third.visualContext().position);

  Dimensions measured =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  list.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));

  EXPECT_EQ(0, first.offsetTop());
  EXPECT_EQ(first.height() + Scaled(8), second.offsetTop());
  EXPECT_EQ(second.offsetTop() + second.height() + Scaled(8),
            third.offsetTop());
}

// Verifies that selection policy resolves the list-owned selected state and
// divider visibility from stored per-entry selection hints.
TEST(Material3List, ListResolvesSelectionAndDividerContext) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestList list(context);
  TestListEntry first(context);
  TestListEntry second(context);
  TestListEntry third(context);
  StandardListItem first_item(StandardListItemInit::OneLine("One"));
  StandardListItem second_item(StandardListItemInit::OneLine("Two"));
  StandardListItem third_item(StandardListItemInit::OneLine("Three"));
  first.setItem(first_item);
  second.setItem(second_item);
  third.setItem(third_item);

  ListEntryVisualContext selected_context;
  selected_context.selected = true;
  first.setVisualContext(selected_context);
  second.setVisualContext(selected_context);

  list.add(first);
  list.add(second);
  list.add(third);

  EXPECT_FALSE(first.visualContext().selected);
  EXPECT_FALSE(second.visualContext().selected);

  ListSelectionPolicy selection_policy;
  selection_policy.mode = SelectionMode::kMultiple;
  list.setSelectionPolicy(selection_policy);

  EXPECT_TRUE(first.visualContext().selected);
  EXPECT_TRUE(second.visualContext().selected);
  EXPECT_FALSE(third.visualContext().selected);

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kInset;
  divider_policy.suppress_between_selected = true;
  list.setDividerPolicy(divider_policy);

  EXPECT_EQ(DividerMode::kInset, first.visualContext().divider_mode);
  EXPECT_EQ(0, first.visualContext().divider_start_inset);
  EXPECT_EQ(0, first.visualContext().divider_end_inset);
  EXPECT_FALSE(first.visualContext().show_divider);
  EXPECT_TRUE(second.visualContext().show_divider);
  EXPECT_FALSE(third.visualContext().show_divider);

  selection_policy.mode = SelectionMode::kSingle;
  list.setSelectionPolicy(selection_policy);

  EXPECT_TRUE(first.visualContext().selected);
  EXPECT_FALSE(second.visualContext().selected);
  EXPECT_TRUE(first.visualContext().show_divider);
}

// Verifies that expressive standard lists keep a small visual separator
// between rows that are explicitly divided, and preserve that same separator
// when a selected-selected divider is suppressed.
TEST(Material3List, ListAddsSmallGapBetweenSeparatedExpressiveRows) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestList list(context);
  TestListEntry first(context);
  TestListEntry second(context);
  TestListEntry third(context);
  StandardListItem first_item(StandardListItemInit::OneLine("One"));
  StandardListItem second_item(StandardListItemInit::OneLine("Two"));
  StandardListItem third_item(StandardListItemInit::OneLine("Three"));
  first.setItem(first_item);
  second.setItem(second_item);
  third.setItem(third_item);

  ListEntryVisualContext selected_context;
  selected_context.selected = true;
  second.setVisualContext(selected_context);
  third.setVisualContext(selected_context);

  list.add(first);
  list.add(second);
  list.add(third);

  ListSelectionPolicy selection_policy;
  selection_policy.mode = SelectionMode::kMultiple;
  list.setSelectionPolicy(selection_policy);

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kInset;
  divider_policy.suppress_between_selected = true;
  list.setDividerPolicy(divider_policy);

  Dimensions measured =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  list.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));

  EXPECT_EQ(first.height() + Scaled(2) + Scaled(1), second.offsetTop());
  EXPECT_EQ(second.offsetTop() + second.height() + Scaled(2),
            third.offsetTop());
}

// Verifies that segmented lists use the full segmented gap while divider mode
// is disabled, and shrink back to a divider-thickness separator when dividers
// are on.
TEST(Material3List, ListGapResolutionDependsOnDividerPolicy) {
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestList list(context);
  TestListEntry first(context);
  TestListEntry second(context);
  StandardListItem first_item(StandardListItemInit::OneLine("One"));
  StandardListItem second_item(StandardListItemInit::OneLine("Two"));
  first.setItem(first_item);
  second.setItem(second_item);
  list.setStyle(ListStyle::kSegmented);
  list.add(first);
  list.add(second);

  Dimensions with_gap =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kFullWidth;
  list.setDividerPolicy(divider_policy);
  Dimensions with_divider =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));

  EXPECT_EQ(with_gap.height(), with_divider.height() + Scaled(8) - Scaled(1));
  EXPECT_TRUE(first.visualContext().show_divider);
  EXPECT_FALSE(second.visualContext().show_divider);
}

// Verifies the checked Phase 1 size budget using pointer-size-aware limits so
// the host build still catches accidental inline state growth.
TEST(Material3List, BaselineTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kStandardItemBudget =
      3 * sizeof(roo_display::StringView) + 4 * sizeof(void*) + 16;
  constexpr size_t kListEntryBudget =
      sizeof(Container) + 7 * sizeof(void*) + 16;
  constexpr size_t kExpandablePanelBudget =
      sizeof(Container) + sizeof(WidgetRef) + 8;
  constexpr size_t kListBudget = sizeof(Container) + sizeof(void*) * 8 + 16;

  EXPECT_LE(sizeof(StandardListItem), kStandardItemBudget);
  EXPECT_LE(sizeof(ListEntry), kListEntryBudget);
  EXPECT_LE(sizeof(ExpandablePanel), kExpandablePanelBudget);
  EXPECT_LE(sizeof(List), kListBudget);
  EXPECT_LE(sizeof(ListRow<StandardListItem>),
            sizeof(ListEntry) + sizeof(StandardListItem) + sizeof(void*));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows